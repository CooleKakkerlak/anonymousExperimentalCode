#include <algorithm>
#include "../../mem_size.hpp"
#include "grid.hpp"
#include "range_trees.hpp"
#include "cell.hpp"

using ranking::Rank;
using std::unordered_set;
using std::vector;
using std::chrono::steady_clock;

SlabColorInfo::~SlabColorInfo()
{
}

size_t AaData::heap_size() const
{
    return memory_size::heap_size_of(points_);
}

struct CutterLess
{
    inline bool operator()(Cutter const &c, double v)
    {
        return c.v < v;
    }
};

std::pair<size_t, bool>
OneDCutters::search_cutter(double v) const
{
    auto iter = std::lower_bound(cutters.begin(), cutters.end(), v, CutterLess());
    bool found = iter != cutters.end() && iter->v == v;
    return std::make_pair(iter - cutters.begin(), found);
}

UuAMCSImpl::UuAMCSImpl(std::vector<ColoredPoint_2> points, size_t slabs, std::unique_ptr<SlabColorInfo> sci, std::string name)
    : name_(name),
      data(std::move(points)),
      range_trees(make_range_trees(data.points())),
      cutting(AasCuttingIntl::new_deterministic(data, slabs, std::move(sci)))
{
}

std::vector<ColorCount> UuAMCSImpl::find_colors(AxisAlignedSquare const &aas) const
{
    vector<Color_> colors = all_relevant_colors(aas);
    auto start = steady_clock::now();

    ColorCount moc = find_most_occurring_color(range_trees, aas, colors);
    auto duration = steady_clock::now() - start;
    auto stats = last_statistics.get();
    if (stats)
    {

        stats->range_search_time = duration;
    }

    if (moc.count == 0)
    {
        return vector<ColorCount>();
    }
    else
    {
        return vector<ColorCount>(1, moc);
    }
}

vector<Color_> UuAMCSImpl::all_relevant_colors(AxisAlignedSquare const &aas) const
{
    auto start = steady_clock::now();
    auto color_set = unordered_set<Color_>();
    auto x_bounds = Bounds(aas.min_x(), aas.max_x());
    auto y_bounds = Bounds(aas.min_y(), aas.max_y());
    // If a side of the aas lies on a cutter, take the outer slab
    auto x_min = get_x_cutter(x_bounds.lower);
    size_t x_min_slb = x_min.first;
    bool need_x_min = !(x_min.second);
    auto x_max = get_x_cutter(x_bounds.upper);
    size_t x_max_slb = x_max.second ? x_max.first + 1 : x_max.first;
    bool need_x_max = !(x_max.second);
    auto y_min = get_y_cutter(y_bounds.lower);
    size_t y_min_slb = y_min.first;
    bool need_y_min = !(y_min.second);
    auto y_max = get_y_cutter(y_bounds.upper);
    size_t y_max_slb = y_max.second ? y_max.first + 1 : y_max.first;
    bool need_y_max = !(y_max.second);
    cutting.cutting_data.slab_color_info->add_border_colors(
        color_set,
        x_min_slb,
        x_max_slb,
        y_min_slb,
        y_max_slb,
        need_x_min,
        need_x_max,
        need_y_min,
        need_y_max,
        aas,
        data,
        cutting.cutting_data);
    auto mid = steady_clock::now();
    bool need_cell_color = x_max_slb - x_min_slb >= 2 && y_max_slb - y_min_slb >= 2;
    if (need_cell_color)
    {
        // x_max_slb - x_min_slb >= 2 && y_max_slb - y_min_slb >= 2
        auto mc0 = cutting.cls.find_cell_color(
            end_of_slab(x_min_slb),
            start_of_slab(x_max_slb),
            end_of_slab(y_min_slb),
            start_of_slab(y_max_slb));
        if (mc0)
        {
            color_set.insert(*mc0);
        }
    }
    auto finish = steady_clock::now();

    vector<Color_> result = vector<Color_>(color_set.begin(), color_set.end());
    auto stats = last_statistics.get();
    if (stats)
    {
        auto dur1 = mid - start;
        auto dur2 = finish - mid;
        stats->slab_time = dur1;
        stats->cell_time = dur2;
        stats->relevant_colors = result.size();
    }
    return result;
}
double ClsInput::x_cutter_half_dist(size_t i, size_t j) const
{
    return (x_cutters->cutters[j].v - x_cutters->cutters[i].v) / 2.0;
}
double ClsInput::y_cutter_half_dist(size_t i, size_t j) const
{
    return (y_cutters->cutters[j].v - y_cutters->cutters[i].v) / 2.0;
}

struct XGetter : CoordGetter
{

    XGetter(AaData const *datap) : datap(datap) {}
    inline double get(size_t i) const
    {
        return datap->points_[i].point.x();
    }

private:
    AaData const *datap;
};

struct YGetter : CoordGetter
{

    YGetter(AaData const *datap) : datap(datap) {}
    inline double get(size_t i) const
    {
        return datap->points_[i].point.y();
    }

private:
    AaData const *datap;
};

CuttingData
CuttingData::new_deterministic(size_t sample_size, AaData const &data, std::unique_ptr<SlabColorInfo> slab_color_info)

{
    auto x_cutters = OneDCutters{
        evenly_distributed(sample_size, data.x_ranking, XGetter(&data)),
        Rank(data.x_ranking.rank_len() - 1),
    };
    auto y_cutters = OneDCutters{
        evenly_distributed(sample_size, data.y_ranking, YGetter(&data)),
        Rank(data.y_ranking.rank_len() - 1),
    };
    slab_color_info->init(data, x_cutters, y_cutters);
    return CuttingData{
        x_cutters,
        y_cutters,
        std::move(slab_color_info),
    };
}

std::vector<Cutter> CuttingData::evenly_distributed(size_t sample_size, ranking::Ranking const &ranking, CoordGetter const &getter)
{
    size_t rank_len = ranking.rank_len();
    size_t count = std::min(sample_size, rank_len);
    size_t chunk_size = ranking.len() / (count + 1);
    size_t rest = ranking.len() % (count + 1);
    auto result = vector<Cutter>();
    result.reserve(count);
    auto last_rank = Rank(std::numeric_limits<size_t>::max());
    size_t oindex = 0;
    while (true)
    {
        if (count == 0)
        {
            break;
        }
        oindex += chunk_size;

        size_t idx = ranking.oindex(oindex);
        Rank rank = ranking.rank_of(idx);
        if (rank == last_rank)
        {
            // take an element of the next rank and adapt chunk_size
            last_rank += 1;
            if (last_rank == Rank(rank_len))
            {
                // cannot add more
                break;
            }
            idx = ranking.one_of_rank(last_rank);
            oindex = ranking.oindex(idx);
            // Because we skipped some items, we have to adapt chunk_size and rest
            // We still have to add count-1 items and have ranking.len-oindex items
            chunk_size = (ranking.len() - oindex) / count;
            rest = (ranking.len() - oindex) % count;
        }
        else
        {
            last_rank = rank;
        }
        result.push_back(Cutter{
            getter.get(idx),
            last_rank,
        });
        if (rest > 0)
        {
            oindex += 1;
            rest -= 1;
        }
        count -= 1;
    }
    return result;
}
