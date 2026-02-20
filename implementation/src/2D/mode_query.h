#pragma once
#include "../shared/defs.h"
#include "../amc/amc.h"

enum RustDSType
{
	Trivial,
	RustRangeTree,
	RustRangeTrees,
	GridNoColor,
	GridSelectColor,
	GridColumnColor,
	GridSortedColor
};


struct AAS2DModeGridDS : AAS2DModeDS
{
	AAS2DModeGridDS(std::vector<Point_2> &points, std::vector<Color_> &colors, int s);
	ColorCount queryMode(const Range2D &query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
};

struct AAS2DModeAmcDS : AAS2DModeDS
{
	std::unique_ptr<UuAasModeColorSolver> solver;
	RustDSType dsType;
	AAS2DModeAmcDS(std::vector<Point_2> &points, std::vector<Color_> &colors, RustDSType dsType, int s);
	ColorCount queryMode(const Range2D &query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
};