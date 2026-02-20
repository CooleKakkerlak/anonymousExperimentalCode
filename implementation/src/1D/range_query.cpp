#include "range_query.h"
#include "../shared/defs.h"
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

// im just testing stuff out at this point, but macros are fun!
#define casted_malloc(type) (type)malloc(sizeof(type))
#define for_each(pointer_name, items) for(auto pointer_name = items.begin(); pointer_name < items.end(); pointer_name++)
//#define DEBUG_PRINT

namespace DS {
	// Reference implementation
	// O(n lg n) (sorting entire input array by distance to q)
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

	Side flip(Side side) {
		return side == Left ? Right : Left;
	}

	Color__ flip(Color__ color) {
		return color == Red ? Blue : Red;
	}

	Range get_range(std::vector<double>& input, double q, int k, int index) {
		if (input[index] > q)
			return { index + 1 - k, index + 1 };
		else
			return { index, index + k };
	}
}