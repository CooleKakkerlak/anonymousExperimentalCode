#include "myRangeTree.h"

MyRangeTree::MyRangeTree(std::vector<ColoredPoint_2> points, bool sorted)
{
	if (points.size() == 0) {
		minX = 1; maxX = 0;
		return;
	}
	if (!sorted)
		std::sort(points.begin(), points.end(), [](const ColoredPoint_2 &a, const ColoredPoint_2 &b)
				  { return a.point.x() < b.point.x(); }); // sort by x
	minX = points[0].point.x();
	maxX = points[points.size() - 1].point.x();

	pointsSorted = points;
	std::sort(pointsSorted.begin(), pointsSorted.end(), [](const ColoredPoint_2 &a, const ColoredPoint_2 &b)
			  { return a.point.y() < b.point.y(); });
	for (int i = 0; i < pointsSorted.size(); i++)
		ys.push_back(pointsSorted[i].point.y());

	if (points.size() == 1)
		return; // leaf

	int m = points.size() / 2;
	std::vector<ColoredPoint_2> leftPoints, rightPoints;
	for (int i = 0; i < m; i++)
		leftPoints.push_back(points[i]);
	for (int i = m; i < points.size(); i++)
		rightPoints.push_back(points[i]);
	left = std::make_unique<MyRangeTree>(leftPoints, true);
	right = std::make_unique<MyRangeTree>(rightPoints, true);
}

MyRangeTree::MyRangeTree(std::vector<Point_2> &points, std::vector<Color_> &colors, bool sorted) : MyRangeTree(convertPoints(points, colors), sorted) {}

int MyRangeTree::rangeCount1D(const Range2D &range) const
{
	int left = std::distance(ys.begin(), std::lower_bound(ys.begin(), ys.end(), range.lower.y()));	// inclusive
	int right = std::distance(ys.begin(), std::upper_bound(ys.begin(), ys.end(), range.upper.y())); // inclusive
	return right - left;
}

std::vector<ColoredPoint_2> MyRangeTree::rangeReport1D(const Range2D &range) const
{
	int left = std::distance(ys.begin(), std::lower_bound(ys.begin(), ys.end(), range.lower.y()));	// inclusive
	int right = std::distance(ys.begin(), std::upper_bound(ys.begin(), ys.end(), range.upper.y())); // inclusive
	std::vector<ColoredPoint_2> res;
	for (int i = left; i < right; i++)
	{
		res.push_back(pointsSorted[i]);
	}
	return res;
}

void MyRangeTree::rangeQuery(const Range2D &range, std::vector<const MyRangeTree *> &output) const
{
	if (range.lower.x() <= minX && range.upper.x() >= maxX) // node fully contained in x-range of query
		output.push_back(this);
	else if (range.lower.x() <= maxX && range.upper.x() >= minX)
	{ // node intersects x-range of query
		left->rangeQuery(range, output);
		right->rangeQuery(range, output);
	} // else: nothing
}

int MyRangeTree::rangeCount(const Range2D &range) const
{
	std::vector<const MyRangeTree *> canonical;
	rangeQuery(range, canonical);
	int count = 0;
	for (const MyRangeTree *rt : canonical)
	{
		count += rt->rangeCount1D(range);
	}
	return count;
}

std::vector<ColoredPoint_2> MyRangeTree::rangeReport(const Range2D &range) const
{
	std::vector<const MyRangeTree *> canonical;
	rangeQuery(range, canonical);
	std::vector<ColoredPoint_2> res;
	for (const MyRangeTree *rt : canonical)
	{
		std::vector<ColoredPoint_2> subres = rt->rangeReport1D(range);
		res.insert(res.end(), subres.begin(), subres.end());
	}
	return res;
}

long MyRangeTree::getMemUsage() const
{
	long count = 0;
	count += sizeof(minX) + sizeof(maxX);
	count += sizeof(double) * ys.size();
	count += sizeof(ColoredPoint_2) * pointsSorted.size();
	if (left != nullptr)
		count += left->getMemUsage();
	if (right != nullptr)
		count += right->getMemUsage();
	return count;
}