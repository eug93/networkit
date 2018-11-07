#include <cmath>
#include <omp.h>
#include <queue>

#include "../auxiliary/Parallel.h"
#include "../auxiliary/PrioQueue.h"
#include "../components/StronglyConnectedComponents.h"
#include "WeightedTopCloseness.h"

namespace NetworKit {
WeightedTopCloseness::WeightedTopCloseness(const Graph &G, const count k,
                                           const bool firstHeu,
                                           const bool secondHeu)
    : G(G), k(k), firstHeu(firstHeu), secondHeu(secondHeu),
      n(G.upperNodeIdBound()), kth(infDist) {
	if (k == 0 || k > n) {
		throw std::runtime_error("Error: k must be at least 1 and at most n.");
	}
	if (!G.hasEdgeIds()) {
		throw std::runtime_error("Call index edges before.");
	}
}

void WeightedTopCloseness::init() {
	topkNodes.resize(k);
	farness.assign(n, infDist);
	toAnalyze.assign(n, true);
	reachL.assign(n, 0);
	dist.assign(n, infDist);
	reached.assign(n, false);
	lowerBoundDist.assign(n, infDist);
	visitedEdges.assign(G.numberOfEdges(), false);
	sortedEdges.resize(G.numberOfEdges());
	G.parallelForEdges([&](node u, node v, edgeweight ew, edgeid eid) {
		sortedEdges[eid] = std::make_pair(eid, ew);
	});
	Aux::Parallel::sort(
	    sortedEdges.begin(), sortedEdges.end(),
	    [&](const std::pair<index, double> x, const std::pair<index, double> y) {
		    return x.second < y.second;
	    });
}

// TODO something better
double WeightedTopCloseness::minUnvisitedEdge() const {
	auto minWeightEdge = sortedEdges.begin();
	while (minWeightEdge != sortedEdges.end() &&
	       visitedEdges[minWeightEdge->first]) {
		++minWeightEdge;
	}
	return minWeightEdge == sortedEdges.end() ? infDist : minWeightEdge->second;
}

void WeightedTopCloseness::computeReachable() {
	StronglyConnectedComponents sccs(G);
	sccs.run();

	const count N = sccs.numberOfComponents();
	Graph sccGraph(N, false, true);
	std::vector<bool> isAdj(N, false);
	std::vector<bool> reachFromMaxSCC(N, false);
	std::vector<count> reachLSCC(N, 0);
	std::vector<count> reachUSCC(N, 0);
	std::vector<count> reachUWithoutMaxSCC(N, 0);
	std::vector<bool> reachFromMaxScc(N, false);
	std::vector<bool> reachesMaxSCC(N, false);
	std::vector<std::vector<node>> sccVec(N);

	for (auto elem : sccs.getPartition().subsetSizeMap()) {
		sccVec[elem.first - 1].reserve(elem.second);
	}

	for (node u = 0; u < n; ++u) {
		sccVec[sccs.componentOfNode(u) - 1].push_back(u);
	}

	count maxSizeSCC = 0;
	// TODO parallelize
	for (count V = 0; V < N; ++V) {
		for (node u : sccVec[V]) {
			G.forNeighborsOf(u, [&](node v) {
				count W = sccs.componentOfNode(v) - 1;
				if (W != V && !isAdj[W]) {
					isAdj[W] = true;
					sccGraph.addEdge(V, W);
				}
			});
		}
		sccGraph.forNeighborsOf(V, [&](node adjSCC) { isAdj[adjSCC] = false; });
		if (sccGraph.degreeOut(V) > sccGraph.degreeOut(maxSizeSCC)) {
			maxSizeSCC = V;
		}
	}

	std::queue<node> Q;
	Q.push(maxSizeSCC);
	reachFromMaxSCC[maxSizeSCC] = true;
	count V;
	while (!Q.empty()) {
		V = Q.front();
		Q.pop();
		reachLSCC[maxSizeSCC] += sccVec[V].size();
		sccGraph.forNeighborsOf(V, [&](node W) {
			if (!reachFromMaxSCC[W]) {
				reachFromMaxSCC[W] = true;
				Q.push(W);
			}
		});
	}

	reachUSCC[maxSizeSCC] = reachLSCC[maxSizeSCC];
	reachesMaxSCC[maxSizeSCC] = true;

	for (node V = 0; V < N; ++V) {
		if (V == maxSizeSCC) {
			continue;
		}
		sccGraph.forNeighborsOf(V, [&](node W) {
			reachLSCC[V] = std::max(reachLSCC[V], reachLSCC[W]);
			if (!reachFromMaxSCC[W]) {
				reachUWithoutMaxSCC[V] += reachUWithoutMaxSCC[W];
			}
			reachUSCC[V] += reachUSCC[W];
			reachUSCC[V] = std::min(reachUSCC[V], n);
			reachesMaxSCC[V] = reachesMaxSCC[V] || reachesMaxSCC[W];
		});
		if (reachesMaxSCC[V]) {
			reachUSCC[V] += reachUWithoutMaxSCC[V];
		}

		reachLSCC[V] += sccVec[V].size();
		reachUSCC[V] += sccVec[V].size();
		reachUSCC[V] = std::min(reachUSCC[V], n);
	}

	for (node v = 0; v < n; ++v) {
		reachL[v] = reachLSCC[sccs.componentOfNode(v) - 1];
	}
}

void WeightedTopCloseness::computeBounds() {
	// TODO: compute an alternative bound
	G.parallelForNodes([&](const node u) {
		if (G.degreeOut(u) > 0) {
			double minWeight = infDist;
			double numberOfNeighbors = 0;
			G.forNeighborsOf(u, [&](const node v) {
				minWeight = std::min(minWeight, G.weight(u, v));
				++numberOfNeighbors;
			});
			const double rL = reachL[u];
			farness[u] =
			    minWeight + (minWeight + sortedEdges[0].second) *
			                    (std::max(0.0, numberOfNeighbors - 1.0) +
			                     std::max(0.0, rL - 1.0 - numberOfNeighbors));
			farness[u] *= (n - 1) / (rL - 1.0) / (rL - 1.0);
		} else {
			farness[u] = infDist;
		}
	});
}

void WeightedTopCloseness::bfsBound(const node &s) {}

double WeightedTopCloseness::bfsCut(const node &s) {
	// Use a vector
	std::queue<node> toReset;
	std::queue<edgeid> edgesToReset;
	reached[s] = true;
	toReset.push(s);
	dist[s] = 0.0;
	Aux::PrioQueue<double, node> pq(dist);
	const double rL = reachL[s];
	double d = 0.0, sumDist = 0.0, newDist, lower;
	edgeweight minWeight = infDist;
	node cur;
	count reachedNodes = 1;

	while (pq.size() > 0) {
		cur = pq.extractMin().second;
		if (dist[cur] == infDist) {
			break;
		}

		G.forNeighborsOf(cur, [&](const node v, const edgeweight w) {
			// Maybe a CSRMatrix would be better
			edgeid eid = G.edgeId(cur, v);
			visitedEdges[eid] = true;
			edgesToReset.push(eid);
			newDist = dist[cur] + w;
			if (dist[v] > newDist) {
				pq.changeKey(newDist, v);
				if (!reached[v]) {
					reached[v] = true;
					toReset.push(v);
					++reachedNodes;
				} else {
					sumDist -= lowerBoundDist[v];
				}
				lowerBoundDist[v] = std::min(newDist, dist[cur] + minWeight);
				sumDist += lowerBoundDist[v];
				dist[v] = newDist;
			}
		});

		d += dist[cur];
		if (lowerBoundDist[cur] != infDist) {
			sumDist -= lowerBoundDist[cur];
		}

		if (pq.size() > 0) {
			// Next node to be picked, this distance is correct.
			newDist = pq.peekMin(0).first;
			if (newDist == infDist) {
				break;
			}
			cur = pq.peekMin(0).second;
			minWeight = minUnvisitedEdge();
			lower = d + newDist + sumDist - lowerBoundDist[cur] +
			        std::max(0.0, rL - reachedNodes) * (newDist + minWeight);

			lower *= (n - 1) / (rL - 1.0) / (rL - 1.0);
			if (lower >= kth) {
				break;
			}
		}
	}

	while (toReset.size() > 0) {
		cur = toReset.front();
		toReset.pop();
		dist[cur] = infDist;
		lowerBoundDist[cur] = infDist;
		reached[cur] = false;
	}

	while (edgesToReset.size() > 0) {
		visitedEdges[edgesToReset.front()] = false;
		edgesToReset.pop();
	}

	return lower;
}

void WeightedTopCloseness::run() {
	init();
	computeReachable();
	computeBounds();

	Aux::PrioQueue<double, node> top(n);
	Aux::PrioQueue<double, node> Q(farness);
	DEBUG("Done filling the queue.");

	std::pair<double, node> topPair;
	node s;
	// TODO parallelize
	while (Q.size() != 0) {
		DEBUG("# of nodes to be analyzed: ", Q.size());
		auto p = Q.extractMin();
		s = p.second;
		toAnalyze[s] = false;

		DEBUG("Priority of node ", s, " is ", p.first);
		if (G.degreeOut(s) == 0 || farness[s] > kth) {
			DEBUG("Discarding node ", s);
			break;
		}

		if (secondHeu) {
			bfsBound(s);
		} else {
			farness[s] = bfsCut(s);
		}

		if (farness[s] < kth) {
			top.insert(-farness[s], s);
			if (top.size() >= k) {
				while (top.size() > k) {
					top.extractMin();
				}
				topPair = top.peekMin(0);
				kth = -topPair.first;
				DEBUG("new kth = ", kth);
			}
		}
	}

	// Filling result vector
	while (top.size() > 0) {
		topPair = top.extractMin();
		topkNodes[top.size()] = topPair.second;
	}

	hasRun = true;
}

} // namespace NetworKit
