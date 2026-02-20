#include "ranking.hpp"
#include <cassert>

using ranking::Rank;
using ranking::Ranking;
using std::vector;

class indir_less
{
    vector<int> const *vp;

public:
    indir_less(vector<int> const *vp) : vp(vp) {}
    inline bool operator()(size_t a, size_t b)
    {
        return (*vp)[a] < (*vp)[b];
    }
};

bool int_ranking()
{
    vector<int> values{7, 11, 2, 8, 7, 2, 3};
    Ranking ranking = Ranking(values.size(), indir_less(&values));
    assert(ranking.len() == values.size());
    assert(ranking.rank_len() == 5);
    assert(ranking.rank_of(0).as_n() == 2);
    assert(ranking.rank_of(6).as_n() == 1);
    assert(ranking.rank_of(2).as_n() == 0);
    assert(ranking.rank_of(1).as_n() == 4);
    assert(ranking.oindex(2) == 6);
    assert(ranking.oindex(5) == 3);
    assert(ranking.oindex(6) == 1);
    assert(ranking.one_of_rank(Rank(1)) == 6);
    assert(ranking.one_of_rank(Rank(3)) == 3);
    assert(ranking.one_of_rank(Rank(4)) == 1);
    auto rank0_items = ranking.all_of_rank(Rank(0));
    assert(rank0_items.second - rank0_items.first == 2);
    for (auto i = rank0_items.first; i != rank0_items.second; i++)
    {
        assert(*i == 2 || *i == 5);
    }
    auto rank01_items = ranking.all_of_ranks(Rank(0), Rank(1));
    for (auto i = rank01_items.first; i != rank01_items.second; i++)

    {
        assert(*i == 2 || *i == 5 || *i == 6);
    }
    assert(ranking.count_of_rank_range(Rank(0), Rank(1)) == 2);
    assert(ranking.count_of_rank_range(Rank(0), Rank(5)) == 7);
    assert(ranking.count_of_rank_range(Rank(3), Rank(3)) == 0);
    assert(ranking.count_of_rank_range(Rank(1), Rank(4)) == 4);
    return true;
}

int main()
{
    bool all_ok = int_ranking();
    return all_ok ? 0 : 1;
}
