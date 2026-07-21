#pragma once

#include <random>
#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Segment_2.h>

// TODO: move things from this file to where they are supposed to be

namespace constants
{
	const double epsilon = 1e-12;
	// not very clean, but this is currently needed because the mode queries are otherwise always an edge case:
	//  they contain a point on their boundary (the k-th closest point). this makes checking correctness difficult.
	// with this epsilon, the correctness checks succeed; this includes a bit of luck, as there do exist queries for which this epsilon
	//  makes the answer incorrect. This is only relevant for checking correctness, and should not have an effect on any of the measured metrics.
}

// returns the given value as a string (useful for class names / variable names)
#define GET_NAME(x) (#x)

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
// typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Point_2<Kernel> Point_2;
typedef CGAL::Line_2<Kernel> Line_2;
typedef CGAL::Segment_2<Kernel> Segment_2;
typedef CGAL::Ray_2<Kernel> Ray_2;

typedef int Color_;

struct ColoredPoint_2
{
	Point_2 point;
	Color_ color;
	ColoredPoint_2(Point_2 point, Color_ color);
	ColoredPoint_2(double x, double y, Color_ color);
	inline size_t heap_size() const { return 0; }
};
std::vector<ColoredPoint_2> convertPoints(std::vector<Point_2>& points, std::vector<Color_>& colors);

struct KQuery
{
	double point;
	int k;
};
struct KQuery2D
{
	Point_2 point;
	int k;
};

// left inclusive, right exclusive
struct Range
{
	int left, right;
};
struct Range2D
{
	Point_2 lower, upper; /*Range2D(Point_2 lower, Point_2 upper);*/
	Range2D(Point_2 point, double radius);
	Range2D(double cx, double cy, double radius);
};

struct Scenario
{
	bool synthetic = true;

	//query parameters
	std::vector<int> ks = { 100 };
	double s = 100;
	int numQueries = 1000;

	//synthetic parameters
	int numPoints = 10000, numColors = 100;
	double min = 0, max = 10000;
	double gamma = 0, alpha = 0;
	int checkerStrips = 0;

	//real dataset parameters
	std::string filename;
	double trainFrac = .9;
	int d0 = 0, d1 = 1;

	std::string toString(int kIndex) const;
	std::string fancyToString() const;

	// checks if the scenario is valid, and clears any invalid k values
	bool isValid();
};

struct TestResult
{
	bool error = false;
	std::string dsType;
	std::chrono::nanoseconds modeBuildTime, rangeBuildTime;
	std::vector<std::chrono::nanoseconds> modeAverageQueryTimes, modeAverageQueryTimesExcludingFirst, rangeAverageQueryTimes;
	long modeSpace, rangeSpace;
};

struct Dataset1D{};
struct Dataset2D {
	std::vector<Point_2> points; 
	std::vector<Color_> colors;
	int numColors = 0;

	std::pair<Dataset2D, Dataset2D> trainTestSample(double trainFrac, std::mt19937& generator);
	void writeToIpe(std::string filename, double maxsize = 200);
};

struct FileOutputter
{
	std::ofstream myfile;

	FileOutputter(std::string filename);

	// output the results in a ;-separated .csv file
	void output(Scenario scenario, TestResult result);
};

struct ColorCount
{
public:
	Color_ color;
	unsigned int count;

	ColorCount();

	ColorCount(Color_ c, unsigned int cnt);

	ColorCount(std::pair<Color_, unsigned int> pair);

	void print();

	bool operator==(const ColorCount &other);
	bool operator!=(const ColorCount &other);
};


ColorCount maxCount(const std::map<Color_, int> &colorCounts);

// random number generation, c# style
struct MyRandom
{
	std::mt19937 gen;

	MyRandom();
	MyRandom(int seed);

	// inclusive min, exclusive max
	int nextInt(int min, int max);

	// inclusive min, exclusive max
	double nextDouble(double min, double max);
};

class Timer
{
	std::chrono::high_resolution_clock::time_point start;

public:
	Timer();

	// reset the timer to 0 and get the time since last start
	std::chrono::nanoseconds reset();

	// get the time since last start
	std::chrono::nanoseconds sinceStart();
};
/*
// measures CPU time (but very inaccurately)
class Timer2
{
	uint64_t start;

	uint64_t getCurrentCpuTime();

public:
	Timer2();

	// reset the timer to 0 and get the time since last start
	uint64_t reset();

	// get the time since last start
	uint64_t sinceStart();
};
*/
struct AAS1DModeDS
{
public:
	virtual ColorCount queryMode(const Range &query) = 0;
	virtual long getDSMemoryUsage() = 0;
	virtual std::string getName() = 0;
};

struct AAS2DModeDS
{
public:
	virtual ColorCount queryMode(const Range2D &query) = 0;
	virtual long getDSMemoryUsage() = 0;
	virtual std::string getName() = 0;
};