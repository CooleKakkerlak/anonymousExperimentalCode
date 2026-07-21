//1D range query code by Thomas van der Plas

#pragma once

#include "../shared/defs.h"
#include <iostream>
#include <vector>
#include <functional>



struct RangeQueryDS1D {
	std::vector<double> points;

	RangeQueryDS1D(std::vector<double>& points);

	// Returns radius from q
	Range rangeQuery(double point, int k);

	long getMemoryUsage();

	//among the points with smaller than q, find the largest point p s.t. the square centered at q with radius abs(q-p) contains at most k points
	template<typename Iterator>
	Range binarySearch(Iterator first, Iterator last, double point, int k, bool ascending);
};

// among the points with smaller coordinate than q, find the largest point p s.t. the square centered at q with radius abs(q-p) contains at most k points
template<typename Iterator>
Range RangeQueryDS1D::binarySearch(Iterator first, Iterator last, double point, int k, bool ascending) {
	//the value we look for lies between l and u (l inclusive, u exclusive)
	//if ascending u is first value greater than point; if descending first value smaller than
	Iterator l = first;
	Iterator u = ascending ? std::upper_bound(first, last, point, [](double a, double b) {return a < b; }) : std::upper_bound(first, last, point, [](double a, double b) {return a > b; });
	Range res{ 0,0 };

	while (l < u) {
		Iterator m = l + (u - l) / 2;
		double distance = std::abs(point - *m) + constants::epsilon;

		int left = std::distance(points.begin(), std::lower_bound(points.begin(), points.end(), point - distance));	//inclusive
		int right = std::distance(points.begin(), std::upper_bound(points.begin(), points.end(), point + distance));  //exclusive
		int count = right - left;

		if (count >= k) {
			res = Range{ left,right };
			l = m + 1;
		}
		else {
			u = m;
		}
	}

	return res;
}

namespace DS {
	Range naiveRangeQuery(std::vector<double>& points, KQuery query);
}