#ifndef GRID_782ert35640sv
#define GRID_782ert35640sv

#include <vector>
#include <unordered_map>
#include <optional>
#include "../../shared/defs.h"
#include "../../ranking.hpp"
#include "../../mem_size.hpp"
#include "../amc.h"

namespace memory_size
{
    template <>
    inline size_t heap_size_of(Color_ const &)
    {
        return 0;
    }

    template <>
    inline size_t heap_size_of(ColoredPoint_2 const &)
    {
        return 0;
    }
}

struct Cutter
{
    // The value for cutting
    double v;
    // The rank that has this value
    ranking::Rank from;

    inline size_t heap_size() const
    {
        return 0;
    }
};

struct OneDCutters
{
    std::vector<Cutter> cutters;
    ranking::Rank max_parent_rank;

    size_t heap_size() const
    {
        return memory_size::heap_size_of(cutters);
    }

    inline ranking::Rank low_rank_of_slab(size_t slab) const
    {
        return (slab == 0)
                   ? ranking::Rank(0)
                   : cutters[slab - 1].from;
    }
    inline ranking::Rank high_rank_of_slab(size_t slab) const
    {
        return slab == cutters.size()
                   ? max_parent_rank
                   : cutters[slab].from;
    }

    std::pair<size_t, bool> search_cutter(double v) const;
};

struct CuttingData;

struct SlabColorInfo
{
    virtual ~SlabColorInfo() = 0;
    /// Create the color info per slab for the slabs defined by the cutters
    virtual void init(AaData const &data, OneDCutters const &x_cutters, OneDCutters const &y_cutters) = 0;
    /// Add the colors of (a subset of) the points in the left, right, bottom and top slab
    /// to the `color_set`.
    virtual void add_border_colors(
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
        CuttingData const &cutters) const = 0;
    virtual size_t heap_size() const = 0;
};

struct CoordGetter
{
    virtual double get(size_t i) const = 0;
};

struct CuttingData
{
    // The cutters ordered in the x-domain
    OneDCutters x;
    // The cutters ordered in the y-domain
    OneDCutters y;
    std::unique_ptr<SlabColorInfo> slab_color_info;

    inline size_t heap_size() const
    {
        using memory_size::heap_size_of;
        return heap_size_of(x) + heap_size_of(y) + slab_color_info->heap_size();
    }

    static CuttingData new_deterministic(size_t sample_size, AaData const &data, std::unique_ptr<SlabColorInfo> slab_color_info);

    static std::vector<Cutter> evenly_distributed(size_t sample_size, ranking::Ranking const &ranking, CoordGetter const &getter);
    inline ranking::Rank left_rank_of_slab(size_t x_slb) const
    {
        return x.low_rank_of_slab(x_slb);
    }
    inline ranking::Rank right_rank_of_slab(size_t x_slb) const
    {
        return x.high_rank_of_slab(x_slb);
    }
    inline ranking::Rank bottom_rank_of_slab(size_t y_slb) const
    {
        return y.low_rank_of_slab(y_slb);
    }
    inline ranking::Rank top_rank_of_slab(size_t y_slb) const
    {
        return y.high_rank_of_slab(y_slb);
    }
};

struct ClsInput
{
    AaData const *data;
    OneDCutters const *x_cutters;
    OneDCutters const *y_cutters;

    double x_cutter_half_dist(size_t i1, size_t i2) const;
    double y_cutter_half_dist(size_t i1, size_t i2) const;
};

// A type containing an index in the extended cutters.
// That is, the cutters extended with -inf before and +inf after the values
// #[derive(Clone, Copy, Debug, PartialEq, Eq)]
struct ExtIdx
{
    size_t v;
};

inline ExtIdx operator+(ExtIdx const ei, size_t u)
{
    auto result = ei;
    result.v += u;
    return result;
}

inline ExtIdx operator-(ExtIdx const ei, size_t u)
{
    auto result = ei;
    result.v -= u;
    return result;
}

inline bool operator<(ExtIdx const e1, ExtIdx const e2)
{
    return e1.v < e2.v;
    ;
}

inline ExtIdx start_of_slab(size_t idx)
{
    return ExtIdx{idx};
}
inline ExtIdx end_of_slab(size_t idx)
{
    return ExtIdx{idx + 1};
}

#endif
