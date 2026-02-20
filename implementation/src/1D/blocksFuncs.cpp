#include "blocksFuncs.h"
#include "../shared/defs.h"

std::vector<std::vector<int>> generateIndicesWithColor(std::vector<Color_>& colors, int numColors) {
	std::vector<std::vector<int>> indicesWithColor(numColors);
	for (int i = 0; i < colors.size(); i++) {
		indicesWithColor[colors[i]].push_back(i);
	}
	return indicesWithColor;
}

std::vector<std::vector<ColorCount>> generateBlockModes(std::vector<Color_>& colors, int s, int t){
	std::vector<std::vector<ColorCount>> blockModes(s, std::vector<ColorCount>(s));
	for (int i = 0; i < s; i++) {
		std::map<Color_, int> colorCounts; //contains the colorCounts of all points between i*t and current
		int current = i * t;
		for (int j = i + 1; j < s; j++) {
			while (current < std::min(j * t, (int)colors.size())) { //move current along until the start of block j
				colorCounts[colors[current]]++;
				current++;
			}
			blockModes[i][j] = maxCount(colorCounts);
		}
	}
	return blockModes;
}