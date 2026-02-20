/* use std::collections::HashMap;

use memory_usage::MemoryUsage;
use ranking::Rank;

use crate::{
    AaData, Color,
    uu_mode_color::{ClsInput, ExtIdx},
};

*/

#include <vector>
#include <optional>

#include "../../shared/defs.h"
#include "grid.hpp"
#include "cell.hpp"

using std::vector;

CellLocationStructure::CellLocationStructure(ClsInput const &input)
{
    auto x_count = input.x_cutters->cutters.size();
    auto y_count = input.y_cutters->cutters.size();
    if (x_count <= 1)
        return;
    cells.reserve(x_count - 1);
    for (size_t left = 0; left < x_count - 1; ++left)
    {
        auto cells1 = vector<vector<CellRange>>();
        if (y_count > 1)
        {
            cells1.reserve(y_count - 1);
            auto min_x_rank = input.x_cutters->cutters[left].from;
            for (size_t bottom = 0; bottom < y_count - 1; ++bottom)
            {
                auto cells2 = vector<CellRange>();
                cells2.reserve(x_count - left - 1);
                for (size_t right = left + 1; right < x_count; ++right)
                {
                    auto min_y = input.x_cutter_half_dist(left, right);
                    double max_y = left == 0 || right == x_count - 1
                                       ? std::numeric_limits<double>::infinity()
                                       : input.x_cutter_half_dist(left - 1, right + 1);

                    auto max_x_rank = input.x_cutters->cutters[right].from;
                    // let emit_debug = left == 0 && bottom == 0 && right == 14;
                    cells2.push_back(make_column(
                        input, min_y, max_y, bottom, y_count, min_x_rank, max_x_rank));
                }
                cells1.push_back(cells2);
            }
        }
        cells.push_back(cells1);
    }
}

CellRange CellLocationStructure::make_column(
    ClsInput const &input,
    double min_y,
    double max_y,
    double bottom,
    size_t y_count,
    ranking::Rank min_x_rank,
    ranking::Rank max_x_rank)

{
    //  let y_half_dists = input.y_cutter_half_dist;
    size_t top = bottom + 1;
    if (bottom > 0)
    {
        while (true)
        {
            if (top == y_count - 1)
            {
                break;
            }
            if (input.y_cutter_half_dist(bottom - 1, top + 1) >= min_y)
            {
                break;
            }
            top += 1;
        }
    }
    size_t first_top_index = top;
    auto cells = vector<std::optional<Color_>>();
    cells.reserve(y_count - top); // overestimate
    if (input.y_cutter_half_dist(bottom, top) <= max_y)
    {
        auto color_counts = std::unordered_map<Color_, size_t>();

        // Add initial points
        auto cur_line = input.y_cutters->cutters[bottom].from;
        add_colors_of_rank(
            color_counts,
            cur_line,
            min_x_rank,
            max_x_rank,
            *input.data);
        auto top_line = input.y_cutters->cutters[top].from;
        add_colors_between_ranks(
            color_counts,
            cur_line,
            top_line,
            min_x_rank,
            max_x_rank,
            *input.data);
        cur_line = top_line;
        while (true)
        {
            // add current top with mode color
            cells.push_back(get_mode_color(color_counts));
            if (top + 1 == y_count)
            {
                break;
            }
            if (input.y_cutter_half_dist(bottom, top + 1) > max_y)
            {
                break;
            }
            // add points between top and top+1
            top += 1;

            auto top_line = input.y_cutters->cutters[top].from;
            add_colors_between_ranks(
                color_counts,
                cur_line,
                top_line,
                min_x_rank,
                max_x_rank,
                *input.data);
            cur_line = top_line;
        }
    }

    cells.shrink_to_fit();
    return CellRange{
        first_top_index,
        cells,
    };
}

