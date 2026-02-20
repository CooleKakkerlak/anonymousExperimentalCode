#pragma once
#include <vector>
#include <CGAL/Cartesian_d.h>
#include "../shared/defs.h"
#include "../myRangeTree/myRangeTree.h"

//TODO: netter maken. bv pair_ni weg of iig verduidelijken

struct RangeQueryDS2D {
	MyRangeTree tree;
	std::vector<double> sortedX, sortedY;

	RangeQueryDS2D(std::vector<Point_2>& points, std::vector<Color_>& colors);

	// in dimension dim, among the points with smaller than q, find the largest point p s.t. the square centered at q with radius abs(q-p) contains at least k points
	template<typename Iterator>
	double binarySearch(Iterator first, Iterator last, Point_2 point, int k, int dim, bool ascending);

	// Returns radius from q
	double query_k_nearest(Point_2 point, int k);

	long get2DRangeMemoryUsage();
};

// in dimension dim, among the points with smaller coordinate than q, find the largest point p s.t. the square centered at q with radius abs(q-p) contains at least k points
template<typename Iterator>
double RangeQueryDS2D::binarySearch(Iterator first, Iterator last, Point_2 point, int k, int dim, bool ascending) {
	//the value we look for lies between l and u (l inclusive, u exclusive)
	//if ascending u is first value greater than point; if descending first value smaller than
	Iterator l = first;
	Iterator u = ascending ? std::upper_bound(first, last, point[dim], [](double a, double b) {return a < b; }) : std::upper_bound(first, last, point[dim], [](double a, double b) {return a > b; });
	double res = std::numeric_limits<double>::max();

	while (l < u) {
		Iterator m = l + (u - l) / 2;
		double distance = std::abs(point[dim] - *m) + constants::epsilon;

		int count = tree.rangeCount(Range2D(point, distance)); 

		if (count >= k) {
			res = std::abs(point[dim] - *m);
			l = m + 1;
		}
		else {
			u = m;
		}
	}

	return res;
}