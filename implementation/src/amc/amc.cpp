#include "amc.h"

using ranking::Rank;
using ranking::Ranking;
using std::optional;
using std::vector;

class indir_smaller_on_x_rv
{
    vector<ColoredPoint_2> const &cpoints;

public:
    indir_smaller_on_x_rv(vector<ColoredPoint_2> const &points) : cpoints(points) {}
    inline bool operator()(size_t i1, double v)
    {
        return cpoints[i1].point.x() < v;
    }
};

class indir_smaller_on_y_rv
{
    vector<ColoredPoint_2> const &cpoints;

public:
    indir_smaller_on_y_rv(vector<ColoredPoint_2> const &points) : cpoints(points) {}
    inline bool operator()(size_t i1, double v)
    {
        return cpoints[i1].point.y() < v;
    }
};

class indir_smaller_on_x_rr
{
    vector<ColoredPoint_2> const &cpoints;

public:
    indir_smaller_on_x_rr(vector<ColoredPoint_2> const &points) : cpoints(points) {}
    inline bool operator()(size_t i1, size_t i2)
    {
        return cpoints[i1].point.x() < cpoints[i2].point.x();
    }
};

class indir_smaller_on_y_rr
{
    vector<ColoredPoint_2> const &cpoints;

public:
    indir_smaller_on_y_rr(vector<ColoredPoint_2> const &points) : cpoints(points) {}
    inline bool operator()(size_t i1, size_t i2)
    {
        return cpoints[i1].point.y() < cpoints[i2].point.y();
    }
};

AaData::AaData(vector<ColoredPoint_2> points)
    : points_(std::move(points)),
      x_ranking(Ranking(points_.size(), indir_smaller_on_x_rr(points_))),
      y_ranking(Ranking(points_.size(), indir_smaller_on_y_rr(points_)))
{
}
vector<ColoredPoint_2> const &AaData::points() const
{
    return points_;
}
optional<Rank> AaData::search_rank_at_least_x(double x) const
{
    auto opt_rank = search_rank_x(x);
    switch (opt_rank.t)
    {
    case RankSearchType::NotFound:
    case RankSearchType::Found:
        return optional<Rank>(opt_rank.r);

        // case RankSearchType::Beyond:
        //     return std::optional<Rank>();
    }
    return std::optional<Rank>();
}
optional<Rank> AaData::search_rank_at_most_x(double x) const
{
    RankSearchResult opt_rank = search_rank_x(x);
    switch (opt_rank.t)
    {
    case RankSearchType::Beyond:
    case RankSearchType::NotFound:
        if (opt_rank.r.as_n() == 0)
        {
            return std::optional<Rank>();
        }
        else
        {
            auto rank = opt_rank.r - 1;
            return optional<Rank>(rank);
        }
        // case RankSearchType::Found:
        //     return optional<Rank>(opt_rank.r);
    }
    return optional<Rank>(opt_rank.r);
}

RankSearchResult AaData::search_rank_x(double x) const
{
    std::vector<size_t> const &oindexes = x_ranking.oindexes();
    auto first = std::lower_bound(oindexes.begin(), oindexes.end(), x, indir_smaller_on_x_rv(points_));
    if (first == oindexes.end())
    {
        return RankSearchResult{RankSearchType::Beyond, Rank(x_ranking.rank_len())};
    }
    size_t idx = *first;
    ranking::Rank rank = x_ranking.rank_of(idx);
    if (points_[*first].point.x() == x)
    {
        return RankSearchResult{RankSearchType::Found, rank};
    }
    else
    {
        return RankSearchResult{RankSearchType::NotFound, rank};
    }
}

optional<Rank> AaData::search_rank_at_least_y(double y) const
{
    auto opt_rank = search_rank_y(y);
    switch (opt_rank.t)
    {
    case RankSearchType::NotFound:
    case RankSearchType::Found:
        return optional<Rank>(opt_rank.r);

        // case RankSearchType::Beyond:
        //     return std::optional<Rank>();
    }
    return std::optional<Rank>();
}
optional<Rank> AaData::search_rank_at_most_y(double y) const
{
    RankSearchResult opt_rank = search_rank_y(y);
    switch (opt_rank.t)
    {
    case RankSearchType::Beyond:
    case RankSearchType::NotFound:
        if (opt_rank.r.as_n() == 0)
        {
            return std::optional<Rank>();
        }
        else
        {
            auto rank = opt_rank.r - 1;
            return optional<Rank>(rank);
        }
        // case RankSearchType::Found:
        //     return optional<Rank>(opt_rank.r);
    }
    return optional<Rank>(opt_rank.r);
}
RankSearchResult AaData::search_rank_y(double y) const
{
    std::vector<size_t> const &oindexes = y_ranking.oindexes();
    auto first = std::lower_bound(oindexes.begin(), oindexes.end(), y, indir_smaller_on_y_rv(points_));
    if (first == oindexes.end())
    {
        return RankSearchResult{RankSearchType::Beyond, Rank(y_ranking.rank_len())};
    }
    size_t idx = *first;
    ranking::Rank rank = y_ranking.rank_of(idx);
    if (points_[*first].point.y() == y)
    {
        return RankSearchResult{RankSearchType::Found, rank};
    }
    else
    {
        return RankSearchResult{RankSearchType::NotFound, rank};
    }
}

std::optional<std::pair<size_t, vector<Color_>>> AasModeColorSolver::mode_colors(AxisAlignedSquare const &aas) const
{
    vector<ColorCount> color_counts = find_colors(aas);
    if (color_counts.empty())
    {
        return std::nullopt;
    }
    size_t count = 0;
    auto colors = vector<Color_>();
    for (ColorCount const &a : color_counts)
    {
        if (a.count >= count)
        {
            if (a.count > count)
            {
                colors.clear();
                count = a.count;
            }
            colors.push_back(a.color);
        }
    }
    return std::optional(make_pair(count, colors));
}

struct GreaterByCount
{
    /// A comparison function which, when used for sorting,
    /// orders the elements on decreasing count values
    /// (and then on increasing col values)
    bool operator()(ColorCount const &a, ColorCount const &b)
    {
        if (a.count > b.count)
            return true;
        if (a.count < b.count)
            return false;
        return a.color < b.color;
    }
};

std::optional<ColorCount>
AasModeColorSolver::a_mode_color(AxisAlignedSquare const &aas) const
{
    vector<ColorCount> color_counts = find_colors(aas);
    if (color_counts.empty())
        return std::nullopt;
    auto mx = std::min_element(color_counts.begin(), color_counts.end(), GreaterByCount());
    return std::optional(*mx);
}

std::vector<ColorCount> AasModeColorSolver::find_colors_dec(AxisAlignedSquare const &aas) const
{
    vector<ColorCount> color_counts = find_colors(aas);
    std::sort(color_counts.begin(), color_counts.end(), GreaterByCount());
    return color_counts;
}