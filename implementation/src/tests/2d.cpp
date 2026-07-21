#include "2d.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <CGAL/constructions_d.h>
#include <fstream>
#include <CGAL/Point_2.h>
#include <CGAL/squared_distance_2.h>
#include "../2D/mode_naive.h"
#include "testFuncs.h"
#include "dataReader.h"

// TODO: there is a lot of duplication between 1D and 2D testing

double Tester2D::naive_range(vector<Point_2>& locations, Point_2 point, int k)
{
	vector<double> distances(locations.size());
	for (int i = 0; i < distances.size(); i++)
	{
		auto dx = abs(locations[i].x() - point.x());
		auto dy = abs(locations[i].y() - point.y());
		distances[i] = max(dx, dy);
	}
	sort(distances.begin(), distances.end());
	return distances[k - 1] + constants::epsilon;
}

// generate uniform points
std::vector<Point_2> Tester2D::generateUniformPoints(const Scenario& scenario)
{
	std::vector<Point_2> points;
	for (int i = 0; i < scenario.numPoints; i++)
		points.push_back(Point_2(random.nextDouble(scenario.min, scenario.max), random.nextDouble(scenario.min, scenario.max)));
	return points;
}

std::vector<Color_> Tester2D::generateGroupedColors(const Scenario& scenario, std::vector<Point_2> points)
{
	std::vector<Color_> colors = std::vector<Color_>(scenario.numPoints, -1);

	// Fix gamma points
	std::vector<std::pair<Point_2, Color_>> gamma_colors;
	for (int i = 0; i < scenario.gamma; i++)
	{
		int index = random.nextInt(0, scenario.numPoints);
		Color_ col = random.nextInt(0, scenario.numColors);
		gamma_colors.push_back(std::make_pair(points[index], col));
	}

	// Then, fill in rest
	for (int i = 0; i < scenario.numPoints; i++)
	{
		if (random.nextDouble(0, 1) >= scenario.alpha)
		{
			// use uniform sample
			colors[i] = random.nextInt(0, scenario.numColors);
		}
		else
		{
			// find closest in gamma list, otherwise fall back to uniform
			// O(n*gamma) implementation, but since this is setup doesn't really matter
			Color_ closest = random.nextInt(0, scenario.numColors);
			double closestDistance = DBL_MAX;
			for (int j = 0; j < gamma_colors.size(); j++)
			{
				double distance = CGAL::squared_distance(points[i], gamma_colors[j].first);
				if (distance < closestDistance)
				{
					closestDistance = distance;
					closest = gamma_colors[j].second;
				}
			}
			colors[i] = closest;
		}
	}

	TestFuncs::fixColors(scenario, colors, random);

	return colors;
}

std::vector<Color_> Tester2D::generateCheckerboardColors(Scenario& scenario, std::vector<Point_2> points)
{
	std::vector<Color_> colors;

	std::vector<double> xsorted, ysorted;
	for (Point_2 p : points) {
		xsorted.push_back(p.x());
		ysorted.push_back(p.y());
	}
	std::sort(xsorted.begin(), xsorted.end());
	std::sort(ysorted.begin(), ysorted.end());

	std::vector<double> xstripBoundaries, ystripBoundaries;
	for (int i = 1; i < scenario.checkerStrips; i++) {
		xstripBoundaries.push_back(xsorted[i * (double)(points.size() / scenario.checkerStrips)]);
		ystripBoundaries.push_back(ysorted[i * (double)(points.size() / scenario.checkerStrips)]);
	}

	std::map<std::tuple<int, int>, Color_> colorMapping;
	int nrColors = 0;
	for (int i = 0; i < points.size(); i++) {
		int stripx = std::distance(xstripBoundaries.begin(), std::lower_bound(xstripBoundaries.begin(), xstripBoundaries.end(), points[i].x()));
		int stripy = std::distance(ystripBoundaries.begin(), std::lower_bound(ystripBoundaries.begin(), ystripBoundaries.end(), points[i].y()));
		if (!colorMapping.count(tuple(stripx, stripy))) {
			colorMapping[tuple(stripx, stripy)] = nrColors++;
		}
		colors.push_back(colorMapping[tuple(stripx, stripy)]);
	}
	scenario.numColors = nrColors;

	return colors;
}

