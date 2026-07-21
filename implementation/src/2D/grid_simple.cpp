#include "grid_simple.h"
#include <limits.h>

GridIndex::GridIndex(int xi, int xj, int yi, int yj) {
	this->xi = xi;
	this->xj = xj;
	this->yi = yi;
	this->yj = yj;
}

AAS2DModeGridDS::AAS2DModeGridDS(std::vector<Point_2>& points, std::vector<Color_>& colors, int s, int nrColors) :
	xslabPoints(s), yslabPoints(s)
{
	//convert to coloredPoint
	std::vector<ColoredPoint_2> coloredPoints = convertPoints(points, colors);

	//for each color, build a range tree
	std::vector<std::vector<ColoredPoint_2>> pointsByColor(nrColors);
	for (int i = 0; i < coloredPoints.size(); i++)
		pointsByColor[colors[i]].push_back(coloredPoints[i]);

	for (int col = 0; col < nrColors; col++) {
		trees.push_back(MyRangeTree(pointsByColor[col]));
	}

	// partition points into s rows and s columns
	//sort by x
	std::sort(coloredPoints.begin(), coloredPoints.end(), [](const ColoredPoint_2& lhs, const ColoredPoint_2& rhs) { return lhs.point.x() < rhs.point.x(); });
	for (int i = 0; i < coloredPoints.size(); i++)
		xslabPoints[i / (coloredPoints.size() / s + 1)].push_back(coloredPoints[i]);

	//sort by y
	std::sort(coloredPoints.begin(), coloredPoints.end(), [](const ColoredPoint_2& lhs, const ColoredPoint_2& rhs) { return lhs.point.y() < rhs.point.y(); });
	for (int i = 0; i < coloredPoints.size(); i++)
		yslabPoints[i / (coloredPoints.size() / s + 1)].push_back(coloredPoints[i]);

	//set the slab boundaries
	xslabBoundaries.push_back(-std::numeric_limits<double>::infinity());
	yslabBoundaries.push_back(-std::numeric_limits<double>::infinity());
	for (int i = 0; i < s - 1; i++) {
		xslabBoundaries.push_back(xslabPoints[i].back().point.x());
		yslabBoundaries.push_back(yslabPoints[i].back().point.y());
	}
	xslabBoundaries.push_back(std::numeric_limits<double>::infinity());
	yslabBoundaries.push_back(std::numeric_limits<double>::infinity());

	//// build dictionary of modes
	//// slow version, can be used for checking
	//for (int xi = 0; xi < s; xi++)
	//	for (int xj = xi + 2; xj < s; xj++)
	//		for (int yi = 0; yi < s; yi++)
	//			for (int yj = yi + 2; yj < s; yj++) {
	//				double xmin = xslabBoundaries[xj] - xslabBoundaries[xi + 1], xmax = xslabBoundaries[xj + 1] - xslabBoundaries[xi],
	//					ymin = yslabBoundaries[yj] - yslabBoundaries[yi + 1], ymax = yslabBoundaries[yj + 1] - yslabBoundaries[yi];
	//				if (xmin > ymax || xmax < ymin) continue; // skip the non-square combinations

	//				Point_2 bottomleft(xslabBoundaries[xi + 1], yslabBoundaries[yi + 1]);
	//				Point_2 topright(xslabBoundaries[xj], yslabBoundaries[yj]);

	//				std::map<Color_, int> colorCounts;
	//				for (int i = 0; i < coloredPoints.size(); i++)
	//					if (bottomleft.x() < coloredPoints[i].point.x() && coloredPoints[i].point.x() <= topright.x() &&
	//						bottomleft.y() < coloredPoints[i].point.y() && coloredPoints[i].point.y() <= topright.y())
	//						colorCounts[coloredPoints[i].color]++;
	//				gridModeColor[GridIndex(xi, xj, yi, yj)] = maxCount(colorCounts).color;
	//			}

	// fast version
	//walk through the possible gridindices. these form a 4D arrangement, and all square-cells are adjacent, so we can safely skip all non-square cells
	std::unordered_set<GridIndex> visited, added;
	std::vector<GridIndex> stack;
	std::map<Color_, int> colorCounts;
	GridIndex current(0, 2, 0, 2), previous(0, 2, 0, 1); //0202 is guaranteed to be square, since cell 00 is unbounded towards -infinity

	stack.push_back(current);

	
	while (!stack.empty()) {
		current = stack.back();
		stack.pop_back();

		//for the new/old slab: add/remove the points
		//find the slab that got added/removed
		bool isAddition = current.xi - previous.xi < 0 || current.xj - previous.xj > 0 || current.yi - previous.yi < 0 || current.yj - previous.yj > 0; //whether we are adding or removing a column/row
		std::vector<ColoredPoint_2>* relevantSlab;
		if (current.xi != previous.xi) relevantSlab = &xslabPoints[std::max(current.xi, previous.xi)];
		if (current.xj != previous.xj) relevantSlab = &xslabPoints[std::min(current.xj, previous.xj)];
		if (current.yi != previous.yi) relevantSlab = &yslabPoints[std::max(current.yi, previous.yi)];
		if (current.yj != previous.yj) relevantSlab = &yslabPoints[std::min(current.yj, previous.yj)];

		//go through points in that slab and compute if they were added or removed
		for (ColoredPoint_2& point : *relevantSlab) {
			//TODO: could be made faster/simpler, since if we add a column we only need to check y-coordinates, and if we add a row only x-coordinates
			if (isAddition) {
				Point_2 bottomleft(xslabBoundaries[current.xi + 1], yslabBoundaries[current.yi + 1]);
				Point_2 topright(xslabBoundaries[current.xj], yslabBoundaries[current.yj]);
				if (bottomleft.x() < point.point.x() && point.point.x() <= topright.x() &&
					bottomleft.y() < point.point.y() && point.point.y() <= topright.y()) {
					colorCounts[point.color]++;
				}
			}
			else {
				Point_2 bottomleft(xslabBoundaries[previous.xi + 1], yslabBoundaries[previous.yi + 1]);
				Point_2 topright(xslabBoundaries[previous.xj], yslabBoundaries[previous.yj]);
				if (bottomleft.x() < point.point.x() && point.point.x() <= topright.x() &&
					bottomleft.y() < point.point.y() && point.point.y() <= topright.y()) {
					colorCounts[point.color]--;
					if (colorCounts[point.color] == 0) colorCounts.erase(point.color); 
				}
			}

		}

		//add directly adjacent neighbors
		if (!visited.count(current)) { //only try to add neighbors the first time we visit a node
			std::vector<GridIndex> neighbors{
				GridIndex(current.xi - 1,	current.xj,		current.yi,		current.yj),
				GridIndex(current.xi + 1,	current.xj,		current.yi,		current.yj),
				GridIndex(current.xi,		current.xj - 1, current.yi,		current.yj),
				GridIndex(current.xi,		current.xj + 1, current.yi,		current.yj),
				GridIndex(current.xi,		current.xj,		current.yi - 1,	current.yj),
				GridIndex(current.xi,		current.xj,		current.yi + 1,	current.yj),
				GridIndex(current.xi,		current.xj,		current.yi,		current.yj - 1),
				GridIndex(current.xi,		current.xj,		current.yi,		current.yj + 1),
			};

			for (GridIndex& neighbor : neighbors) {
				if (!added.count(neighbor)) { //only add each node once
					if (neighbor.xi < 0 || neighbor.xi > s - 1 || neighbor.xj < 0 || neighbor.xj > s - 1 ||
						neighbor.yi < 0 || neighbor.yi > s - 1 || neighbor.yj < 0 || neighbor.yj > s - 1) continue; //don't add non-existing indices
					//compute whether this gridIndex corresponds to a square; this is the case when the x-range and y-range overlap (the intervals of possible heights and widths of rectangles with this gridindex)
					//xmin = minimum width of a rectangle in this gridIndex, xmax is maximum width, xrange = xmax - xmin.
					double xmin = xslabBoundaries[neighbor.xj] - xslabBoundaries[neighbor.xi + 1], xmax = xslabBoundaries[neighbor.xj + 1] - xslabBoundaries[neighbor.xi],
						ymin = yslabBoundaries[neighbor.yj] - yslabBoundaries[neighbor.yi + 1], ymax = yslabBoundaries[neighbor.yj + 1] - yslabBoundaries[neighbor.yi];
					if (xmin <= 0 || ymin <= 0 || xmin > ymax || xmax < ymin) continue; //don't add non-square neighbors
					stack.push_back(current);
					stack.push_back(neighbor);
					added.insert(neighbor);
				}
			}

			if (colorCounts.size() > 0)
				gridModeColor[current] = maxCount(colorCounts).color;

			////CHECK if colorCounts is correct
			//Point_2 bottomleft(xslabBoundaries[current.xi + 1], yslabBoundaries[current.yi + 1]);
			//Point_2 topright(xslabBoundaries[current.xj], yslabBoundaries[current.yj]);
			//std::map<Color_, int> colorCountsCheck;
			//for (int i = 0; i < coloredPoints.size(); i++)
			//	if (bottomleft.x() < coloredPoints[i].point.x() && coloredPoints[i].point.x() <= topright.x() &&
			//		bottomleft.y() < coloredPoints[i].point.y() && coloredPoints[i].point.y() <= topright.y())
			//		colorCountsCheck[coloredPoints[i].color]++;
			//for (auto pair : colorCountsCheck) {
			//	if (colorCounts[pair.first] != pair.second)
			//		throw std::runtime_error("incorrect count");
			//}
			//for (auto pair : colorCounts) {
			//	if (pair.second != 0 && colorCountsCheck[pair.first] != pair.second)
			//		throw std::runtime_error("incorrect count");
			//}
		}

		//if (fastGridModeColor[current] != maxCount(colorCounts).color)
		//	throw std::runtime_error("oh noes"); //TEST, should be the same every time we visit the node


		visited.insert(current);
		previous = current;
	}
}

