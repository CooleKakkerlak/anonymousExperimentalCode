#include "ranking.hpp"

using memory_size::heap_size_of;

namespace ranking
{

    /*
    /// Type returned by [rank_range] and [incl_rank_range].
    pub struct RankIter
    {
        cur : Rank,
              beyond : Rank,
    }

    impl Iterator for RankIter
    {
        type Item = Rank;

        fn next(&mut self) -> Option<Self::Item>
        {
            if self
                .cur >= self.beyond
                {
                    None
                }
            else
            {
                let result = self.cur;
                self.cur += 1;
                Some(result)
            }
        }
        fn size_hint(&self)->(usize, Option<usize>)
        {
            if self
                .cur >= self.beyond
                {
                    (0, Some(0))
                }
            else
            {
                let diff = self.beyond.v - self.cur.v;
                (diff, Some(diff))
            }
        }
    }

    /// Return an iterator over a range of ranks (Half open interval)
    pub fn rank_range(start : Rank, beyond : Rank) -> RankIter
    {
        RankIter
        {
        cur:
            start, beyond
        }
    }

    /// Return an iterator over a range of ranks (Closed interval)
    pub fn incl_rank_range(start : Rank, end : Rank) -> RankIter
    {
        RankIter
        {
        cur:
            start,
                beyond : end + 1,
        }
    }
        */

    std::pair<std::vector<size_t>::const_iterator, std::vector<size_t>::const_iterator> Ranking::all_of_rank(Rank rank) const
    {
        size_t start = (rank.as_n() == 0) ? 0
                                          : next_[rank.as_n() - 1];
        size_t end = next_[rank.as_n()];
        return std::make_pair(order_.begin() + start, order_.begin() + end);
    }

    std::pair<std::vector<size_t>::const_iterator, std::vector<size_t>::const_iterator> Ranking::all_of_ranks(Rank start_rank, Rank end_rank) const
    {
        size_t start = start_rank.as_n() == 0 ? 0 : next_[start_rank.as_n() - 1];
        size_t end = next_[end_rank.as_n()];
        return std::make_pair(order_.begin() + start, order_.begin() + end);
    }

    size_t Ranking::count_of_rank_range(Rank begin_rank, Rank beyond_rank) const
    {
        size_t min = begin_rank.as_n() == 0 ? 0 : next_[begin_rank.as_n() - 1];
        size_t max = beyond_rank.as_n() == 0 ? 0 : next_[beyond_rank.as_n() - 1];
        return max - min;
    }
    size_t Ranking::heap_size() const
    {
        return heap_size_of(order_) + heap_size_of(ranks_) + heap_size_of(next_);
    }
}

/*
/// The type returned by [Ranking::one_per_rank_iter]
pub struct RankingIter<'a> {
    ranking: &'a Ranking,
    rank: Rank,
}

impl<'a> RankingIter<'a>
{
    fn new (ranking : &Ranking, rank : Rank)->RankingIter <'_> { RankingIter { ranking, rank }
}
}

impl<'a> Iterator for RankingIter<' a>
{
    type Item = usize;

    fn next(&mut self) -> Option<Self::Item>
    {
        if self
            .rank == self.ranking.rank_len().into()
            {
                None
            }
        else
        {
            let rank = self.rank;
            self.rank += 1;
            Some(self.ranking.one_of_rank(rank))
        }
    }
}
    */