std::vector<vector<KQuery2D>> Tester2D::generateUniformKQueries(const Scenario& scenario)
{
	std::vector<vector<KQuery2D>> allQueries;
	for (int k : scenario.ks)
	{
		std::vector<KQuery2D> queries;
		double scenarioRange = scenario.max - scenario.min;
		double min = scenario.min - 0.1 * scenarioRange, max = scenario.max + 0.1 * scenarioRange;
		for (int i = 0; i < scenario.numQueries; i++)
		{
			queries.push_back(KQuery2D{ Point_2(random.nextDouble(min, max), random.nextDouble(min, max)), k });
		}
		allQueries.push_back(queries);
	}
	return allQueries;
}

void Tester2D::checkAnswer(const Range2D& query, const ColorCount& answer, const vector<Color_>& colors, MyRangeTree& tree)
{
	auto pointsInRange = tree.rangeReport(query);

	std::map<Color_, int> colorCounts;
	for (ColoredPoint_2& point : pointsInRange)
		colorCounts[point.color]++;

	ColorCount max = maxCount(colorCounts);

	if (answer.count != max.count || colorCounts[answer.color] != answer.count || answer.color == -1)
		throw std::runtime_error("wrong answer");
	// if (answer.count != max.count) std::cout << "(wrong color)";
	// if (colorCounts[answer.color] != answer.count) std::cout << "(wrong count)";
}
namespace
{
	bool is_correct(const ColorCount& answer, unsigned int expected_count, std::vector<Color_> const& expected_colors)
	{
		if (answer.count != expected_count)
			return false;
		if (!std::binary_search(begin(expected_colors), end(expected_colors), answer.color))
			return false;
		return true;
	}

	std::pair<unsigned int, std::vector<Color_>> correct_answers(const Range2D& query, MyRangeTree& tree)
	{
		auto pointsInRange = tree.rangeReport(query);
		std::map<Color_, int> colorCounts;
		for (ColoredPoint_2& point : pointsInRange)
			colorCounts[point.color]++;
		vector<Color_> most_occurring;
		unsigned int max_count = 0;
		std::for_each(begin(colorCounts), end(colorCounts), [&max_count, &most_occurring](map<Color_, int>::value_type const& cc)
			{
				if (cc.second > max_count) {
					most_occurring.clear();
					max_count = cc.second;
				}
				if (cc.second == max_count) {
					most_occurring.push_back(cc.first);
				} });
				std::sort(most_occurring.begin(), most_occurring.end());
				return make_pair(max_count, std::move(most_occurring));
	}
}

//verify correctness of both range finding and mode finding
void Tester2D::checkCorrectness()
{
	bool debugTimes = false;

	std::cout << "Checking 2D correctness..." << std::endl;

	for (int i = 3; i < 25; i++)
	{
		Scenario scenario{};
		scenario.synthetic = true;
		scenario.numPoints = std::pow(2, i);
		std::cout << "n = " << scenario.numPoints << " (phi = ";

		int nrColorRuns = 5;
		for (int j = 1; j <= nrColorRuns; j++) {
			scenario.numColors = std::max(scenario.numPoints * j / nrColorRuns, 1);
			scenario.min = 0;
			scenario.max = 10;

			std::cout << scenario.numColors << ",";

			scenario.ks = { 1, 10, 50, 100, 1000, 5000, 10000, 30000, 100000, 300000, 800000 };
			scenario.gamma = 0;
			scenario.alpha = 0;
			scenario.numQueries = 10000;

			scenario.isValid(); // clear out invalid k values

			vector<Point_2> points = generateUniformPoints(scenario);
			vector<Color_> colors = generateGroupedColors(scenario, points);

			std::vector<std::vector<KQuery2D>> allQueries = generateUniformKQueries(scenario);
			std::vector<Range2D> rangeQueries;

			RangeQueryDS2D rangeDS(points, colors);

			//std::cout << "Range queries.. (";
			int nrQuery = 0;
			for (std::vector<KQuery2D>& queries : allQueries)
				for (KQuery2D& query : queries)
				{
					auto naiveRadius = naive_range(points, query.point, query.k);
					auto smartRadius = rangeDS.query_k_nearest(query.point, query.k);
					Range2D range(query.point, naiveRadius);
					int count = rangeDS.tree.rangeCount(range);
					if (std::abs(naiveRadius - smartRadius) > constants::epsilon || count != query.k) {
						std::cout << "2D range finding incorrect";
						throw std::runtime_error("2D range finding incorrect");
					}
					rangeQueries.push_back(range);
					//if (nrQuery++ % (scenario.ks.size() * scenario.numQueries / 10) == 0)
					//	std::cout << (int)((double)nrQuery / (scenario.ks.size() * scenario.numQueries) * 100) << "%, ";
				}

			Timer timer;

			//std::cout << ") succesful." << std::endl
			//	<< "Testing mode queries.. (";
			nrQuery = 0;
			int s = (int)std::pow(colors.size(), 1.0 / 3);
			std::vector<std::unique_ptr<AAS2DModeDS>> datastructs;
			datastructs.push_back(std::make_unique<AAS2DModeReportDS>(points, colors));
			datastructs.push_back(std::make_unique<AAS2DModePerColorDS>(points, colors, scenario.numColors));
			datastructs.push_back(std::make_unique<AAS2DModeGridDS>(points, colors, s, scenario.numColors));
			//datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridNoColor, s));
			//datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridSelectColor, s));
			//datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridColumnColor, s));
			//datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridSortedColor, s));
			for (Range2D& rangeQuery : rangeQueries)
			{
				auto expected = correct_answers(rangeQuery, rangeDS.tree);
				for (auto& ds : datastructs)
				{
					timer.reset();
					auto res = ds->queryMode(rangeQuery);
					if (debugTimes) std::cout << "(" << ds->getName() << ": " << timer.sinceStart().count() << ") ";
					if (!is_correct(res, expected.first, expected.second))
					{
						std::cout << "Error in algorithm " << ds->getName() << "\n";
						throw runtime_error("Incorrect answer 1");
					}
					checkAnswer(rangeQuery, res, colors, rangeDS.tree);
				}
				if (debugTimes) std::cout << std::endl;
				//if (nrQuery++ % (rangeQueries.size() / 10) == 0)
				//	std::cout << (int)((double)nrQuery / rangeQueries.size() * 100) << "%, ";
			}
		}
		std::cout << ") succesful" << std::endl;
	}
}

