#pragma once
#include <vector>
#include "../shared/defs.h"
#include "../myRangeTree/myRangeTree.h"

struct AAS2DModeReportDS : AAS2DModeDS {
private:
	MyRangeTree tree;
public:
	AAS2DModeReportDS(std::vector<Point_2>& points, std::vector<Color_>& colors);

	ColorCount queryMode(const Range2D& query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
};

struct AAS2DModePerColorDS : AAS2DModeDS {
private:
	std::vector<MyRangeTree> trees;
public:
	AAS2DModePerColorDS(std::vector<Point_2>& points, std::vector<Color_>& colors, int nrColors);

	ColorCount queryMode(const Range2D& query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
};