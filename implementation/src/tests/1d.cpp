#include "1d.h"

#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <numeric>
#include <string>
#include <iostream>
#include <fstream>
#include "../shared/defs.h"
#include "../1D/mode_arrangement.h"
#include "../1D/mode_rangeTree.h"
#include "../1D/mode_chan.h"
#include "../1D/range_query.h"
#include "testFuncs.h"
#include "float.h"

MyRandom my_random = MyRandom(0); // SEEDED

std::vector<std::string> Tester1D::split(std::string str)
{
	std::vector<std::string> substrings = {};
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, ';'))
	{
		substrings.push_back(token);
	}

	return substrings;
}

std::pair<std::vector<double>, std::vector<Color_>> Tester1D::read_file(std::string filename, int dim)
{
	std::string text;
	std::vector<Point> points;

	std::ifstream file(filename);

	while (std::getline(file, text))
	{
		// determine dimensionality of line
		std::vector<std::string> args = split(text);
		int dims = args.size() - 1;

		points.push_back({ std::stod(args[dim]), (Color_)std::stoi(args[args.size() - 1]) });
	}

	file.close();

	auto compare = [](const Point& a, const Point& b)
		{
			return a.pos < b.pos;
		};
	std::sort(points.begin(), points.end(), compare);

	std::vector<double> coords;
	std::vector<Color_> colors;
	for (Point p : points)
	{
		coords.push_back(p.pos);
		colors.push_back(p.color);
	}

	return std::make_pair(coords, colors);
}

std::vector<Color_> Tester1D::generateGroupedColors(const Scenario& scenario, std::vector<double>& points)
{
	std::vector<Color_> colors = std::vector<Color_>(scenario.numPoints, -1);

	// Fix gamma points
	std::vector<std::pair<double, Color_>> gamma_colors;
	for (int i = 0; i < scenario.gamma; i++)
	{
		int index = my_random.nextInt(0, scenario.numPoints);
		Color_ col = my_random.nextInt(0, scenario.numColors);
		gamma_colors.push_back(std::make_pair(points[index], col));
	}

	// Then, fill in rest
	for (int i = 0; i < scenario.numPoints; i++)
	{
		if (my_random.nextDouble(0, 1) >= scenario.alpha)
		{
			// use uniform sample
			colors[i] = my_random.nextInt(0, scenario.numColors);
		}
		else
		{
			// find closest in gamma list, otherwise fall back to uniform
			// O(n*gamma) implementation, but since this is setup doesn't really matter
			Color_ closest = -1;
			double closestDistance = DBL_MAX;
			for (int j = 0; j < gamma_colors.size(); j++)
			{
				double distance = abs(points[i] - gamma_colors[j].first);
				if (distance < closestDistance)
				{
					closestDistance = distance;
					closest = gamma_colors[j].second;
				}
			}
			colors[i] = closest;
		}
	}

	TestFuncs::fixColors(scenario, colors, my_random);

	return colors;
}

// generate uniform points
std::vector<double> Tester1D::generateUniformPoints(const Scenario& scenario)
{
	std::vector<double> points;
	for (int i = 0; i < scenario.numPoints; i++)
		points.push_back(my_random.nextDouble(scenario.min, scenario.max));
	std::sort(points.begin(), points.end());
	return points;
}

std::vector<std::vector<KQuery>> Tester1D::generateUniformKQueries(const Scenario& scenario)
{
	std::vector<std::vector<KQuery>> allQueries;

	for (int k : scenario.ks)
	{
		std::vector<KQuery> queries;
		// generate queries
		double scenarioRange = scenario.max - scenario.min;
		double min = scenario.min - 0.1 * scenarioRange, max = scenario.max + 0.1 * scenarioRange;
		for (int i = 0; i < scenario.numQueries; i++)
		{
			queries.push_back(KQuery{ my_random.nextDouble(min, max), k });
		}
		allQueries.push_back(queries);
	}
	return allQueries;
}

void Tester1D::checkAnswer(const Range& query, const ColorCount& answer, const std::vector<Color_>& colors, AAS1DModeReportDS reportDS)
{
	ColorCount naiveRes = reportDS.queryMode(query);
	if (answer.count != naiveRes.count)
		throw std::runtime_error("wrong answer");

	int count = 0;
	for (int i = 0; i < colors.size(); i++)
	{
		if (query.left <= i && i < query.right && colors[i] == answer.color)
		{
			count++;
		}
	}
	if (answer.count != count)
		throw std::runtime_error("wrong answer");
}

