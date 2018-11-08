#include "GroupClosenessWeighted.h"
#include "../auxiliary/Log.h"

namespace NetworKit {
GroupClosenessWeighted::GroupClosenessWeighted(const Graph &G, const count k)
    : G(G), k(k), n(G.upperNodeIdBound()) {
	if (k == 0 || k > n) {
		throw std::runtime_error("Error: k must be at least 1 and at most n.");
	}
}

void GroupClosenessWeighted::init() { group.reserve(k); }

void GroupClosenessWeighted::run() {
	init();
	hasRun = true;
}
} // namespace NetworKit
