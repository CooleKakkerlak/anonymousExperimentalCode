#ifndef RANGE_TREES_78239873122
#define RANGE_TREES_78239873122
#include "../amc.h"
#include "../../myRangeTree/myRangeTree.h"

typedef MyRangeTree RangeTreeType;

std::unordered_map<Color_, RangeTreeType> make_range_trees(std::vector<ColoredPoint_2> const &points);

ColorCount find_most_occurring_color(
    std::unordered_map<Color_, RangeTreeType> const &range_trees,
    AxisAlignedSquare const &aas,
    std::vector<Color_> candidates);

#endif