void Tester1D::checkCorrectness()
{
	std::cout << "Checking 1D correctness.." << std::endl;

	for (int i = 0; i < 17; i++) {
		Scenario scenario{};
		scenario.numPoints = std::pow(2, i);
		scenario.numColors = std::min(scenario.numPoints, std::max(2, scenario.numPoints / 100));
		scenario.gamma = 0;
		scenario.alpha = 0;
		scenario.min = 0;
		scenario.max = 10;
		scenario.numQueries = 10000;
		scenario.ks = { 1, 5, 10, 50, 100 , 500, 1000, 5000, 9000 };
		scenario.isValid();
		auto points = generateUniformPoints(scenario);
		auto colors = generateGroupedColors(scenario, points);

		std::cout << "run " << i << ", n = " << scenario.numPoints << std::endl;

		// check range correctness
		auto tree = DS::generate_tree<double>(points);

		std::cout << "Range queries.. (";
		int nrQuery = 0;
		std::vector<std::vector<KQuery>> allKqueries = generateUniformKQueries(scenario);
		std::vector<Range> rangeQueries;

		for (std::vector<KQuery>& kqueries : allKqueries)
			for (KQuery& kquery : kqueries)
			{
				Range naiveRange = DS::naiveRangeQuery(points, kquery);
				Range range = DS::queryKRange(tree, points, kquery);

				rangeQueries.push_back(range);
				if (naiveRange.left != range.left || naiveRange.right != range.right)
					throw std::runtime_error("range finding incorrect");
				if (nrQuery++ % (scenario.ks.size() * scenario.numQueries / 10) == 0)
					std::cout << (int)((double)nrQuery / (scenario.ks.size() * scenario.numQueries) * 100) << "%, ";
			}


		std::cout << ") succesful." << std::endl
			<< "Testing mode queries.. (";
		nrQuery = 0;
		std::vector<std::unique_ptr<AAS1DModeDS>> datastructs;

		// check mode correctness
		AAS1DModeReportDS reportDS(colors, scenario.numColors);
		datastructs.push_back(std::make_unique<AAS1DModeRangeTreesDS>(colors, scenario.numColors));
		datastructs.push_back(std::make_unique<AAS1DModeReportDS>(colors, scenario.numColors));
		datastructs.push_back(std::make_unique<AAS1DModeChanDS>(colors, scenario.numColors, std::sqrt(colors.size())));
		datastructs.push_back(std::make_unique<AAS1DModeArrangementDS>(colors, scenario.numColors, std::sqrt(colors.size()), my_random.gen));
		datastructs.push_back(std::make_unique<AAS1DModeGridDS>(colors, scenario.numColors, sqrt(colors.size())));

		for (const Range& query : rangeQueries)
		{
			for (auto& ds : datastructs)
			{
				ColorCount res = ds->queryMode(query);
				checkAnswer(query, res, colors, reportDS);
			}
			if (nrQuery++ % (rangeQueries.size() / 10) == 0)
				std::cout << (int)((double)nrQuery / rangeQueries.size() * 100) << "%, ";
		}
		std::cout << ") succesful." << std::endl;
	}
}

void Tester1D::runScenario(Scenario& scenario, FuncType makeDS, FileOutputter& file)
{
	std::vector<double> points = generateUniformPoints(scenario);

	std::vector<Color_> colors;

	if (scenario.gamma == 0 || scenario.alpha == 0)
		colors = TestFuncs::generateUniformColors(scenario, my_random);
	else
		colors = generateGroupedColors(scenario, points);

	runDSOnPoints(scenario, points, colors, makeDS, file);
}

void Tester1D::runDSOnPoints(Scenario& scenario, std::vector<double>& points, std::vector<Color_>& colors, FuncType makeDS, FileOutputter& file)
{
	Timer totalTime;
	std::cout << "Running " << scenario.fancyToString() << " with ds: ";

	std::vector<std::vector<KQuery>> allKQueries = generateUniformKQueries(scenario);

	TestResult res{};

	// build the range query tree
	Timer timer{};
	auto tree = DS::generate_tree<double>(points);
	res.rangeBuildTime = timer.reset();
	res.rangeSpace = DS::get1DRangeMemoryUsage(tree);

	// build the mode datastructure
	timer.reset();
	auto ds = makeDS(colors, scenario);
	res.modeBuildTime = timer.reset();
	res.modeSpace = ds->getDSMemoryUsage();

	res.dsType = ds->getName();
	std::cout << res.dsType;

	for (std::vector<KQuery> kQueries : allKQueries)
	{
		std::vector<Range> rangeQueries;
		timer.reset();
		for (KQuery query : kQueries)
		{
			Range queryRange = DS::queryKRange(tree, points, query);
			rangeQueries.push_back(queryRange);
		}
		res.rangeAverageQueryTimes.push_back(timer.reset() / scenario.numQueries);
		for (Range query : rangeQueries)
		{
			ColorCount ans = ds->queryMode(query);
		}
		res.modeAverageQueryTimes.push_back(timer.reset() / scenario.numQueries);
	}

	std::cout << " (" << totalTime.sinceStart().count() / 1e9 << " sec)" << std::endl;
	file.output(scenario, res);

	DS::free_tree(tree, true);
}