//create ipe files to visualize the different types of point sets
void Tester2D::visualizePointsets() {
	Scenario scenario;
	scenario.numPoints = 1000;
	scenario.numColors = 6;
	scenario.alpha = 0;
	scenario.gamma = 0;
	scenario.min = 100;
	scenario.max = 200;
	scenario.synthetic = true;

	Dataset2D data;
	data.points = generateUniformPoints(scenario);
	data.colors = TestFuncs::generateUniformColors(scenario, random);
	data.writeToIpe("uniform.ipe");

	scenario.gamma = 10;
	scenario.alpha = .9;
	data.points = generateUniformPoints(scenario);
	data.colors = generateGroupedColors(scenario, data.points);
	data.writeToIpe("grouped.ipe");

	scenario.checkerStrips = 3;
	data.points = generateUniformPoints(scenario);
	data.colors = generateCheckerboardColors(scenario, data.points);
	data.writeToIpe("checker.ipe");


	scenario.synthetic = false;

	scenario.filename = "osm/1.points";
	data = DataReader::readPointsFile(scenario, random);
	data.trainTestSample(0.1, random.gen).first.writeToIpe("1.ipe", 500);

	scenario.filename = "osm/bbg.points";
	data = DataReader::readPointsFile(scenario, random);
	data.trainTestSample(0.1, random.gen).first.writeToIpe("bbg.ipe", 500);

	scenario.d0 = 1;
	scenario.d1 = 3;
	scenario.filename = "openML/php3CTpvq.arff";
	data = DataReader::readOpenML(scenario, random);
	data.trainTestSample(0.01, random.gen).first.writeToIpe("php3CTpvq.ipe", 500);


	scenario.filename = "temperature/temperature-11-06-2024.points";
	data = DataReader::readPointsFile(scenario, random);
	data.trainTestSample(1, random.gen).first.writeToIpe("temperature-11-06-2024.ipe", 500);
}

TestResult Tester2D::runScenario(Scenario& scenario, std::function<std::unique_ptr<AAS2DModeDS>(std::vector<Point_2>&, std::vector<Color_>&, Scenario&)> makeDS)
{
	Dataset2D data;

	if (scenario.synthetic)
	{
		data.points = generateUniformPoints(scenario);

		if (scenario.gamma == 0 || scenario.alpha == 0)
			data.colors = TestFuncs::generateUniformColors(scenario, random);
		else if (scenario.checkerStrips != 0)
			data.colors = generateCheckerboardColors(scenario, data.points);
		else
			data.colors = generateGroupedColors(scenario, data.points);
	}
	else
	{
		if (scenario.filename.length() >= 5 && 0 == scenario.filename.compare(scenario.filename.length() - 5, 5, ".arff"))
		{
			data = DataReader::readOpenML(scenario, random);
		}
		else if (scenario.filename.length() >= 7 && 0 == scenario.filename.compare(scenario.filename.length() - 7, 7, ".points"))
		{
			data = DataReader::readPointsFile(scenario, random);
		}
	}

	return runDsOnPoints(scenario, data, makeDS);
}

