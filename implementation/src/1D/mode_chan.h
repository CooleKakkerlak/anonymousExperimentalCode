#pragma once

#include "../shared/defs.h"
#include "mode_rangeTree.h"


struct AAS1DModeChanDS : AAS1DModeDS {
	int n, numColors, s, t;
	AAS1DModeReportDS reportDS;
	std::vector<Color_> colors;
	std::vector < std::vector<ColorCount>> blockModes;
	std::vector<std::vector<int>> indicesWithColor;
	std::vector<int> rankInColor;	//for every index i, this stores the index j s.t. indicesWithColor[colors[i]][j] = i (the rank of point i within its own color)
		
	ColorCount queryMode(const Range& query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
	AAS1DModeChanDS(std::vector<Color_>& colors, int numColors, int s);
};