/*
 * DynBFS.h
 *
 *  Created on: 17.07.2014
 *      Author: cls, ebergamini
 */

#ifndef DYNBFS_H_
#define DYNBFS_H_

#include "DynSSSP.hpp"


namespace NetworKit {

/**
 * @ingroup distance
 * Dynamic breadth-first search.
 */
class DynBFS : public DynSSSP {

public:

	/**
	 * Creates the object for @a G and source @a s.
	 *
	 * @param G The graph.
	 * @param s The source node.
	 * @param   storePredecessors   keep track of the lists of predecessors?
	 */
	DynBFS(const Graph& G, node s, bool storePredecessors = true);

	void run() override;

	/** Updates the distances after an edge insertion.*/
	void update(GraphEvent e) override;

	/** Updates the distances after a batch of edge insertions.*/
	void updateBatch(const std::vector<GraphEvent>& batch) override;

	/* Returns the number of shortest paths to node t.*/
	bigfloat getNumberOfPaths(node t) const;

protected:
	enum Color {WHITE, BLACK, GRAY};
	std::vector<Color> color;
	count maxDistance;

};

inline bigfloat DynBFS::getNumberOfPaths(node t) const {
	return npaths[t];
}

} /* namespace NetworKit */

#endif /* DYNSSSP_H_ */