TestResult Tester2D::runDsOnPoints(Scenario& scenario, Dataset2D data, std::function<std::unique_ptr<AAS2DModeDS>(std::vector<Point_2>&, std::vector<Color_>&, Scenario&)> makeDS)
{
	std::vector<std::vector<KQuery2D>> allQueries;
	if (scenario.synthetic)
		allQueries = generateUniformKQueries(scenario);
	else
	{ // subsample some points to separate into train set (DS) and test set (queries)
		auto trainTest = data.trainTestSample(scenario.trainFrac, random.gen);
		Dataset2D train = trainTest.first, test = trainTest.second;
		data = train;
		scenario.numPoints = train.points.size();
		scenario.s = (int)std::pow(scenario.numPoints, 1.0 / 3);
		scenario.numColors = train.numColors;
		if (!scenario.isValid()) { //if numColors > n ; too lazy to actually recompute numColors for train set
			TestResult res;
			res.error = true;
			return res;
		}
		for (int k : scenario.ks)
		{
			std::vector<KQuery2D> queries;
			for (int i = 0; i < std::min((int)test.points.size(), scenario.numQueries); i++)
			{
				queries.push_back(KQuery2D{ test.points[i], k });
			}
			allQueries.push_back(queries);
		}
		scenario.numQueries = allQueries[0].size();

	}

	Timer totalTime;
	std::cout << "Running scenario " << scenario.fancyToString() << " with ds: ";

	TestResult res{};

	// build the range DS
	Timer timer{}, timer2{};
	RangeQueryDS2D rangeDS(data.points, data.colors);
	res.rangeBuildTime = timer.reset();
	res.rangeSpace = rangeDS.get2DRangeMemoryUsage();

	// build the mode datastructure
	timer.reset();
	auto ds = makeDS(data.points, data.colors, scenario);
	res.modeBuildTime = timer.reset();
	res.modeSpace = ds->getDSMemoryUsage();

	res.dsType = ds->getName();
	std::cout << res.dsType;

	for (std::vector<KQuery2D>& queries : allQueries)
	{
		std::vector<Range2D> rangeQueries;
		timer.reset();
		for (KQuery2D& query : queries)
		{
			double queryRadius = rangeDS.query_k_nearest(query.point, query.k);
			Range2D queryRange(query.point, queryRadius);
			rangeQueries.push_back(queryRange);

		}
		res.rangeAverageQueryTimes.push_back(timer.reset() / scenario.numQueries);

		int nrQueries = 0;
		int fractionToDiscard = 10;
		for (Range2D& query : rangeQueries)
		{
			ColorCount ans = ds->queryMode(query);

			nrQueries++;
			if (nrQueries == scenario.numQueries / fractionToDiscard)
				timer2.reset(); //don't count the first x% of queries for caching-reasons
		}
		if (nrQueries != scenario.numQueries)
			throw exception("beep boop");
		res.modeAverageQueryTimes.push_back(timer.reset() / scenario.numQueries);
		res.modeAverageQueryTimesExcludingFirst.push_back(timer2.reset() / (scenario.numQueries * (fractionToDiscard - 1) / fractionToDiscard));
	}

	std::cout << " (" << totalTime.sinceStart().count() / 1e9 << " sec)" << std::endl;
	return res;
}

