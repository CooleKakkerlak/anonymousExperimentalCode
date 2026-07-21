#include "range_query.h"
#include "../shared/defs.h"
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

RangeQueryDS1D::RangeQueryDS1D(std::vector<double>& points) :
	points(points) {
}

Range RangeQueryDS1D::rangeQuery(double point, int k) {
	auto lower = binarySearch(points.begin(), points.end(), point, k, true);
	auto upper = binarySearch(points.rbegin(), points.rend(), point, k, false);

	if (lower.right - lower.left == k) return lower;
	else if (upper.right - upper.left == k) return upper;
	std::cout << "not possible";
	for (double p : points)
		std::cout << p << ", ";
	std::cout << std::endl;
	std::cout << point << ", " << k << std::endl;
	std::cout << lower.left << ", " << lower.right << std::endl;
	std::cout << upper.left << ", " << upper.right << std::endl;
	throw std::runtime_error("not possible");
}

long RangeQueryDS1D::getMemoryUsage() {
	return sizeof(double) * points.size();
}

namespace DS {
	Range naiveRangeQuery(std::vector<double>& points, KQuery query) {
		// copy array for in place operation
		std::vector<double> copy(points);

		auto f = [&query](double x, double y) -> bool { return std::abs(x - query.point) < std::abs(y - query.point); };
		std::sort(copy.begin(), copy.end(), f); //sort by distance to q
		double kth = copy[query.k - 1]; //k cannot be zero
		double left, right;
		if (kth < query.point) {
			left = kth; right = query.point + query.point - kth;
		}
		else {
			left = query.point - (kth - query.point); right = kth;
		}
		int start = points.size(), end = -1;
		for (int i = 0; i < points.size(); i++) {
			if (points[i] >= left) {
				start = std::min(start, i); break;
			}
		}
		for (int i = points.size(); i > 0; i--) {
			if (points[i - 1] <= right) {
				end = std::max(end, i); break;
			}
		}
		return Range{ start, end };
	}
}