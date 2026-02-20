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

void Tester2D::checkCorrectness()
{
	std::cout << "Checking 2D correctness..." << std::endl;

	for (int i = 11; i < 17; i++)
	{
		Scenario scenario{};
		scenario.synthetic = true;
		scenario.numPoints = std::pow(2, i);
		scenario.numColors = std::min(scenario.numPoints, std::max(2, scenario.numPoints / 100));
		scenario.min = 0;
		scenario.max = 10;

		scenario.ks = { 1, 10, 50, 100, 1000, 5000, 9999, 10000 };
		scenario.gamma = 0;
		scenario.alpha = 0;
		scenario.numQueries = 10000;

		scenario.isValid(); // clear out invalid k values

		std::cout << "run " << i << ", n = " << scenario.numPoints << std::endl;
		vector<Point_2> points = generateUniformPoints(scenario);
		vector<Color_> colors = generateGroupedColors(scenario, points);

		std::vector<std::vector<KQuery2D>> allQueries = generateUniformKQueries(scenario);
		std::vector<Range2D> rangeQueries;

		RangeQueryDS2D rangeDS(points, colors);

		std::cout << "Range queries.. (";
		int nrQuery = 0;
		for (std::vector<KQuery2D>& queries : allQueries)
			for (KQuery2D& query : queries)
			{
				auto naiveRadius = naive_range(points, query.point, query.k);
				auto smartRadius = rangeDS.query_k_nearest(query.point, query.k);
				Range2D range(query.point, naiveRadius);
				int count = rangeDS.tree.rangeCount(range);
				if (std::abs(naiveRadius - smartRadius) > constants::epsilon || count != query.k)
					throw std::runtime_error("2D range finding incorrect");
				rangeQueries.push_back(range);
				if (nrQuery++ % (scenario.ks.size() * scenario.numQueries / 10) == 0)
					std::cout << (int)((double)nrQuery / (scenario.ks.size() * scenario.numQueries) * 100) << "%, ";
			}

		std::cout << ") succesful." << std::endl
			<< "Testing mode queries.. (";
		nrQuery = 0;
		std::vector<std::unique_ptr<AAS2DModeDS>> datastructs;
		datastructs.push_back(std::make_unique<AAS2DModeReportDS>(points, colors));
		datastructs.push_back(std::make_unique<AAS2DModePerColorDS>(points, colors, scenario.numColors));
		datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridNoColor, (int)std::pow(colors.size(), 1.0 / 3)));
		datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridSelectColor, (int)std::pow(colors.size(), 1.0 / 3)));
		datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridColumnColor, (int)std::pow(colors.size(), 1.0 / 3)));
		datastructs.push_back(std::make_unique<AAS2DModeAmcDS>(points, colors, RustDSType::GridSortedColor, (int)std::pow(colors.size(), 1.0 / 3)));
		for (Range2D& rangeQuery : rangeQueries)
		{
			auto expected = correct_answers(rangeQuery, rangeDS.tree);
			for (auto& ds : datastructs)
			{
				auto res = ds->queryMode(rangeQuery);
				if (!is_correct(res, expected.first, expected.second))
				{
					std::clog << "Error in algorithm " << ds->getName() << "\n";
					throw runtime_error("Incorrect answer 1");
				}
				// checkAnswer(rangeQuery, res, colors, rangeDS.tree);
			}
			if (nrQuery++ % (rangeQueries.size() / 10) == 0)
				std::cout << (int)((double)nrQuery / rangeQueries.size() * 100) << "%, ";
		}
		std::cout << ") succesful" << std::endl;
	}
}

