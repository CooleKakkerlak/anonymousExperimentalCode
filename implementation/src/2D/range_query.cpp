#include "range_query.h"
#include "../shared/defs.h"
#include <algorithm>

RangeQueryDS2D::RangeQueryDS2D(std::vector<Point_2>& points, std::vector<Color_>& colors):
	tree(points, colors) {

	//build sorted lists
	for (int i = 0; i < points.size(); i++) {
		sortedX.push_back(points[i].x());
		sortedY.push_back(points[i].y());
	}
	std::sort(sortedX.begin(), sortedX.end());
	std::sort(sortedY.begin(), sortedY.end());
}

// Returns radius from q
double RangeQueryDS2D::query_k_nearest(Point_2 point, int k) {
	auto x_lower = binarySearch(sortedX.begin(), sortedX.end(), point, k, 0, true);
	auto y_lower = binarySearch(sortedY.begin(), sortedY.end(), point, k, 1, true);
	auto x_upper = binarySearch(sortedX.rbegin(), sortedX.rend(), point, k, 0, false);
	auto y_upper = binarySearch(sortedY.rbegin(), sortedY.rend(), point, k, 1, false);

	return std::min({ x_lower, y_lower, x_upper, y_upper }) + constants::epsilon;
}

long RangeQueryDS2D::get2DRangeMemoryUsage() {
	long count = 0;
	count += tree.getMemUsage(); //TODO: fix
	count += sizeof(double) * sortedX.size();
	count += sizeof(double) * sortedY.size();
	return count;
}