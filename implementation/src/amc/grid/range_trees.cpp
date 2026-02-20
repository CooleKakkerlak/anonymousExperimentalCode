#include "range_trees.hpp"

using std::unordered_map;
using std::vector;
//, time::Instant};

// use range_tree::{Bounds, CoordAccess2, RangeTree2};

// use crate::{AxisAlignedSquare, Color, ColorCount, ColoredPoint};

// const PRINT_TIMINGS : bool = false;

/*
struct PointIndex<'a> {
    points: &'a Vec<ColoredPoint>,
    idxes: &'a Vec<usize>,
}

impl<'a> PointIndex<'a>
{
    fn new (points : &'a Vec<ColoredPoint>, idxes: &' a Vec<usize>)->PointIndex <'a> { PointIndex { points, idxes }
}
}

impl<'a> CoordAccess2<usize, f64, f64> for PointIndex<' a>
{
    fn len(&self)->usize{
        self.idxes.len()}

    fn id(&self, idx : usize)
        ->usize{
            self.idxes[idx]}

    fn coords(&self, idx : usize)
        ->(f64, f64)
    {
        let pt = &self.points[self.idxes[idx]];
        (pt.x, pt.y)
    }
}
*/

unordered_map<Color_, RangeTreeType> make_range_trees(std::vector<ColoredPoint_2> const &points)
{
    auto point_idx_per_color = unordered_map<Color_, vector<Point_2>>();
    for (auto pt : points)
    {
        Color_ col = pt.color;
        auto item = point_idx_per_color.find(col);
        if (item == point_idx_per_color.end())
        {
            vector<Point_2> points(1, pt.point);
            point_idx_per_color.insert(std::make_pair(col, points));
        }
        else
        {
            item->second.push_back(pt.point);
        }
    }
    auto result = unordered_map<Color_, RangeTreeType>();
    for (auto idxes : point_idx_per_color)
    {
        vector<Color_> colors(idxes.second.size(), 0);

        // auto ca = PointIndex::new (points, idxes->second);
        // auto tree = MyRangeTree(idxes.second, colors);
        // result.try_emplace(idxes.first, std::move(tree));
        result.try_emplace(idxes.first, idxes.second, colors);
    }
    return result;
}

ColorCount find_most_occurring_color(
    std::unordered_map<Color_, RangeTreeType> const &range_trees,
    AxisAlignedSquare const &aas,
    std::vector<Color_> candidates)
{
    if (candidates.empty())
    {
        return ColorCount(0, 0);
    }

    int max_count = 0;
    Color_ max_color = 0;
    Range2D range(aas.cx(), aas.cy(), aas.r());
    for (auto color : candidates)
    {
        auto cur_range_tree = range_trees.find(color);
        auto count = cur_range_tree->second.rangeCount(range);
        if (count > max_count)
        {
            max_count = count;
            max_color = color;
        }
    }

    return ColorCount(max_color, max_count);
}
