#pragma once
#include "../shared/defs.h"

namespace DataReader {
	std::vector<std::string> split(std::string str, char delimiter);

	Dataset2D readOpenML(Scenario& scenario, MyRandom& random);
	Dataset2D readPointsFile(Scenario& scenario, MyRandom& random);
};