#ifndef CELL_49824599723393559
#define CELL_49824599723393559

#include <vector>
#include <optional>

#include "../../shared/defs.h"
#include "grid.hpp"
#include "range_trees.hpp"

struct CellRange
{
    size_t first_top_index;
    std::vector<std::optional<Color_>> cells;
    inline size_t heap_size() const &
    {
        return memory_size::heap_size_of(cells);
    }
};

struct CellLocationStructure
{
    std::vector<std::vector<std::vector<CellRange>>> cells;

    inline size_t heap_size() const &
    {
        return memory_size::heap_size_of(cells);
    }

public:
    CellLocationStructure(ClsInput const &input);
    static CellRange make_column(
        ClsInput const &input,
        double min_y,
        double max_y,
        double bottom,
        size_t y_count,
        ranking::Rank min_x_rank,
        ranking::Rank max_x_rank);

    static void add_colors_between_ranks(
        std::unordered_map<Color_, size_t> &color_counts,
        ranking::Rank y_rank_before,
        ranking::Rank y_rank_till,
        ranking::Rank min_x_rank,
        ranking::Rank max_x_rank,
        AaData const &data);

    static void add_colors_of_rank(
        std::unordered_map<Color_, size_t> &color_counts,
        ranking::Rank y_rank,
        ranking::Rank min_x_rank,
        ranking::Rank max_x_rank,
        AaData const &data);
    static std::optional<Color_> get_mode_color(std::unordered_map<Color_, size_t> const &color_counts);

    /// Find cell mode color between extended cutters
    inline std::optional<Color_> find_cell_color(
        ExtIdx left,
        ExtIdx right,
        ExtIdx bottom,
        ExtIdx top) const &
    {
        // assert !(left >= 1);
        // assert !(bottom >= 1);
        // assert !(right >= left + 1);
        // assert !(top >= bottom + 1);
        return find_cell_color_between(left.v - 1, right.v - 1, bottom.v - 1, top.v - 1);
    }
    /// Find cell mode color between cutters
    std::optional<Color_> find_cell_color_between(
        size_t left,
        size_t right,
        size_t bottom,
        size_t top) const &;
};

struct AasCuttingIntl
{
    CuttingData cutting_data;
    CellLocationStructure cls;

    inline size_t heap_size() const
    {
        return memory_size::heap_size_of(cutting_data) + memory_size::heap_size_of(cls);
    }

    static AasCuttingIntl new_deterministic(AaData const &data, size_t slabs, std::unique_ptr<SlabColorInfo> slab_color_info);

    static CellLocationStructure make_cells(AaData const &data, CuttingData const &cutting_data);
};

/// A type used to implement [AasModeColorSolver] using the Utrecht University Grid method.
///
/// All grid solvers divide the points using a grid. They have a range tree for each point color.
/// A query (`find_colors`) is done as follows
/// - find the left, right, bottom and top slab that contain the sides of the query square
///   If a side lies on a cutter, we take the outer slab
/// - determine the the colors occurring in those slabs (with the help of `SlabColorInfo`)
/// - if the slabs enclose a cell, get the mode color of this cell and add it to the colors
/// - for every color, do a counting range tree query. Report the color with the highest count.
struct UuAMCSImpl : UuAasModeColorSolver
{
    std::string name_;
    AaData data;
    std::unordered_map<Color_, RangeTreeType> range_trees; //: HashMap<Color, RangeTree2<usize, f64, f64>>,
    AasCuttingIntl cutting;
    mutable std::unique_ptr<UuQueryStatistics> last_statistics;

public:
    inline std::string const &name() const
    {
        return name_;
    }
    inline void set_name(std::string name)
    {
        name_ = name;
    }
    //}

    // impl<SCI: SlabColorInfo> MemoryUsage for UuAMCSImpl<SCI>
    //{
    inline size_t heap_size() const
    {
        using memory_size::heap_size_of;
        return heap_size_of(data) + heap_size_of(range_trees) + heap_size_of(cutting);
    }

    /// Create a new solver with a deterministic cutting.
    /// # Arguments
    /// * points - the input points
    /// * slabs - the number of slabs (horizontal and vertical). Should be bigger than 1.
    UuAMCSImpl(std::vector<ColoredPoint_2> points, size_t slabs, std::unique_ptr<SlabColorInfo> sci, std::string name);

    void set_statistics_gathering(bool v) const
    {
        if (v)
        {
            last_statistics.reset(new UuQueryStatistics());
        }
        else
        {
            last_statistics.reset(0);
        }
    }
    const UuQueryStatistics *get_last_statistics() const
    {
        return last_statistics.get();
    }

    std::vector<ColorCount> find_colors(AxisAlignedSquare const &aas) const;

    std::vector<Color_> all_relevant_colors(AxisAlignedSquare const &aas) const;

    std::pair<size_t, bool> get_x_cutter(double x) const
    {
        return cutting.cutting_data.x.search_cutter(x);
    }
    std::pair<size_t, bool> get_y_cutter(double y) const
    {
        return cutting.cutting_data.y.search_cutter(y);
    }
};

#endif
