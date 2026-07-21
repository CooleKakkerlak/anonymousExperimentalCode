#pragma once
#include "../shared/defs.h"

// first on x, then on y
struct MyRangeTree
{

public:
	double minX, maxX;
	std::unique_ptr<MyRangeTree> left = nullptr, right = nullptr;
	std::vector<double> ys;
	std::vector<ColoredPoint_2> pointsSorted;

	// points must be sorted by x
	MyRangeTree(std::vector<ColoredPoint_2> points, bool sorted = false);
	MyRangeTree(std::vector<Point_2> &points, std::vector<Color_> &colors, bool sorted = false);
	MyRangeTree(MyRangeTree const &rt) = default;
	MyRangeTree(MyRangeTree &&rt) = default;

	int rangeCount1D(const Range2D &range) const;
	std::vector<ColoredPoint_2> rangeReport1D(const Range2D &range) const;

	// builds the canonical subset of nodes contained in output
	void rangeQuery(const Range2D &range, std::vector<const MyRangeTree *> &output) const;
	int rangeCount(const Range2D &range) const;
	std::vector<ColoredPoint_2> rangeReport(const Range2D &range) const;

	long getMemUsage() const;
};
