#include "defs.h"

#include <random>
#include <vector>
#include <CGAL/Point_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <fstream>
#include <iomanip>
// #include <windows.h>

// the different relevant values of a scenario, interspersed with semicolons; only prints a specified value of k
// order: n, phi, gamma, alpha, s, k
std::string Scenario::toString(int kIndex) const
{
	std::stringstream stream;
	stream << (this->synthetic ? "synthetic" : this->filename) << ";" << this->numPoints << ";" << this->numColors << ";" << this->gamma << ";" << this->alpha << ";" << this->s << ";" << this->ks[kIndex];
	return stream.str();
}

std::string Scenario::fancyToString() const
{
	std::stringstream stream;
	stream << "Scenario: dataType=" << (this->synthetic ? "synthetic" : this->filename) << ", n=" << numPoints << ", phi=" << numColors << ", gamma=" << gamma << ", alpha=" << alpha << ", s=" << s << ", ks={";
	for (int k : ks)
		stream << k << ",";
	stream << "}. ";
	return stream.str();
}

bool Scenario::isValid()
{
	std::vector<int> goodKs;
	for (int k : ks)
		if (k <= numPoints)
			goodKs.push_back(k);
	ks = goodKs;
	return ks.size() > 0 && s <= numPoints && numColors <= numPoints;
}

FileOutputter::FileOutputter(std::string filename)
{
	//std::string folder = "C:\\Users\\6224474\\OneDrive - Universiteit Utrecht\\Documents\\code\\implementing_chromatic_knn\\implementation\\results\\";
	std::string folder = "C:\\Users\\erwin\\Documents\\phd\\code\\implementing_chromatic_knn\\implementation\\results\\";
	//std::string folder = "/home/ggiezeman/results/";

	std::string finalFileName = folder + filename + ".csv";

	bool fileAlreadyExisted = std::ifstream(finalFileName).good();

	myfile = std::ofstream(finalFileName, std::ios_base::app);

	if (!myfile.is_open())
		throw std::runtime_error("could not open file");
	if (fileAlreadyExisted)
	{
		std::cout << "Appending to file " << finalFileName << std::endl;
	}
	else
	{
		std::cout << "Creating new file " << finalFileName << std::endl;
		myfile << "DSType;dataType;numPoints;numColors;gamma;alpha;s;k;rangeSpace;rangeBuildTime;rangeQueryTime;modeSpace;modeBuildTime;modeQueryTime;modeQueryTimeExcludingFirst" << std::endl;
	}
}

void FileOutputter::output(Scenario scenario, TestResult result)
{
	if (!myfile.is_open())
		throw std::runtime_error("file no longer open");
	for (int j = 0; j < scenario.ks.size(); j++)
	{
		if (result.error)
			myfile << scenario.toString(j) << "ERROR" << std::endl;
		else
		{
			myfile << result.dsType << ";" << scenario.toString(j) << ";" << result.rangeSpace << ";"
				<< result.rangeBuildTime.count() << ";" << result.rangeAverageQueryTimes[j].count() << ";"
				<< result.modeSpace << ";" << result.modeBuildTime.count() << ";"
				<< result.modeAverageQueryTimes[j].count() << ";" << result.modeAverageQueryTimesExcludingFirst[j].count() << std::endl;
		}
	}
}

ColoredPoint_2::ColoredPoint_2(Point_2 point, Color_ color) : point(point), color(color) {}

ColoredPoint_2::ColoredPoint_2(double x, double y, Color_ color) : point(Point_2(x, y)), color(color)
{
}
std::vector<ColoredPoint_2> convertPoints(std::vector<Point_2>& points, std::vector<Color_>& colors)
{
	std::vector<ColoredPoint_2> res;
	for (int i = 0; i < points.size(); i++)
	{
		res.push_back(ColoredPoint_2(points[i], colors[i]));
	}
	return res;
}

ColorCount::ColorCount()
{
	color = -1;
	count = 0;
}

ColorCount::ColorCount(Color_ c, unsigned int cnt)
	: color(c), count(cnt)
{
}

ColorCount::ColorCount(std::pair<Color_, unsigned int> pair)
	: color(pair.first), count(pair.second)
{
}

void ColorCount::print()
{
	std::cout << "Color " << color << " appears " << count << " times." << std::endl;
}

bool ColorCount::operator==(const ColorCount& other)
{
	return this->color == other.color && this->count == other.count;
}
bool ColorCount::operator!=(const ColorCount& other)
{
	return !operator==(other);
}

Range2D::Range2D(Point_2 point, double radius) : lower(point.x() - radius, point.y() - radius), upper(point.x() + radius, point.y() + radius)
{
}
Range2D::Range2D(double cx, double cy, double radius)
	: lower(cx - radius, cy - radius), upper(cx + radius, cy + radius)
{
}
// Range2D::Range2D(Point_2 lower, Point_2 upper) :
//	lower(lower), upper(upper){
// }

ColorCount maxCount(const std::map<Color_, int>& colorCounts)
{
	if (colorCounts.size() == 0)
		return ColorCount();
	return ColorCount(*std::max_element(
		begin(colorCounts),
		end(colorCounts),
		[](const std::pair<Color_, int> a, const std::pair<Color_, int> b)
		{ return a.second < b.second; }));
}

