#pragma once
#include <vector>
#include "../shared/defs.h"

#include <atomic>
#include <new>
#include <cstdlib>

namespace TestFuncs {
	//each point has color c with probability 1/numColors
	std::vector<Color_> generateUniformColors(const Scenario& scenario, MyRandom& random);

	//ensure that every color is used at least once
	void fixColors(const Scenario& scenario, std::vector<Color_>& colors, MyRandom& random);

	//count the number of colors numColors, and make sure the colors range from 0..numColors-1
	int countColors(std::vector<Color_>& colors);

	std::vector<int> range(int start, int stop, int stepSize);
	std::vector<Scenario> generateScenarios(std::vector<int> ns, std::vector<int> ss, std::vector<int> ks, std::vector<int> numColors, double defaultPower, std::vector<double> alphas = { 0 }, std::vector<int> gammas = { 0 });
	std::vector<Scenario> generateRealScenarios(std::string filename, int d0, int d1, std::vector<double> trainFracs, std::vector<int> ks);
}