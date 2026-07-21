#pragma once

#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include <string>
#include <sstream>
#include <fstream> 
#include <iomanip>
#include "../shared/defs.h"
#include "../1D/mode_arrangement.h"
#include "../1D/mode_rangeTree.h"
#include "../1D/mode_chan.h"





typedef struct { unsigned int begin; unsigned int end; } IndexRange;
typedef struct { double pos; Color_ color; } Point;


typedef std::function<std::unique_ptr<AAS1DModeDS>(std::vector<Color_>&, Scenario&)> FuncType;

struct Tester1D {
	MyRandom random = MyRandom(0); //SEEDED

	std::vector<std::string> split(std::string str);


	std::pair<std::vector<double>, std::vector<Color_>> read_file(std::string filename, int dim);

	std::vector<double> generateUniformPoints(const Scenario& scenario);
	std::vector<Color_> generateGroupedColors(const Scenario& scenario, std::vector<double>& points);

	std::vector<std::vector<KQuery>> generateUniformKQueries(const Scenario& scenario);

	Range get_range(std::vector<double>& input, double q, int k, int index);

public:
	void checkAnswer(const Range& query, const ColorCount& answer, const std::vector<Color_>& colors, AAS1DModeReportDS reportDS);

	void run();
	void checkCorrectness();

	void runScenario(Scenario& scenario, FuncType makeDS, FileOutputter& file);
	void runDSOnPoints(Scenario& scenario, std::vector<double>& points, std::vector<Color_>& colors, FuncType makeDS, FileOutputter& file);
};


