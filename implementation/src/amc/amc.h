#ifndef AMC_48523108794626648
#define AMC_48523108794626648

#include <cassert>
#include <chrono>
#include "../shared/defs.h"
#include "../ranking.hpp"

/// A square, aligned to the x- and y-axis
struct AxisAlignedSquare
{
    double cx_;
    double cy_;
    double r_;

    /// Create a new square, given the center and the radius.
    AxisAlignedSquare(double cx, double cy, double r) : cx_(cx), cy_(cy), r_(r)
    {
        assert(r_ >= 0.0);
    }
    /// Return the x coordinate of the center.
    inline double cx() const
    {
        return cx_;
    }
    /// Return the y coordinate of the center.
    inline double cy() const
    {
        return cy_;
    }
    /// Return the radius of the square.
    inline double r() const
    {
        return r_;
    }
    /// Return the minimal x-value.
    inline double min_x() const
    {
        return cx_ - r_;
    }
    /// Return the maximal x-value.
    inline double max_x() const
    {
        return cx_ + r_;
    }
    /// Return the minimal y-value.
    inline double min_y() const
    {
        return cy_ - r_;
    }
    /// Return the maximal y-value.
    inline double max_y() const
    {
        return cy_ + r_;
    }
    /// Returns if the square contains a point, where a point on the boundary counts as contained.
    inline bool contains_incl(double x, double y) const
    {
        return cx_ - r_ <= x && x <= cx_ + r_ && cy_ - r_ <= y && y <= cy_ + r_;
    }
};

enum class RankSearchType
{
    Found,
    NotFound,
    Beyond
};

struct RankSearchResult
{
    RankSearchType t;
    ranking::Rank r;
};

struct AaData
{
    std::vector<ColoredPoint_2> points_;
    ranking::Ranking x_ranking;
    ranking::Ranking y_ranking;

public:
    AaData(std::vector<ColoredPoint_2> points);
    size_t heap_size() const;
    std::vector<ColoredPoint_2> const &points() const;
    RankSearchResult search_rank_x(double x) const;
    std::optional<ranking::Rank> search_rank_at_least_x(double x) const;
    std::optional<ranking::Rank> search_rank_at_most_x(double x) const;
    RankSearchResult search_rank_y(double y) const;
    std::optional<ranking::Rank> search_rank_at_least_y(double y) const;
    std::optional<ranking::Rank> search_rank_at_most_y(double y) const;
};

/// Trait that solvers for the axis aligned square (aas) mode color problem must implement
///
/// A mode color of an aas is one of the colors that occurs most in the points inside
/// (or on the border of) an aas.
struct AasModeColorSolver
{
    /// Return the count and values of the colors
    /// that have a maximal occurrence in the query square
    /// May return a single color even if there are more.
    std::optional<std::pair<size_t, std::vector<Color_>>> mode_colors(AxisAlignedSquare const &aas) const;

    /// Return one of the maximal occurring colors in the query square, together with its count.
    std::optional<ColorCount> a_mode_color(AxisAlignedSquare const &aas) const;
    /// Return counts for colors occurring in the query square.
    /// The colors are sorted by [ColorCount::cmp_by_dec_count]
    std::vector<ColorCount> find_colors_dec(AxisAlignedSquare const &aas) const;
    /// Should return counts for colors occurring in the query square. No ordering is required.
    ///
    /// This is the only method that must be implemented. The other ones are implemented
    /// with the help of this one.
    ///
    /// A mode color must be returned, if there is one. The solver may return more than one [ColorCount].
    /// In that case, it should return at least all mode colors.
    virtual std::vector<ColorCount> find_colors(AxisAlignedSquare const &aas) const = 0;
    /// A name for this solver
    virtual std::string const &name() const = 0;
    virtual size_t heap_size() const = 0;
};

struct UuQueryStatistics
{
public:
    // The time for getting the colors in the four slabs
    std::chrono::steady_clock::duration slab_time;
    // The time for getting the mode color of the cell
    std::chrono::steady_clock::duration cell_time;
    // The time for performing the range queries with the relevant colors
    std::chrono::steady_clock::duration range_search_time;
    // The number of relevant colors
    size_t relevant_colors;

    UuQueryStatistics() : slab_time(0), cell_time(0), range_search_time(0), relevant_colors(0)
    {
    }
};

struct UuAasModeColorSolver : AasModeColorSolver
{
    virtual void set_statistics_gathering(bool v) const = 0;
    virtual UuQueryStatistics const *get_last_statistics() const = 0;
};

/// Bounds contain the upper and lower bound of a RangeType
template <class E>
struct Bounds
{
    E lower;
    E upper;

    Bounds(E lower, E upper) : lower(lower), upper(upper)
    {
    }
    /// Check whether a value lies inside the bounds
    /// The check is inclusive, so a value equal to a bound qualifies
    inline bool lies_inside(E v)
    {
        return !(v < lower) && !(upper < v);
    }
};

/// A Grid UuAasModeColorSolver that does not store any color information with the slabs
///
/// All Grid Aas Mode Color Solvers divide the input points in horizontal and vertical slabs.
/// Each slab contains approximately equal number of points.
/// This solver selects the points in the slabs that lie within the query square.
/// For each color that is found there, and for the mode color of the enclosed cell,
/// a range tree counting query is done.
std::unique_ptr<UuAasModeColorSolver> grid_select_colors_solver(std::vector<ColoredPoint_2> points, size_t slabs);
std::unique_ptr<UuAasModeColorSolver> grid_all_colors_solver(std::vector<ColoredPoint_2> points, size_t slabs);
std::unique_ptr<UuAasModeColorSolver> grid_sorted_colors_solver(std::vector<ColoredPoint_2> points, size_t slabs);
/// A UU AasModeColorSolver that does not store any color information with the slabs
///
/// This variant should use the same colors as the variant with two lists per slab.
/// All UU Aas Mode Color Solvers divide the input points in horizontal and vertical slabs.
/// Each slab contains approximately equal number of points.
std::unique_ptr<UuAasModeColorSolver> grid_select_sorted_colors_solver(std::vector<ColoredPoint_2> points, size_t slabs);
std::unique_ptr<UuAasModeColorSolver> grid_all_points_solver(std::vector<ColoredPoint_2> points, size_t slabs);

#endif