std::pair<Dataset2D, Dataset2D> Dataset2D::trainTestSample(double trainFrac, std::mt19937& generator)
{
	std::vector<int> indices;
	for (int i = 0; i < points.size(); i++)
		indices.push_back(i);

	int trainN = trainFrac * points.size();
	std::vector<int> copy(indices.begin(), indices.end());
	std::shuffle(copy.begin(), copy.end(), generator);
	std::vector<int> trainIndices = std::vector<int>(copy.begin(), copy.begin() + trainN);
	std::vector<int> testIndices = std::vector<int>(copy.begin() + trainN, copy.end());

	Dataset2D train, test;
	for (int i : trainIndices)
	{
		train.points.push_back(points[i]);
		train.colors.push_back(colors[i]);
	}
	for (int i : testIndices)
	{
		test.points.push_back(points[i]);
		test.colors.push_back(colors[i]);
	}

	//ensure we don't have missing colors
	std::vector<Color_> fixedTrainColors;
	std::map<Color_, Color_> colorMapping;
	for (int i = 0; i < train.colors.size(); i++) {
		if (colorMapping.find(train.colors[i]) == colorMapping.end())
			colorMapping[train.colors[i]] = train.numColors++;
		fixedTrainColors.push_back((Color_)colorMapping[train.colors[i]]);
	}
	train.colors = fixedTrainColors;

	return std::pair(train, test);
}

void Dataset2D::writeToIpe(std::string filename, double maxsize) {
	//std::string folder = "C:\\Users\\6224474\\OneDrive - Universiteit Utrecht\\Documents\\code\\implementing_chromatic_knn\\implementation\\";
	std::string folder = "C:\\Users\\erwin\\Documents\\phd\\code\\implementing_chromatic_knn\\implementation\\";

	std::ifstream  ipeStart(folder + "src\\shared\\ipeStart.txt", std::ios::binary);
	std::ofstream  newIpeFile(folder + "figs\\pointsets\\" + filename, std::ios::binary);

	newIpeFile << ipeStart.rdbuf();
	ipeStart.close();

	std::vector<std::string> colorMapping{ "red", "blue", "lightgreen", "violet", "seagreen", "orange", "brown", "darkblue", "darkcyan", "darkgray", "darkgreen", "darkmagenta", "darkorange", "darkred", "gold", "gray", "green", "lightblue", "lightcyan", "lightgray", "lightgreen", "lightyellow"};

	if (numColors > colorMapping.size())
		throw std::exception("too many colors to visualize");


	double minx = 999999999, miny = 999999999, maxx = -99999999, maxy = -999999999;
	for (int i = 0; i < points.size(); i++) {
		minx = std::min(minx, points[i].x());
		miny = std::min(miny, points[i].y());
		maxx = std::max(maxx, points[i].x());
		maxy = std::max(maxy, points[i].y());
	}

	double xrange = maxx - minx, yrange = maxy - miny;
	double scale = std::min(maxsize / xrange, maxsize / yrange);
	for (int i = 0; i < points.size(); i++) {
		newIpeFile << "<use name=\"mark/disk(sx)\" pos=\"" << (points[i].x() - minx) * scale << " " << (points[i].y() - miny) * scale << "\" size=\"normal\" stroke=\"" << colorMapping[colors[i]] << "\"/>" << std::endl;
	}

	std::ifstream  ipeEnd(folder + "src\\shared\\ipeEnd.txt", std::ios::binary);

	newIpeFile << ipeEnd.rdbuf();
	ipeEnd.close();
	newIpeFile.close();
}

MyRandom::MyRandom() : gen{ std::random_device{}() } {} // random seed
MyRandom::MyRandom(int seed) : gen(seed) {}			  // given seed

int MyRandom::nextInt(int min, int max)
{
	return std::uniform_int_distribution<int>(min, max - 1)(gen);
}

double MyRandom::nextDouble(double min, double max)
{
	return std::uniform_real_distribution<double>(min, max)(gen);
}

Timer::Timer()
{
	reset();
}
std::chrono::nanoseconds Timer::reset()
{
	auto elapsed = sinceStart();
	start = std::chrono::high_resolution_clock::now();
	return elapsed;
}
std::chrono::nanoseconds Timer::sinceStart()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
}
/*
Timer2::Timer2()
{
	reset();
}
uint64_t Timer2::getCurrentCpuTime()
{
	FILETIME create, exit, kernel, user;
	GetThreadTimes(GetCurrentThread(), &create, &exit, &kernel, &user);

	ULARGE_INTEGER k, u;
	k.LowPart = kernel.dwLowDateTime;
	k.HighPart = kernel.dwHighDateTime;
	u.LowPart = user.dwLowDateTime;
	u.HighPart = user.dwHighDateTime;

	// FILETIME = 100 ns units
	return (k.QuadPart + u.QuadPart) * 100;
}
uint64_t Timer2::reset()
{
	auto elapsed = sinceStart();
	start = getCurrentCpuTime();
	return elapsed;
}
uint64_t Timer2::sinceStart()
{
	return getCurrentCpuTime() - start;
}
	*/