#include "mode_naive.h"
#include "../shared/defs.h"
#include "range_query.h"

AAS2DModeReportDS::AAS2DModeReportDS(std::vector<Point_2> &points, std::vector<Color_> &colors) : tree(points, colors) {}

ColorCount AAS2DModeReportDS::queryMode(const Range2D &query)
{
	auto pointsInRange = tree.rangeReport(query);

	std::map<Color_, int> candidate_modes;
	for (ColoredPoint_2 &point : pointsInRange)
		candidate_modes[point.color]++;

	return maxCount(candidate_modes);
}

long AAS2DModeReportDS::getDSMemoryUsage()
{
	return tree.getMemUsage();
}

std::string AAS2DModeReportDS::getName()
{
	return GET_NAME(AAS2DModeReportDS);
}

AAS2DModePerColorDS::AAS2DModePerColorDS(std::vector<Point_2> &points, std::vector<Color_> &colors, int nrColors)
{
	std::vector<std::vector<Point_2>> pointsByColor(nrColors);
	for (int i = 0; i < points.size(); i++)
		pointsByColor[colors[i]].push_back(points[i]);

	for (int col = 0; col < nrColors; col++)
	{
		auto color_vec = std::vector<Color_>(pointsByColor[col].size(), col);
		trees.push_back(MyRangeTree(pointsByColor[col], color_vec));
	}
}

ColorCount AAS2DModePerColorDS::queryMode(const Range2D &query)
{
	int maxCount = 0;
	Color_ maxCol = -1;
	for (int col = 0; col < trees.size(); col++)
	{
		int count = trees[col].rangeCount(query);
		if (count > maxCount)
		{
			maxCount = count;
			maxCol = col;
		}
	}
	return ColorCount(maxCol, maxCount);
}

long AAS2DModePerColorDS::getDSMemoryUsage()
{
	long count = 0;
	for (MyRangeTree &tree : trees)
		count += tree.getMemUsage();
	return count;
}

std::string AAS2DModePerColorDS::getName()
{
	return GET_NAME(AAS2DModePerColorDS);
}