void Tester1D::run()
{
	// experimental tests
	//												prameters:	ns, rs, ks, numCols, defaultPower (, alpha, gamma)
	// s = -1 means default n^{defaultPower}
	std::vector<int> ks = { 10, 100, 1000, 10000, 25000, 50000, 75000, 100000, 125000, 150000, 175000, 200000 };
	std::vector<int> arrKs = { 10, 100, 1000, 2500, 5000, 7500, 10000, 12500, 15000, 17500, 20000 };	

	std::vector<Scenario> increasingNScenarios = TestFuncs::generateScenarios(TestFuncs::range(25000, 200000, 25000), { -1 }, ks, { 10000 }, .5);					// increasing n
	std::vector<Scenario> increasingPhiScenarios = TestFuncs::generateScenarios({ 100000 }, { -1 }, ks, { 1, 10, 100, 1000, 5000, 10000, 25000, 50000, 75000, 100000 }, .5);							// increasing numCols
	std::vector<Scenario> increasingSScenarios = TestFuncs::generateScenarios({ 100000 }, { 25, 50, 100, 150, 200, 300, 400, 500, 600 }, ks, { 1000 }, .5); // increasing s

	// arrangement version of the above (smaller)
	std::vector<Scenario> arrIncreasingNScenarios = TestFuncs::generateScenarios(TestFuncs::range(2500, 30000, 2500), { -1 }, arrKs, { 1000 }, .5);				   // increasing n
	std::vector<Scenario> arrIncreasingPhiScenarios = TestFuncs::generateScenarios({ 10000 }, { -1 }, arrKs, { 1, 10, 100, 500, 1000, 2500, 5000, 7500, 10000 }, .5);						   // increasing numCols
	std::vector<Scenario> arrIncreasingSScenarios = TestFuncs::generateScenarios({ 10000 }, { 20, 40, 70, 100, 150, 200, 250, 300 }, arrKs, { 1000 }, .5); // increasing s

	// grouped scenarios
	std::vector<Scenario> groupedScenarios = TestFuncs::generateScenarios({ 100000 }, { -1 }, ks, { 10000 }, .5, { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 }, { 5000 });	// grouped
	std::vector<Scenario> arrGroupedScenarios = TestFuncs::generateScenarios({ 10000 }, { -1 }, arrKs, { 1000 }, .5, { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 }, { 500 });		// grouped but smaller


	// use one of these functions as parameter to runScenario to build a specific ds
	FuncType buildReport = [](std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS1DModeReportDS>(colors, scenario.numColors); };
	FuncType buildRangeTree = [](std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS1DModeRangeTreesDS>(colors, scenario.numColors); };
	FuncType buildChan = [](std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS1DModeChanDS>(colors, scenario.numColors, scenario.s); };
	FuncType buildGrid = [](std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS1DModeGridDS>(colors, scenario.numColors, scenario.s); };
	std::vector<FuncType> allBuildFuncs = { buildReport, buildRangeTree, buildChan, buildGrid };
	std::vector<FuncType> increasingSBuildFuncs = { buildChan, buildGrid };

	FuncType buildArrangement = [this](std::vector<Color_>& colors, Scenario& scenario)
		{ return std::make_unique<AAS1DModeArrangementDS>(colors, scenario.numColors, scenario.s, random.gen); };

	auto tuples = {
		std::make_tuple("1DincreasingN", increasingNScenarios, arrIncreasingNScenarios, allBuildFuncs),
		std::make_tuple("1DincreasingS", increasingSScenarios, arrIncreasingSScenarios, increasingSBuildFuncs),
		std::make_tuple("1DincreasingPhi", increasingPhiScenarios, arrIncreasingPhiScenarios, allBuildFuncs),
		std::make_tuple("1DgroupedIncreasingAlpha", groupedScenarios, arrGroupedScenarios, allBuildFuncs),
	};

	for (auto run : tuples)
	{
		FileOutputter file(std::get<0>(run));
		for (Scenario& scenario : std::get<1>(run))
		{
			for (FuncType buildFunc : std::get<3>(run)) {
				try {
					runScenario(scenario, buildFunc, file);
				}
				catch(...){
					std::cout << "ERROR for scenario" << scenario.fancyToString() << std::endl;
					TestResult res;
					res.error = true;
					file.output(scenario, res);
				}
			}
		}
		for (Scenario& scenario : std::get<2>(run))
		{
			try {
				runScenario(scenario, buildArrangement, file);
			}
			catch (...) {
				std::cout << "ERROR for scenario" << scenario.fancyToString() << std::endl;
				TestResult res;
				res.error = true;
				file.output(scenario, res);
			}
		}
	}
}
