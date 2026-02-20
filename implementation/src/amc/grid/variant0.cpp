#include <unordered_set>

#include "../../shared/defs.h"
#include "../amc.h"
#include "cell.hpp"

struct SlabColorInfo0 : SlabColorInfo
{
    virtual ~SlabColorInfo0();
    void init(AaData const &data, OneDCutters const &x_cutters, OneDCutters const &y_cutters);
    /// Add the colors of (a subset of) the points in the left, right, bottom and top slab
    /// to the `color_set`.
    void add_border_colors(
        std::unordered_set<Color_> &color_set,
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

    SlabColorInfo0();

    inline void add_x_slab_between(
        std::unordered_set<Color_> &color_set,
        AaData const &data,
        ranking::Rank from,
        ranking::Rank to,
        ranking::Rank y_min_rank,
        ranking::Rank y_max_rank) const
    {
        add_range_between(
            color_set,
            data.points(),
            data.x_ranking,
            data.y_ranking,
            from,
            to,
            y_min_rank,
            y_max_rank);
    }
    void add_y_slab_between(
        std::unordered_set<Color_> &color_set,
        AaData const &data,
        ranking::Rank from,
        ranking::Rank to,
        ranking::Rank x_min_rank,
        ranking::Rank x_max_rank) const
    {
        add_range_between(
            color_set,
            data.points(),
            data.y_ranking,
            data.x_ranking,
            from,
            to,
            x_min_rank,
            x_max_rank);
    }

    void add_range_between(
        std::unordered_set<Color_> &color_set,
        std::vector<ColoredPoint_2> const &points,
        ranking::Ranking const &ranking,
        ranking::Ranking const &orto_ranking,
        ranking::Rank from,
        ranking::Rank to,
        ranking::Rank orto_min_rank,
        ranking::Rank orto_max_rank) const;
};

struct GridSelectColorsSolver : UuAMCSImpl
{
    /// Construct a [GridSelectColorsSolver] from a vector of [ColoredPoint].
    GridSelectColorsSolver(std::vector<ColoredPoint_2> points, size_t slabs);
};

using ranking::Rank;
using ranking::Ranking;

SlabColorInfo0::~SlabColorInfo0()
{
}

SlabColorInfo0::SlabColorInfo0()
{
}

size_t SlabColorInfo0::heap_size() const
{
    return 0;
}

void SlabColorInfo0::init(AaData const &data, OneDCutters const &x_cutters, OneDCutters const &y_cutters)
{
}

void SlabColorInfo0::add_border_colors(
    std::unordered_set<Color_> &color_set,
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
    CuttingData const &cutting_data) const
{
    auto y_min_rank = data.search_rank_at_least_y(aas.min_y());
    if (!y_min_rank.has_value())
    {
        return;
    }
    // let y_min_rank = y_min_rank.unwrap();
    auto y_max_rank = data.search_rank_at_most_y(aas.max_y());
    if (!y_max_rank.has_value())
    {
        return;
    }
    // let y_max_rank = y_max_rank.unwrap();
    auto x_min_rank = data.search_rank_at_least_x(aas.min_x());
    if (!x_min_rank.has_value())
    {
        return;
    }
    // let x_min_rank = x_min_rank.unwrap();
    auto x_max_rank = data.search_rank_at_most_x(aas.max_x());
    if (!x_max_rank.has_value())
    {
        return;
    }
    // let x_max_rank = x_max_rank.unwrap();
    if (x_max_slb < x_min_slb + 2)
    {
        // we only need to look at these two neighboring slabs
        add_x_slab_between(
            color_set, data, *x_min_rank, *x_max_rank, *y_min_rank, *y_max_rank);
        return;
    }
    if (y_max_slb < y_min_slb + 2)
    {
        // we only need to look at these two neighboring slabs
        add_y_slab_between(
            color_set, data, *y_min_rank, *y_max_rank, *x_min_rank, *x_max_rank);
        return;
    }
    if (need_x_min)
    {
        add_x_slab_between(
            color_set,
            data,
            *x_min_rank,
            cutting_data.right_rank_of_slab(x_min_slb),
            *y_min_rank,
            *y_max_rank);
    }
    if (need_x_max)
    {
        add_x_slab_between(
            color_set,
            data,
            cutting_data.left_rank_of_slab(x_max_slb),
            *x_max_rank,
            *y_min_rank,
            *y_max_rank);
    }
    if (need_y_min)
    {
        add_y_slab_between(
            color_set,
            data,
            *y_min_rank,
            cutting_data.top_rank_of_slab(y_min_slb),
            *x_min_rank,
            *x_max_rank);
    }
    if (need_y_max)
    {
        add_y_slab_between(
            color_set,
            data,
            cutting_data.bottom_rank_of_slab(y_max_slb),
            *y_max_rank,
            *x_min_rank,
            *x_max_rank);
    }
}

void SlabColorInfo0::add_range_between(
    std::unordered_set<Color_> &color_set,
    std::vector<ColoredPoint_2> const &points,
    Ranking const &ranking,
    Ranking const &orto_ranking,
    Rank from,
    Rank to,
    Rank orto_min_rank,
    Rank orto_max_rank) const
{
    while (true)
    {
        if (from > to)
        {
            break;
        }
        auto p = ranking.all_of_rank(from);
        for (std::vector<size_t>::const_iterator pt_idx = p.first; pt_idx != p.second; ++pt_idx)
        {
            auto orto_rank = orto_ranking.rank_of(*pt_idx);
            if (orto_min_rank <= orto_rank && orto_rank <= orto_max_rank)
            {
                color_set.insert(points[*pt_idx].color);
            }
        }
        from += 1;
    }
}

GridSelectColorsSolver::GridSelectColorsSolver(std::vector<ColoredPoint_2> points, size_t slabs)
    : UuAMCSImpl(std::move(points), slabs, std::unique_ptr<SlabColorInfo>(new SlabColorInfo0()), std::string("GridSelectColors"))
{
}

/// A Grid AasModeColorSolver that does not store any color information with the slabs
///
/// All Grid Aas Mode Color Solvers divide the input points in horizontal and vertical slabs.
/// Each slab contains approximately equal number of points.
/// This solver selects the points in the slabs that lie within the query square.
/// For each color that is found there, and for the mode color of the enclosed cell,
/// a range tree counting query is done.
std::unique_ptr<UuAasModeColorSolver> grid_select_colors_solver(std::vector<ColoredPoint_2> points, size_t slabs)
{
    return std::unique_ptr<UuAasModeColorSolver>(new GridSelectColorsSolver(std::move(points), slabs));
}