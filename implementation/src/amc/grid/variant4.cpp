#include <unordered_set>

#include "../../shared/defs.h"
#include "../amc.h"
#include "cell.hpp"

using ranking::Rank;
using ranking::Ranking;

struct SlabColorInfo4 : SlabColorInfo
{
    virtual ~SlabColorInfo4();
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

    SlabColorInfo4();

    inline void add_x_slab_between(
        std::unordered_set<Color_> &color_set,
        AaData const &data,
        ranking::Rank from,
        ranking::Rank to) const
    {
        add_range_between(
            color_set,
            data.points(),
            data.x_ranking,
            from,
            to);
    }
    void add_y_slab_between(
        std::unordered_set<Color_> &color_set,
        AaData const &data,
        ranking::Rank from,
        ranking::Rank to) const
    {
        add_range_between(
            color_set,
            data.points(),
            data.y_ranking,
            from,
            to);
    }

    void add_range_between(
        std::unordered_set<Color_> &color_set,
        std::vector<ColoredPoint_2> const &points,
        ranking::Ranking const &ranking,
        ranking::Rank from,
        ranking::Rank to) const;
};

struct GridAllPointsSolver : UuAMCSImpl
{
    /// Construct a [GridSelectColorsSolver] from a vector of [ColoredPoint].
    GridAllPointsSolver(std::vector<ColoredPoint_2> points, size_t slabs);
};

SlabColorInfo4::~SlabColorInfo4()
{
}

SlabColorInfo4::SlabColorInfo4()
{
}

size_t SlabColorInfo4::heap_size() const
{
    return 0;
}

void SlabColorInfo4::init(AaData const &data, OneDCutters const &x_cutters, OneDCutters const &y_cutters)
{
}

void SlabColorInfo4::add_border_colors(
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
    Rank y_min_rank = cutting_data.bottom_rank_of_slab(y_min_slb);
    Rank y_max_rank = cutting_data.top_rank_of_slab(y_max_slb);
    Rank x_min_rank = cutting_data.left_rank_of_slab(x_min_slb);
    Rank x_max_rank = cutting_data.right_rank_of_slab(x_max_slb);
    if (x_max_slb < x_min_slb + 2)
    {
        // we only need to look at these two neighboring slabs
        add_x_slab_between(
            color_set, data, x_min_rank, x_max_rank);
        return;
    }
    if (y_max_slb < y_min_slb + 2)
    {
        // we only need to look at these two neighboring slabs
        add_y_slab_between(
            color_set, data, y_min_rank, y_max_rank);
        return;
    }
    if (need_x_min)
    {
        add_x_slab_between(
            color_set,
            data,
            x_min_rank,
            cutting_data.right_rank_of_slab(x_min_slb));
    }
    if (need_x_max)
    {
        add_x_slab_between(
            color_set,
            data,
            cutting_data.left_rank_of_slab(x_max_slb),
            x_max_rank);
    }
    if (need_y_min)
    {
        add_y_slab_between(
            color_set,
            data,
            y_min_rank,
            cutting_data.top_rank_of_slab(y_min_slb));
    }
    if (need_y_max)
    {
        add_y_slab_between(
            color_set,
            data,
            cutting_data.bottom_rank_of_slab(y_max_slb),
            y_max_rank);
    }
}

void SlabColorInfo4::add_range_between(
    std::unordered_set<Color_> &color_set,
    std::vector<ColoredPoint_2> const &points,
    Ranking const &ranking,
    Rank from,
    Rank to) const
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
            color_set.insert(points[*pt_idx].color);
        }
        from += 1;
    }
}

GridAllPointsSolver::GridAllPointsSolver(std::vector<ColoredPoint_2> points, size_t slabs)
    : UuAMCSImpl(std::move(points), slabs, std::unique_ptr<SlabColorInfo>(new SlabColorInfo4()), std::string("GridSelectColors"))
{
}

std::unique_ptr<UuAasModeColorSolver> grid_all_points_solver(std::vector<ColoredPoint_2> points, size_t slabs)
{
    return std::unique_ptr<UuAasModeColorSolver>(new GridAllPointsSolver(std::move(points), slabs));
}