//prepare and run all 2D experiments
void Tester2D::run()
{
	// parameters are: ns, rs, ks, numCols, defaultPower
	// s = -1 means n^{defaultPower}

	//synthetic uniform random scenarios
	std::vector<int> /*professor doctor*/ testks = { 10, 100, 1000, 5000, 10000, 25000, 50000, 75000,  100000, 150000, 200000, 250000, 290000, 350000, 400000, 450000, 490000 };
	std::vector<Scenario> increasingNScenarios = TestFuncs::generateScenarios(TestFuncs::range(50000, 500000, 50000), { -1 }, testks, { 50000 }, 1.0 / 3);
	std::vector<Scenario> increasingPhiScenarios = TestFuncs::generateScenarios({ 250000 }, { -1 }, testks, { 1, 100, 1000, 5000, 10000, 25000, 50000, 75000, 100000, 125000, 150000, 175000, 200000, 225000, 250000 }, 1.0 / 3);
	std::vector<Scenario> increasingSScenarios = TestFuncs::generateScenarios({ 250000 }, { 10, 20, 40, 60, 80, 100, 150, 200 }, testks, { 50000 }, 1.0 / 3);

	// grouped scenarios
	std::vector<Scenario> groupedScenarios = TestFuncs::generateScenarios({ 250000 }, { -1 }, testks, { 50000 }, 1.0 / 3, { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 }, { 5000 });
	std::vector<Scenario> checkerScenarios = TestFuncs::generateScenarios({ 250000 }, { -1 }, testks, { 50000 }, 1.0 / 3, { 0 }, { 0 }, { 222 });

	// real scenarios
	std::vector<Scenario> realScenariosWalking = TestFuncs::generateRealScenarios("openML/php3CTpvq.arff", 1, 2, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);
	//std::vector<Scenario> realScenariosRansom = TestFuncs::generateRealScenarios("openML/dataset.arff", 4, 8, { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09 }, testks); //too large to push to git; find at https://www.openml.org/search?type=data&id=42553&sort=runs&status=active

	std::vector<Scenario> scienceParkScenarios = TestFuncs::generateRealScenarios("osm/1.points", 4, 8, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);
	std::vector<Scenario> bbgScenarios = TestFuncs::generateRealScenarios("osm/bbg.points", 4, 8, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);
	std::vector<Scenario> bieleveldScenarios = TestFuncs::generateRealScenarios("osm/bieleveld.points", 4, 8, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);

	std::vector<Scenario> syntheticBbg = TestFuncs::generateScenarios(TestFuncs::range(6687, 66870, 6687), { -1 }, testks, { 11 }, 1.0 / 3);
	std::vector<Scenario> syntheticBbgGrouped = TestFuncs::generateScenarios(TestFuncs::range(6687, 66870, 6687), { -1 }, testks, { 11 }, 1.0 / 3, { 1 }, { 1000 });





	// all DS's to test
	typedef std::function<std::unique_ptr<AAS2DModeDS>(std::vector<Point_2>&, std::vector<Color_>&, Scenario&)> FuncType;

	FuncType buildReport = [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS2DModeReportDS>(points, colors); };
	FuncType buildPerColor = [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS2DModePerColorDS>(points, colors, scenario.numColors); };
	FuncType buildSimpleGrid = [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS2DModeGridDS>(points, colors, scenario.s, scenario.numColors); };
	/*FuncType f1{ [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
				{
					return std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridNoColor, scenario.s);
				} };
	FuncType f2{ [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
				{
					return std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridSelectColor, scenario.s);
				} };
	FuncType f3{ [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
				{
					return std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridColumnColor, scenario.s);
				} };
	FuncType f4{ [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
				{
					return std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridSortedColor, scenario.s);
				} };*/
	std::vector<FuncType> funcs = {
		buildReport,
		buildPerColor,
		buildSimpleGrid,
		//f1 , 
		//f2,
		//f3,
		//f4
	};

	/*std::vector<Scenario> repeatedScenarios;
	for (int i = 0; i < 100; i++)
		repeatedScenarios.push_back(increasingNScenarios[0]);*/

	auto tuples = {
		std::make_tuple("2D_increasingN", increasingNScenarios),
		std::make_tuple("2D_increasingS", increasingSScenarios),
		std::make_tuple("2D_increasingPhi", increasingPhiScenarios),

		std::make_tuple("2D_grouped", groupedScenarios),
		std::make_tuple("2D_checkers", checkerScenarios),

		std::make_tuple("2D_sciencePark", scienceParkScenarios),
		std::make_tuple("2D_bieleveld", bieleveldScenarios),
		std::make_tuple("2D_bbg", bbgScenarios),
		std::make_tuple("2D_bbgSynthetic", syntheticBbg),
		std::make_tuple("2D_bbgSyntheticGrouped", syntheticBbgGrouped),

		std::make_tuple("2D_realWalking", realScenariosWalking),
		std::make_tuple("2D_realRansom", realScenariosRansom),
	};

	for (auto run : tuples)
	{
		FileOutputter file(std::get<0>(run));
		for (Scenario& scenario : std::get<1>(run))
		{
			for (FuncType buildFunc : funcs)
			{
				try
				{
					TestResult res = runScenario(scenario, buildFunc);
					file.output(scenario, res);
				}
				catch (...)
				{
					std::cout << "ERROR for scenario" << scenario.fancyToString() << std::endl;
					TestResult res;
					res.error = true;
					file.output(scenario, res);
				}
			}
		}
	}
}