ColorCount AAS2DModeGridDS::queryMode(const Range2D& query)
{
	int xi = std::distance(xslabBoundaries.begin(), std::lower_bound(xslabBoundaries.begin(), xslabBoundaries.end(), query.lower.x())) - 1;
	int xj = std::distance(xslabBoundaries.begin(), std::upper_bound(xslabBoundaries.begin(), xslabBoundaries.end(), query.upper.x())) - 1;
	int yi = std::distance(yslabBoundaries.begin(), std::lower_bound(yslabBoundaries.begin(), yslabBoundaries.end(), query.lower.y())) - 1;
	int yj = std::distance(yslabBoundaries.begin(), std::upper_bound(yslabBoundaries.begin(), yslabBoundaries.end(), query.upper.y())) - 1;


	//test if the GridIndex corresponds to a square; if not then something is wrong (with the non-square computation probably)
	double xmin = xslabBoundaries[xj] - xslabBoundaries[xi + 1], xmax = xslabBoundaries[xj + 1] - xslabBoundaries[xi],
		ymin = yslabBoundaries[yj] - yslabBoundaries[yi + 1], ymax = yslabBoundaries[yj + 1] - yslabBoundaries[yi];
	if (xmin > ymax || xmax < ymin) throw std::runtime_error("square is not square :(");

	std::unordered_set<Color_> relevantColors;

	//add the mode color of the largest enclosed grid-rectangle
	auto modeColor = gridModeColor.find(GridIndex(xi, xj, yi, yj));
	if (modeColor != gridModeColor.end())
		relevantColors.insert(modeColor->second);

	//add all relevant colors in the rows/columns containing the query's cornerpoints
	std::array<std::vector<ColoredPoint_2>*, 4> relevantPoints{ &xslabPoints[xi], &xslabPoints[xj], &yslabPoints[yi], &yslabPoints[yj] };
	for (int i = 0; i < relevantPoints.size(); i++)
		for (int j = 0; j < relevantPoints[i]->size(); j++) {
			ColoredPoint_2 point = (*relevantPoints[i])[j];
			if (query.lower.x() <= point.point.x() && query.lower.y() <= point.point.y() && point.point.x() <= query.upper.x() && point.point.y() <= query.upper.y())
				relevantColors.insert(point.color);
		}

	int max = 0, cur = 0;
	Color_ maxCol = -1;
	for (Color_ col : relevantColors) {
		cur = trees[col].rangeCount(query);
		if (cur > max) {
			max = cur;
			maxCol = col;
		}
	}

	return ColorCount(maxCol, max);
}

long AAS2DModeGridDS::getDSMemoryUsage()
{
	long sum = 0;
	for (MyRangeTree& tree : this->trees)
		sum += tree.getMemUsage();
	sum += gridModeColor.size() * (sizeof(GridIndex) + sizeof(Color_));
	sum += xslabBoundaries.size() * sizeof(double);
	sum += yslabBoundaries.size() * sizeof(double);
	for (std::vector<ColoredPoint_2> vec : xslabPoints)
		sum += vec.size() * sizeof(ColoredPoint_2);
	for (std::vector<ColoredPoint_2> vec : yslabPoints)
		sum += vec.size() * sizeof(ColoredPoint_2);
	return sum;
}


std::string AAS2DModeGridDS::getName()
{
	return GET_NAME(AAS2DModeGridDS);
}