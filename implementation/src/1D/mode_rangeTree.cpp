#include "mode_rangeTree.h"

#include <set>
#include "../shared/defs.h"
#include "blocksFuncs.h"

RangeTree1D::RangeTree1D(std::vector<int>& points) : points(points) {}

int RangeTree1D::queryCount(const Range& query) {
	int left = std::distance(points.begin(), std::lower_bound(points.begin(), points.end(), query.left));	//inclusive
	int right = std::distance(points.begin(), std::lower_bound(points.begin(), points.end(), query.right));	//exclusive
	return right - left;
}

long RangeTree1D::getMemoryUsage() {
	return sizeof(int) * points.size();
}

AAS1DModeRangeTreesDS::AAS1DModeRangeTreesDS(std::vector<Color_>& colors, int numColors)
	: n(colors.size()), numColors(numColors) {
	std::vector<std::vector<int>> indicesWithColor = generateIndicesWithColor(colors, numColors);
	for (int col = 0; col < numColors; col++)
		rangeTrees.push_back(RangeTree1D(indicesWithColor[col]));
}


ColorCount AAS1DModeRangeTreesDS::queryMode(const Range& query) {
	ColorCount mode;
	for (int col = 0; col < numColors; col++) {
		int count = rangeTrees[col].queryCount(query);
		if (count > mode.count) {
			mode = ColorCount(col, count);
		}
	}
	return mode;
}

long AAS1DModeRangeTreesDS::getDSMemoryUsage() {
	long count = 0;
	for (RangeTree1D& rangeTree : rangeTrees)
		count += rangeTree.getMemoryUsage();
	count += sizeof(numColors) + sizeof(n);
	return count;
}

std::string AAS1DModeRangeTreesDS::getName() {
	return GET_NAME(AAS1DModeRangeTreesDS);
}


AAS1DModeReportDS::AAS1DModeReportDS(std::vector<Color_>& colors, int numColors)
: colors(colors), counts(numColors, 0) {}


ColorCount AAS1DModeReportDS::queryMode(const Range& query) {
	for (int i = query.left; i < query.right; i++)
		counts[colors[i]]++;
	ColorCount max;
	for (int i = query.left; i < query.right; i++) {
		if (counts[colors[i]] > max.count)
			max = ColorCount(colors[i], counts[colors[i]]);
		counts[colors[i]] = 0;
	}
	return max;
}

long AAS1DModeReportDS::getDSMemoryUsage() {
	long count = 0;
	count += sizeof(Color_) * colors.size();
	return count;
}

std::string AAS1DModeReportDS::getName() {
	return GET_NAME(AAS1DModeReportDS);
}


AAS1DModeGridDS::AAS1DModeGridDS(std::vector<Color_>& colors, int numColors, int s)
	: colors(colors), n(colors.size()), numColors(numColors), s(s), t(std::ceil((double)n / s)), reportDS(colors, numColors) {
	blockModes = generateBlockModes(colors, s, t);
	std::vector<std::vector<int>> indicesWithColor = generateIndicesWithColor(colors, numColors);
	for (int col = 0; col < numColors; col++)
		rangeTrees.push_back(RangeTree1D(indicesWithColor[col]));
}

ColorCount AAS1DModeGridDS::queryMode(const Range& query) {
	int leftIndex = std::clamp(query.left, 0, n);
	int rightIndex = std::clamp(query.right, 0, n);

	// compute the block containing first and last contained elements
	int bi = leftIndex / t;
	int bj = (rightIndex - 1) / t;

	// if start and end are in the same or adjacent blocks, the span is empty
	// naive query is used instead
	if (bi + 1 >= bj) return reportDS.queryMode(Range{ leftIndex, rightIndex });

	std::set<Color_> candidateColors;
	candidateColors.insert(blockModes[bi + 1][bj].color); //span
	for (int i = leftIndex; i < (bi + 1) * t; i++)
		candidateColors.insert(colors[i]); //prefix
	for (int i = rightIndex - 1; i >= bj * t; i--)
		candidateColors.insert(colors[i]); //suffix

	ColorCount mode;
	for (int col : candidateColors) {
		int count = rangeTrees[col].queryCount(query);
		if (count > mode.count) {
			mode = ColorCount(col, count);
		}
	}

	return mode;
}

long AAS1DModeGridDS::getDSMemoryUsage() {
	long count = 0;
	count += sizeof(n) + sizeof(numColors) + sizeof(s) + sizeof(t);
	count += sizeof(Color_) * colors.size();
	for (std::vector<ColorCount>& vec : blockModes)
		count += sizeof(ColorCount) * vec.size();
	for (RangeTree1D& rangeTree : rangeTrees)
		count += rangeTree.getMemoryUsage();
	return count;
}

std::string AAS1DModeGridDS::getName() {
	return GET_NAME(AAS1DModeGridDS);
}