TestResult Tester2D::runScenario(Scenario& scenario, std::function<std::unique_ptr<AAS2DModeDS>(std::vector<Point_2>&, std::vector<Color_>&, Scenario&)> makeDS)
{
	Dataset2D data;

	if (scenario.synthetic)
	{
		data.points = generateUniformPoints(scenario);

		if (scenario.gamma == 0 || scenario.alpha == 0)
			data.colors = TestFuncs::generateUniformColors(scenario, random);
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
	Timer timer{};
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

	//std::map<int, UuQueryStatistics> averages;
	//for (int k : scenario.ks) averages[k] = UuQueryStatistics();
	//AAS2DModeAmcDS* amcDS = dynamic_cast<AAS2DModeAmcDS*>(ds.get());
	//amcDS->solver->set_statistics_gathering(true);

	for (std::vector<KQuery2D>& queries : allQueries)
	{
		std::vector<Range2D> rangeQueries;
		//int k = queries[0].k;
		timer.reset();
		for (KQuery2D& query : queries)
		{
			double queryRadius = rangeDS.query_k_nearest(query.point, query.k);
			Range2D queryRange(query.point, queryRadius);
			rangeQueries.push_back(queryRange);
		}
		res.rangeAverageQueryTimes.push_back(timer.reset() / scenario.numQueries);
		for (Range2D& query : rangeQueries)
		{
			ColorCount ans = ds->queryMode(query);

			//auto stats = amcDS->solver->get_last_statistics();
			//averages[k].cell_time += stats->cell_time;
			//averages[k].range_search_time += stats->range_search_time;
			//averages[k].relevant_colors += stats->relevant_colors;
			//averages[k].slab_time += stats->slab_time;
		}
		res.modeAverageQueryTimes.push_back(timer.reset() / scenario.numQueries);
	}

	/*std::cout << std::endl << "cell_time, range_search_time, relevant_colors, slab_time" << std::endl;
	for (int k : scenario.ks) {
		std::cout  << k << ", " << averages[k].cell_time.count() << ", " << averages[k].range_search_time.count() << ", " << averages[k].relevant_colors << ", " << averages[k].slab_time.count() << std::endl;
	}*/

	std::cout << " (" << totalTime.sinceStart().count() / 1e9 << " sec)" << std::endl;
	return res;
}

void Tester2D::run()
{
	// parameters are: ns, rs, ks, numCols, defaultPower
	// s = -1 means n^{defaultPower}

	//synthetic uniform random scenarios
	std::vector<int> /*professor doctor*/ testks = { 1, 3, 10, 30, 100, 300, 1000, 3000, 10000, 20000, 40000, 60000, 80000, 100000, 120000, 140000, 150000 };
	std::vector<Scenario> increasingNScenarios = TestFuncs::generateScenarios(TestFuncs::range(10000, 150000, 10000), { -1 }, testks, { 10000 }, 1.0 / 3);
	std::vector<Scenario> increasingPhiScenarios = TestFuncs::generateScenarios({ 100000 }, { -1 }, testks, { 1, 100, 1000, 5000, 10000, 20000, 30000, 40000,50000, 60000, 70000, 80000, 90000, 100000 }, 1.0 / 3); // increasing numCols
	std::vector<Scenario> increasingSScenarios = TestFuncs::generateScenarios({ 100000 }, { 10, 20, 40, 60, 80, 100, 150 }, testks, { 10000 }, 1.0 / 3);												  // increasing s

	// grouped scenarios
	std::vector<Scenario> groupedScenarios = TestFuncs::generateScenarios({ 100000 }, { -1 }, testks, { 10000 }, 1.0 / 3, { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 }, { 5000 }); // grouped

	// real scenarios
	std::vector<Scenario> realScenariosWalking = TestFuncs::generateRealScenarios("openML/php3CTpvq.arff", 1,2, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);
	std::vector<Scenario> realScenariosRansom = TestFuncs::generateRealScenarios("openML/dataset.arff", 4, 8, { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09 }, testks);

	std::vector<Scenario> scienceParkScenarios = TestFuncs::generateRealScenarios("osm/1.points", 4, 8, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);
	std::vector<Scenario> bbgScenarios = TestFuncs::generateRealScenarios("osm/bbg.points", 4, 8, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);
	std::vector<Scenario> bieleveldScenarios = TestFuncs::generateRealScenarios("osm/bieleveld.points", 4, 8, { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 }, testks);
	

	// all DS's to test
	typedef std::function<std::unique_ptr<AAS2DModeDS>(std::vector<Point_2>&, std::vector<Color_>&, Scenario&)> FuncType;

	FuncType buildReport = [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS2DModeReportDS>(points, colors); };
	FuncType buildPerColor = [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS2DModePerColorDS>(points, colors, scenario.numColors); };
	FuncType f1{ [](std::vector<Point_2>& points, std::vector<Color_>& colors, Scenario& scenario)
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
				} };
	std::vector<FuncType> funcs = { buildReport, buildPerColor, f1, f2, f3, f4 };

	auto tuples = {
		std::make_tuple("2D_increasingN", increasingNScenarios),
		std::make_tuple("2D_increasingPhi", increasingPhiScenarios),
		std::make_tuple("2D_increasingS", increasingSScenarios),

		std::make_tuple("2D_grouped", groupedScenarios),

		std::make_tuple("2D_sciencePark", scienceParkScenarios),
		std::make_tuple("2D_bbg", bbgScenarios),
		std::make_tuple("2D_bieleveld", bieleveldScenarios),

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
