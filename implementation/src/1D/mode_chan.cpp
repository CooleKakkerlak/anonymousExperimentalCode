// Based on DOI 10.1007/s00224-013-9455-2

#include "mode_chan.h"

#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include "../shared/defs.h"
#include "blocksFuncs.h"

// TODO: there is some duplication between grid and chan, but not sure how to remove it in a clean way (e.g. computing t, computing span/prefix/postfix, computing memory usage)
AAS1DModeChanDS::AAS1DModeChanDS(std::vector<Color_> &colors, int numColors, int s)
	: colors(colors), n(colors.size()), numColors(numColors), s(s), t(std::ceil((double)n / s)), reportDS(colors, numColors)
{
	blockModes = generateBlockModes(colors, s, t);
	indicesWithColor = generateIndicesWithColor(colors, numColors);
	rankInColor = std::vector<int>(n);
	std::vector<int> ranks = std::vector<int>(numColors);
	for (int i = 0; i < n; i++)
	{
		rankInColor[i] = ranks[colors[i]]++;
	}
}

ColorCount AAS1DModeChanDS::queryMode(const Range &query)
{
	int leftIndex = std::clamp(query.left, 0, n);
	int rightIndex = std::clamp(query.right, 0, n);
	// compute the block containing first and last contained elements
	int bi = leftIndex / t;
	int bj = (rightIndex - 1) / t;

	// if start and end are in the same or adjacent blocks, the span is empty; naive query is used instead
	if (bi + 1 >= bj)
		return reportDS.queryMode(Range{leftIndex, rightIndex}); 

	// prefix: A[leftIndex : (bi+1)*t]
	// Span: A[(bi+1)*t : bj*t], blocks (bi+1 : bj)
	// suffix: A[bj*t : rightIndex]

	// Best color from span
	ColorCount candidate_mode = blockModes[bi + 1][bj];

	// Now check values from prefix/postfix by lemma 2
	// prefix
	for (int i = leftIndex; i < (bi + 1) * t; i++)
	{
		int previous = rankInColor[i] - 1; // the previous point with the same color
		if (previous > 0 && indicesWithColor[colors[i]][previous] >= leftIndex)
		{
			// entry was already counted
			continue;
		}
		// < instead of <= for end due to exclusive
		// check whether more than candidate_mode.count points of color colors[i] fall within the query range
		int indexToCheck = rankInColor[i] + candidate_mode.count;
		if (!(indexToCheck < indicesWithColor[colors[i]].size() && indicesWithColor[colors[i]][indexToCheck] < rightIndex))
		{
			// colors[i] can't have higher frequency than current candidate
			continue;
		}
		// linear scan in indicesWithColor[colors[i]] in order to find the new frequency
		int iterations = 0;
		while (indexToCheck < indicesWithColor[colors[i]].size() && indicesWithColor[colors[i]][indexToCheck] < rightIndex)
		{
			indexToCheck++;
		}
		candidate_mode = ColorCount(colors[i], indexToCheck - rankInColor[i]);
	}

	// for the suffix, do the same but in the other direction
	for (int i = rightIndex - 1; i >= bj * t; i--)
	{
		int next = rankInColor[i] + 1;
		if (next < indicesWithColor[colors[i]].size() && indicesWithColor[colors[i]][next] < rightIndex)
		{
			// entry was already counted
			continue;
		}
		int indexToCheck = rankInColor[i] - candidate_mode.count;
		if (!(indexToCheck >= 0 && indicesWithColor[colors[i]][indexToCheck] >= leftIndex))
		{
			// entry can't have higher frequency than current candidate
			continue;
		}

		// linear scan in indicesWithColor[colors[i]] in order to find the new frequency
		while (indexToCheck >= 0 && indicesWithColor[colors[i]][indexToCheck] >= leftIndex)
		{
			indexToCheck--;
		}
		candidate_mode = ColorCount(colors[i], rankInColor[i] - indexToCheck);
	}

	// prefix and suffix done, therefore candidate must now be final
	return candidate_mode;
}

long AAS1DModeChanDS::getDSMemoryUsage()
{
	long count = 0;
	count += sizeof(n) + sizeof(numColors) + sizeof(s) + sizeof(t);
	count += sizeof(Color_) * colors.size();
	for (std::vector<ColorCount> &vec : blockModes)
		count += sizeof(ColorCount) * vec.size();
	for (std::vector<int> &vec : indicesWithColor)
		count += sizeof(int) * vec.size();
	count += sizeof(int) * rankInColor.size();
	return count;
}

std::string AAS1DModeChanDS::getName()
{
	return GET_NAME(AAS1DModeChanDS);
}