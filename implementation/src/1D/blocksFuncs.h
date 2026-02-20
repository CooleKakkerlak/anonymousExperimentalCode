#pragma once
#include "../shared/defs.h"

//indicesWithColor[c] = list of indices i s.t. colors[i] = c
std::vector<std::vector<int>> generateIndicesWithColor(std::vector<Color_>& colors, int numColors);

//blockModes[i][j] stores mode of blocks i-j (i inclusive, j exclusive)
std::vector<std::vector<ColorCount>> generateBlockModes(std::vector<Color_>& colors, int s, int t); 