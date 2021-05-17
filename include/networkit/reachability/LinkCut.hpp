#ifndef NETWORKIT_REACHABILITY_LINK_CUT_HPP
#define NETWORKIT_REACHABILITY_LINK_CUT_HPP

#include <vector>

#include <networkit/graph/Graph.hpp>

namespace NetworKit {
class LinkCut final {
public:
    LinkCut(const Graph &G);

    std::vector<double> simulation(count reps, count cutsPerRep);
private:
    const Graph *G;
    node root;

    std::vector<node> sequence, parent, seenNodes;
    std::vector<count> edgeScores, pathLength;
    std::vector<EdgeWithId> nonSTEdges;

    std::vector<short> status;
    std::vector<bool> edgeInST;

    void sampleUST();
    void doLinkCut();
    void buildNodeSequence();
};

} // namespace NetworKit
#endif
