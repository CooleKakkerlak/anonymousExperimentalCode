#pragma once

#include "../shared/defs.h"

struct RangeTree1D {
	std::vector<int> points;

	RangeTree1D(std::vector<int>& points);

	int queryCount(const Range& query);
	long getMemoryUsage();
};

struct AAS1DModeReportDS : AAS1DModeDS {
	std::vector<Color_> colors;
	std::vector<int> counts;

	AAS1DModeReportDS(std::vector<Color_>& colors, int numColors);
	ColorCount queryMode(const Range& query);
	long getDSMemoryUsage() override;
	std::string getName() override;
};

struct AAS1DModeRangeTreesDS : AAS1DModeDS {
	std::vector<RangeTree1D> rangeTrees;
	int n, numColors;

	AAS1DModeRangeTreesDS(std::vector<Color_>& colors, int numColors);
	ColorCount queryMode(const Range& query);
	long getDSMemoryUsage() override;
	std::string getName() override;
};

struct AAS1DModeGridDS : AAS1DModeDS {
	int n, numColors, s, t;
	AAS1DModeReportDS reportDS;
	std::vector<Color_> colors;
	std::vector < std::vector<ColorCount>> blockModes;
	std::vector<RangeTree1D> rangeTrees;

	AAS1DModeGridDS(std::vector<Color_>& colors, int nrColors, int s);
	ColorCount queryMode(const Range& query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
};