#ifndef INDEPENDENT_CASCADE_H_
#define INDEPENDENT_CASCADE_H_

#include <vector>

#include "../graph/Graph.h"

namespace NetworKit {

class IndependentCascade {
public:
	IndependentCascade(const Graph &G);

	void performSimulation(const std::vector<node> &seeds,
	                       const count nIter = 1e4, const count randomSeed = 1);
	double getAverage() const;
	count getMinimum() const;
	count getMaximum() const;
	std::vector<double> getInfluencedVector() const;

private:
	const Graph &G;
	const count n;
	count max, min;
	double avg;
	bool hasRun;
	std::vector<double> influencedVector;

	void checkHasRun() const;
};

inline void IndependentCascade::checkHasRun() const {
	if (!hasRun) {
		throw std::runtime_error("Error: call performSimulation method first.");
	}
}

inline double IndependentCascade::getAverage() const {
	checkHasRun();
	return avg;
}

inline count IndependentCascade::getMaximum() const {
	checkHasRun();
	return max;
}

inline count IndependentCascade::getMinimum() const {
	checkHasRun();
	return min;
}

inline std::vector<double> IndependentCascade::getInfluencedVector() const {
	checkHasRun();
	return influencedVector;
}

} // namespace NetworKit

#endif