void CellLocationStructure::add_colors_between_ranks(
    std::unordered_map<Color_, size_t> &color_counts,
    ranking::Rank y_rank_before,
    ranking::Rank y_rank_till,
    ranking::Rank min_x_rank,
    ranking::Rank max_x_rank,
    AaData const &data)
{
    while (true)
    {
        y_rank_before += 1;
        if (y_rank_before > y_rank_till)
        {
            break;
        }
        add_colors_of_rank(color_counts, y_rank_before, min_x_rank, max_x_rank, data);
    }
}
void CellLocationStructure::add_colors_of_rank(
    std::unordered_map<Color_, size_t> &color_counts,
    ranking::Rank y_rank,
    ranking::Rank min_x_rank,
    ranking::Rank max_x_rank,
    AaData const &data)
{
    auto iter_pair = data.y_ranking.all_of_rank(y_rank);
    for (auto idx = iter_pair.first; idx != iter_pair.second; ++idx)
    {
        auto x_rank = data.x_ranking.rank_of(*idx);
        if (min_x_rank <= x_rank && x_rank <= max_x_rank)
        {
            Color_ color = data.points_[*idx].color;
            auto item = color_counts.insert(std::make_pair(color, 0)).first;
            item->second += 1;
        }
    }
}

std::optional<Color_> CellLocationStructure::get_mode_color(std::unordered_map<Color_, size_t> const &color_counts)
{
    if (color_counts.empty())
    {
        return std::nullopt;
    }
    auto iter = color_counts.begin();
    Color_ color = iter->first;
    size_t count = iter->second;
    for (; iter != color_counts.end(); ++iter)
    {
        if (
            iter->second > count)
        {
            count = iter->second;
            color = iter->first;
        }
    }
    return color;
}

/// Find cell mode color between cutters
std::optional<Color_> CellLocationStructure::find_cell_color_between(
    size_t left,
    size_t right,
    size_t bottom,
    size_t top) const &

{
    // assert !(right >= left + 1);
    // assert !(top >= bottom + 1);
    auto right_idx = right - left - 1;
    // eprintln!("indexes: {left} {bottom} {right} {top}, {right_idx}");
    CellRange const &top_items = cells[left][bottom][right_idx];

    if (top < top_items.first_top_index)
    {
        return std::nullopt;
    }
    auto top_idx = top - top_items.first_top_index;
    if (top_idx >= top_items.cells.size())
    {
        return std::nullopt;
    }
    return top_items.cells[top_idx];
}

AasCuttingIntl AasCuttingIntl::new_deterministic(AaData const &data, size_t slabs, std::unique_ptr<SlabColorInfo> slab_color_info)

{
    // assert !(slabs >= 1);
    auto sample_size = slabs - 1;
    auto cutters = CuttingData::new_deterministic(sample_size, data, std::move(slab_color_info));
    auto pls = make_cells(data, cutters);
    return AasCuttingIntl{std::move(cutters), std::move(pls)};
}

CellLocationStructure AasCuttingIntl::make_cells(AaData const &data, CuttingData const &cutting_data)

{
    // for i,j pairs
    // - compute x_intersections (for 0<=i<j<vx.len), that is r, where (x,y,r) on P_LE[i] and P_RE[j]
    // - compute y_intersections (for 0<=i<j<vy.len), that is r, where (x,y,r) on P_BE[i] and P_TE[j]
    /*     auto const &x_cutters = cutting_data.x.cutters;
        size_t x_count = x_cutters.size();
        // let mut x_cutter_half_dist = TriMatrix::new (x_count);
        if (x_count > 0)
        {
            for (size_t i = 0; i < x_count - 1; ++i)
            {
                for (size_t j = i + 1; j < x_count; ++j)
                {
                    let value = (x_cutters[j].v - x_cutters[i].v) / 2.0;
                    x_cutter_half_dist.set(i, j, value);
                }
            }
        }

        let y_cutters : &Vec<Cutter> = &cutting_data.y.cutters;
        let y_count = y_cutters.len();
        let mut y_cutter_half_dist = TriMatrix::new (y_count);
        if y_count
            > 0
            {
            for
                i in 0..(y_count - 1)
                {
                for
                    j in(i + 1)..y_count
                    {
                        let value = (y_cutters[j].v - y_cutters[i].v) / 2.0;
                        y_cutter_half_dist.set(i, j, value);
                    }
                }
            } */

    auto pls_input = ClsInput{
        &data,
        &cutting_data.x,
        &cutting_data.y,
    };
    return CellLocationStructure(pls_input);
}
