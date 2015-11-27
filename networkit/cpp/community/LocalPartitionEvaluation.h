#ifndef LOCALPARTITIONEVALUATION_H
#define LOCALPARTITIONEVALUATION_H

#include "LocalCommunityEvaluation.h"
#include "../structures/Partition.h"
#include "../graph/Graph.h"

namespace NetworKit {

/**
 * Virtual base class of all evaluation methods for a single Partition which is based on the evaluation of single clusters.
 * This is the base class for Partitions.
 */
class LocalPartitionEvaluation : public LocalCommunityEvaluation {
public:
	/**
	 * Initialize the partition evaluation method.
	 *
	 * @param G The graph on which the evaluation shall be performed
	 * @param P The partition that shall be evaluated.
	 */
	LocalPartitionEvaluation(const Graph &G, const Partition &P);
protected:
	const Graph &G;
	const Partition &P;
};

}

#endif // LOCALPARTITIONEVALUATION_H
