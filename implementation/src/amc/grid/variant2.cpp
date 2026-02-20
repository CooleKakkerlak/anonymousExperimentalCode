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

struct SlabColorInfo2 : SlabColorInfo
{
    vector<vector<size_t>> colors_in_x_slab_inc;
    vector<vector<size_t>> colors_in_x_slab_dec;
    vector<vector<size_t>> colors_in_y_slab_inc;
    vector<vector<size_t>> colors_in_y_slab_dec;
    virtual ~SlabColorInfo2();
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

    SlabColorInfo2();

    void add_colors_of_x_slab_dec(unordered_set<Color_> &color_set, size_t x_slb, double v, vector<ColoredPoint_2> const &points) const;
    void add_colors_of_x_slab_inc(unordered_set<Color_> &color_set, size_t x_slb, double v, vector<ColoredPoint_2> const &points) const;
    void add_colors_of_y_slab_dec(unordered_set<Color_> &color_set, size_t x_slb, double v, vector<ColoredPoint_2> const &points) const;
    void add_colors_of_y_slab_inc(unordered_set<Color_> &color_set, size_t x_slb, double v, vector<ColoredPoint_2> const &points) const;
};

struct GridSortedColorsSolver : UuAMCSImpl
{
    /// Construct a [GridSelectColorsSolver] from a vector of [ColoredPoint].
    GridSortedColorsSolver(vector<ColoredPoint_2> points, size_t slabs);
};

SlabColorInfo2::~SlabColorInfo2()
{
}

SlabColorInfo2::SlabColorInfo2()
{
}

size_t SlabColorInfo2::heap_size() const
{
    return 0;
}
namespace
{

    void add_colors_of_slab(
        vector<vector<size_t>> &result_inc,
        vector<vector<size_t>> &result_dec,
        Rank low_rank,
        Rank high_rank,
        vector<ColoredPoint_2> const &points,
        Ranking const &ranking)
    {
        unordered_set<Color_> seen;
        // let mut seen = HashSet::new ();
        Rank rank = low_rank;
        vector<size_t> inc_indexes;
        while (true)
        {
            auto range = ranking.all_of_rank(rank);
            for (auto item = range.first; item != range.second; ++item)
            {
                size_t idx = *item;
                Color_ col = points[idx].color;
                if (seen.find(col) == seen.end())
                {
                    seen.insert(col);
                    inc_indexes.push_back(idx);
                }
            }
            if (rank == high_rank)
                break;
            rank += 1;
        }
        vector<size_t> dec_indexes;
        result_inc.push_back(move(inc_indexes));
        while (true)
        {
            auto range = ranking.all_of_rank(rank);
            for (auto item = range.first; item != range.second; ++item)
            {
                size_t idx = *item;
                Color_ col = points[idx].color;
                if (seen.find(col) != seen.end())
                {
                    seen.erase(col);
                    dec_indexes.push_back(idx);
                }
            }
            if (rank == low_rank)
                break;
            rank -= 1;
        }
        result_dec.push_back(move(dec_indexes));
    }

    std::pair<vector<vector<size_t>>, vector<vector<size_t>>> fill_slabs(
        vector<ColoredPoint_2> const &points,
        Ranking const &ranking,
        OneDCutters const &one_d_cutters)
    {
        vector<Cutter> const &cutters = one_d_cutters.cutters;
        size_t slab_count = cutters.size() + 1;
        vector<vector<size_t>> result_inc, result_dec;
        result_inc.reserve(slab_count);
        result_dec.reserve(slab_count);
        Rank low_rank(0);
        for (size_t slab = 0; slab < slab_count; ++slab)
        {
            Rank high_rank = one_d_cutters.high_rank_of_slab(slab);
            add_colors_of_slab(
                result_inc,
                result_dec,
                low_rank,
                high_rank,
                points,
                ranking);
            low_rank = high_rank;
        }
        return make_pair(result_inc, result_dec);
    }

}

void SlabColorInfo2::init(AaData const &data, OneDCutters const &x_cutters, OneDCutters const &y_cutters)

{
    auto x_pair(fill_slabs(data.points(), data.x_ranking, x_cutters));
    this->colors_in_x_slab_inc = move(x_pair.first);
    this->colors_in_x_slab_dec = move(x_pair.second);
    auto y_pair(fill_slabs(data.points(), data.y_ranking, y_cutters));
    this->colors_in_y_slab_inc = move(y_pair.first);
    this->colors_in_y_slab_dec = move(y_pair.second);
}

void SlabColorInfo2::add_border_colors(
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
        this->add_colors_of_x_slab_dec(color_set, x_min_slb, aas.min_x(), data.points());
    }
    if (need_x_max)
    {
        this->add_colors_of_x_slab_inc(color_set, x_max_slb, aas.max_x(), data.points());
    }
    if (need_y_min)
    {
        this->add_colors_of_y_slab_dec(color_set, y_min_slb, aas.min_y(), data.points());
    }
    if (need_y_max)
    {
        this->add_colors_of_y_slab_inc(color_set, y_max_slb, aas.max_y(), data.points());
    }
}

void SlabColorInfo2::
    add_colors_of_x_slab_inc(unordered_set<Color_> &color_set, size_t x_slb, double v, vector<ColoredPoint_2> const &points) const

{
    auto color_vec = this->colors_in_x_slab_inc[x_slb];
    for (size_t idx : color_vec)
    {
        ColoredPoint_2 const &col_pt = points[idx];
        if (col_pt.point.x() > v)
        {
            break;
        }
        color_set.insert(col_pt.color);
    }
}

void SlabColorInfo2::
    add_colors_of_x_slab_dec(unordered_set<Color_> &color_set, size_t x_slb, double v, vector<ColoredPoint_2> const &points) const

{
    auto color_vec = this->colors_in_x_slab_dec[x_slb];
    for (size_t idx : color_vec)
    {
        ColoredPoint_2 const &col_pt = points[idx];
        if (col_pt.point.x() < v)
        {
            break;
        }
        color_set.insert(col_pt.color);
    }
}

void SlabColorInfo2::
    add_colors_of_y_slab_inc(unordered_set<Color_> &color_set, size_t y_slb, double v, vector<ColoredPoint_2> const &points) const

{
    auto color_vec = this->colors_in_y_slab_inc[y_slb];
    for (size_t idx : color_vec)
    {
        ColoredPoint_2 const &col_pt = points[idx];
        if (col_pt.point.y() > v)
        {
            break;
        }
        color_set.insert(col_pt.color);
    }
}

void SlabColorInfo2::
    add_colors_of_y_slab_dec(unordered_set<Color_> &color_set, size_t y_slb, double v, vector<ColoredPoint_2> const &points) const

{
    auto color_vec = this->colors_in_y_slab_dec[y_slb];
    for (size_t idx : color_vec)
    {
        ColoredPoint_2 const &col_pt = points[idx];
        if (col_pt.point.y() < v)
        {
            break;
        }
        color_set.insert(col_pt.color);
    }
}

GridSortedColorsSolver::GridSortedColorsSolver(vector<ColoredPoint_2> points, size_t slabs)
    : UuAMCSImpl(move(points), slabs, std::unique_ptr<SlabColorInfo>(new SlabColorInfo2()), std::string("GridSortedColors"))
{
}

unique_ptr<UuAasModeColorSolver> grid_sorted_colors_solver(vector<ColoredPoint_2> points, size_t slabs)
{
    return unique_ptr<UuAasModeColorSolver>(new GridSortedColorsSolver(move(points), slabs));
}
