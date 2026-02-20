#include "mode_query.h"
#include "../shared/defs.h"
#include <iostream>

AAS2DModeGridDS::AAS2DModeGridDS(std::vector<Point_2> &points, std::vector<Color_> &colors, int s)
{
	// partition points into s rows, and into s columns

	// build dictionary of modes

	// for each color, build range-counting DS
}

ColorCount AAS2DModeGridDS::queryMode(const Range2D &query)
{
	// find relevant colors:
	//	mode color of largest enclosed rectangle
	//	colors appearing in rows/columns

	// foreach relevant color, do a counting query

	return ColorCount();
}

long AAS2DModeGridDS::getDSMemoryUsage()
{
	return 0;
}

std::string AAS2DModeGridDS::getName()
{
	return GET_NAME(AAS2DModeGridDS);
}

AAS2DModeAmcDS::AAS2DModeAmcDS(std::vector<Point_2> &points, std::vector<Color_> &colors, RustDSType dsType, int r)
	: dsType(dsType)
{
	// convert from our points to the rust-compatible type
	std::vector<ColoredPoint_2> colPoints;
	for (int i = 0; i < points.size(); i++)
	{
		colPoints.push_back(ColoredPoint_2{points[i], colors[i]});
	}
	// build the requested DS
	switch (dsType)
	{
	case RustDSType::GridNoColor:
		solver = grid_all_points_solver(colPoints, r);
		break;
	case RustDSType::GridSelectColor:
		solver = grid_select_colors_solver(colPoints, r);
		break;
	case RustDSType::GridColumnColor:
		solver = grid_all_colors_solver(colPoints, r);
		break;
	case RustDSType::GridSortedColor:
		solver = grid_sorted_colors_solver(colPoints, r);
		break;
	}
}

ColorCount AAS2DModeAmcDS::queryMode(const Range2D &query)
{
	double cx = (query.lower.x() + query.upper.x()) / 2;
	double cy = (query.lower.y() + query.upper.y()) / 2;
	double radius = (query.upper.x() - query.lower.x()) / 2;
	auto col_count_opt = solver->a_mode_color(AxisAlignedSquare(cx, cy, radius));
	return col_count_opt.has_value() ? *col_count_opt : ColorCount(999899, 0);
}

long AAS2DModeAmcDS::getDSMemoryUsage()
{
	return memory_size::total_size_of(*solver);
}

std::string AAS2DModeAmcDS::getName()
{
	switch (dsType)
	{
	case RustDSType::Trivial:
		return GET_NAME(Amc::Trivial);
		break;
	case RustDSType::RustRangeTree:
		return GET_NAME(Amc::RustRangeTree);
		break;
	case RustDSType::RustRangeTrees:
		return GET_NAME(Amc::RustRangeTrees);
		break;
	case RustDSType::GridNoColor:
		return GET_NAME(Amc::GridNoColor);
		break;
	case RustDSType::GridSelectColor:
		return GET_NAME(Amc::GridSelectColor);
		break;
	case RustDSType::GridColumnColor:
		return GET_NAME(Amc::GridColumnColor);
		break;
	case RustDSType::GridSortedColor:
		return GET_NAME(Amc::GridSortedColor);
		break;
	default:
		throw std::runtime_error("invalid argument");
	}
}
