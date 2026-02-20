#include <unordered_set>

#include "../../shared/defs.h"
#include "../amc.h"
#include "cell.hpp"

using ranking::Rank;
using ranking::Ranking;
using std::move;
using std::unique_ptr;
using std::unordered_set;
using std::vector;

struct SlabColorInfo1 : SlabColorInfo
{
    vector<vector<Color_>> colors_in_x_slab;
    vector<vector<Color_>> colors_in_y_slab;
    virtual ~SlabColorInfo1();
    void init(AaData const &data, OneDCutters const &x_cutters, OneDCutters const &y_cutters);
    void add_border_colors(
        unordered_set<Color_> &color_set,
        size_t left_slab,
        size_t right_slab,
        size_t bottom_slab,
        size_t top_slab,
        bool need_left,
        bool need_right,
        bool need_bottom,
        bool need_top,
        AxisAlignedSquare const &aas,
        AaData const &data,
        CuttingData const &cutters) const;
    size_t heap_size() const;

    SlabColorInfo1();
    void add_colors_of_x_slab(unordered_set<Color_> &color_set, size_t x_slb) const;
    void add_colors_of_y_slab(unordered_set<Color_> &color_set, size_t y_slb) const;
};

struct GridAllColorsSolver : UuAMCSImpl
{
    GridAllColorsSolver(vector<ColoredPoint_2> points, size_t slabs);
};

SlabColorInfo1::~SlabColorInfo1()
{
}

SlabColorInfo1::SlabColorInfo1()
{
}

size_t SlabColorInfo1::heap_size() const
{
    return 0;
}

void push_colors(vector<vector<Color_>> &result, unordered_set<Color_> const &cur_set)
{
    vector<Color_> colors;
    colors.reserve(cur_set.size());
    std::for_each(cur_set.begin(), cur_set.end(), [&colors](unordered_set<Color_>::value_type const &c)
                  { colors.push_back(c); });
    result.push_back(move(colors));
}

vector<vector<Color_>> fill_slabs(
    vector<ColoredPoint_2> const &points,
    Ranking const &ranking,
    OneDCutters const &one_d_cutters)
{
    vector<Cutter> const &cutters = one_d_cutters.cutters;
    vector<vector<Color_>> result;
    result.reserve(cutters.size() + 1);
    unordered_set<Color_> cur_set;
    unordered_set<Color_> next_set;
    size_t next_cutter = 0;
    Rank next_rank_bound = next_cutter == cutters.size() ? Rank(ranking.rank_len()) : cutters[next_cutter].from;
    for (Rank rank = Rank(0); rank != Rank(ranking.rank_len()); rank = rank + 1)
    {
        if (rank > next_rank_bound)
        {
            // write cur_set, move next_set to cur_set, set next_rank_bound
            push_colors(result, cur_set);
            cur_set = move(next_set);
            next_set = unordered_set<Color_>();
            next_cutter += 1;

            next_rank_bound = next_cutter == cutters.size() ? Rank(ranking.rank_len()) : cutters[next_cutter].from;
        }
        bool in_both = rank == next_rank_bound;
        auto range = ranking.all_of_rank(rank);
        for (auto item = range.first; item != range.second; ++item)
        {
            auto idx = *item;
            Color_ col = points[idx].color;
            cur_set.insert(col);
            if (in_both)
            {
                next_set.insert(col);
            }
        }
    }
    push_colors(result, cur_set);
    if (result
            .size() <= cutters.max_size())
    {
        push_colors(result, next_set);
    }
    // debug_assert_eq !(result.len(), cutters.len() + 1);
    return result;
}

void SlabColorInfo1::add_colors_of_x_slab(unordered_set<Color_> &color_set, size_t x_slb) const
{
    auto color_vec = this->colors_in_x_slab[x_slb];
    for (auto color : color_vec)
    {
        color_set.insert(color);
    }
}
void SlabColorInfo1::add_colors_of_y_slab(unordered_set<Color_> &color_set, size_t y_slb) const
{
    auto color_vec = this->colors_in_y_slab[y_slb];
    for (auto color : color_vec)
    {
        color_set.insert(color);
    }
}

void SlabColorInfo1::init(AaData const &data, OneDCutters const &x_cutters, OneDCutters const &y_cutters)

{
    this->colors_in_x_slab = fill_slabs(data.points(), data.x_ranking, x_cutters);
    this->colors_in_y_slab = fill_slabs(data.points(), data.y_ranking, y_cutters);
}

void SlabColorInfo1::add_border_colors(
    unordered_set<Color_> &color_set,
    size_t x_min_slb,
    size_t x_max_slb,
    size_t y_min_slb,
    size_t y_max_slb,
    bool need_x_min,
    bool need_x_max,
    bool need_y_min,
    bool need_y_max,
    AxisAlignedSquare const &aas,
    AaData const &data,
    CuttingData const &cutters) const
{
    if (need_x_min)
    {
        this->add_colors_of_x_slab(color_set, x_min_slb);
    }
    if (need_x_max)
    {
        this->add_colors_of_x_slab(color_set, x_max_slb);
    }
    if (need_y_min)
    {
        this->add_colors_of_y_slab(color_set, y_min_slb);
    }
    if (need_y_max)
    {
        this->add_colors_of_y_slab(color_set, y_max_slb);
    }
}

GridAllColorsSolver::GridAllColorsSolver(vector<ColoredPoint_2> points, size_t slabs)
    : UuAMCSImpl(move(points), slabs, unique_ptr<SlabColorInfo>(new SlabColorInfo1()), std::string("GridAllColors"))
{
}

unique_ptr<UuAasModeColorSolver> grid_all_colors_solver(vector<ColoredPoint_2> points, size_t slabs)
{
    return unique_ptr<UuAasModeColorSolver>(new GridAllColorsSolver(move(points), slabs));
}
