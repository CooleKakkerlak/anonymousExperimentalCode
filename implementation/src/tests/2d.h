#pragma once
#include <vector>
#include <numeric>
#include <set>
#include "../shared/defs.h"
#include "../2D/range_query.h"
#include "../2D/mode_naive.h"
#include "../2D/grid_simple.h"

using namespace std;


typedef pair<pair<Point_2, Color_>, int> gamma_triple;

struct Tester2D {
	MyRandom random = MyRandom(0); //SEEDED

	// Naive O(n log n) sorting of all points and then taking the kth element
	double naive_range(vector<Point_2>& locations, Point_2 point, int k);

	std::vector<Point_2> generateUniformPoints(const Scenario& scenario);
	std::vector<Color_> generateGroupedColors(const Scenario& scenario, std::vector<Point_2> points);
	std::vector<Color_> generateCheckerboardColors(Scenario& scenario, std::vector<Point_2> points);


	std::vector<std::vector<KQuery2D>> generateUniformKQueries(const Scenario& scenario);

	//std::vector<Range> generateUniformModeQueries(const Scenario& scenario);


public:
	void checkAnswer(const Range2D& query, const ColorCount& answer, const vector<Color_>& colors, MyRangeTree& tree);


	void run();

	void checkCorrectness();
	void visualizePointsets();


	TestResult runScenario(Scenario& scenario, std::function<std::unique_ptr<AAS2DModeDS>(std::vector<Point_2>&,std::vector<Color_>&, Scenario&)> makeDS);
	TestResult runDsOnPoints(Scenario& scenario, Dataset2D data, std::function<std::unique_ptr<AAS2DModeDS>(std::vector<Point_2>&,std::vector<Color_>&, Scenario&)> makeDS);

	void run_1d_real();
};

