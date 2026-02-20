#ifndef RANKING_45811346857
#define RANKING_45811346857
//! A small library for ordering indexes of collections.
//!
//! Usually, if we need ordered access to a collection, we just sort it.
//! Sometimes, this is not practical or not possible. We may use multiple orderings,
//! for instance, an ordering on x-coordinate and one on y-coordinate for points.
//! Or we may have access to an immutable collection.
//!
//! In those cases this library allows to have a ranking (ordering) of collections that are indexed by integers of type usize.
//! Vec and slices are the obvious candidates.
//!
//! # Example
//!
//! In this example we make a ranking of a vector of 7 integers.
//! Two values (2 and 7) occur twice, so we will have 5 rank values
//! (from 0 up to and including 4).
//!
//! ```
//!        let values = [7, 11, 2, 8, 7, 2, 3];
//!        let ranking = ranking::Ranking::new(values.len(), |a, b| values[a].cmp(&values[b]));
//!        assert_eq!(ranking.len(), values.len());
//!        assert_eq!(ranking.rank_len(), 5);
//!        assert_eq!(ranking.rank_of(2), 0.into());
//!        assert_eq!(ranking.rank_of(1), 4.into());
//!        assert_eq!(ranking.oindex(2), 6);
//!        assert_eq!(ranking.oindex(5), 3);
//!        assert_eq!(ranking.oindex(6), 1);
//!        assert_eq!(ranking.one_of_rank(3.into()), 3);
//!        let rank0_items = ranking.all_of_rank(0.into());
//!        assert_eq!(rank0_items.len(), 2);
//!        for i in rank0_items {
//!            assert!(*i == 2 || *i == 5);
//!        }
//!        assert_eq!(ranking.count_of_rank_range(0.into(), 1.into()), 2);
//!        assert_eq!(ranking.count_of_rank_range(0.into(), 5.into()), 7);
//!        assert_eq!(ranking.count_of_rank_range(3.into(), 3.into()), 0);
//!        assert_eq!(ranking.count_of_rank_range(1.into(), 4.into()), 4);
//! ```

#include <stddef.h>
#include <vector>
#include <algorithm>
#include "mem_size.hpp"

namespace ranking
{
    class Ranking;
}

namespace ranking
{
    // #[derive(Clone, Copy, Debug, Eq, PartialEq, Ord, PartialOrd)]
    class Rank
    {
        size_t v;

    public:
        inline size_t as_n() const
        {
            return this->v;
        }

        inline explicit Rank(size_t v) : v(v) {}
        inline void operator+=(size_t v) { this->v += v; }
        inline void operator-=(size_t v) { this->v -= v; }
        inline size_t heap_size() const { return 0; }
    };

    inline Rank operator+(Rank const &r, size_t v)
    {
        return Rank(r.as_n() + v);
    }
    inline Rank operator-(Rank const &r, size_t v)
    {
        return Rank(r.as_n() - v);
    }
    inline bool operator==(Rank const &r1, Rank const &r2)
    {
        return r1.as_n() == r2.as_n();
    }
    inline bool operator!=(Rank const &r1, Rank const &r2)
    {
        return r1.as_n() != r2.as_n();
    }
    inline bool operator<(Rank const &r1, Rank const &r2)
    {
        return r1.as_n() < r2.as_n();
    }
    inline bool operator<=(Rank const &r1, Rank const &r2)
    {
        return r1.as_n() <= r2.as_n();
    }
    inline bool operator>(Rank const &r1, Rank const &r2)
    {
        return r1.as_n() > r2.as_n();
    }
    inline bool operator>=(Rank const &r1, Rank const &r2)
    {
        return r1.as_n() >= r2.as_n();
    }
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
    /// A ranking of elements, ordered from small to large by a comparison function
    class Ranking
    {
        // A Vec of length sz
        // This is a permutation of 0..sz, where for all i,j
        //   0<=i<j<sz => cmp(order[i], order[j]) != Greater
        std::vector<size_t> order_;
        // A Vec of length sz
        // For all i,j 0<=i<j<sz:
        //   cmp(i,j) = ranks[i].cmp(&ranks[j])
        // For all i o<=i<sz
        //   ranks[i]==0 || Exists(j: ranks[j] = ranks[i]-1)
        std::vector<Rank> ranks_;
        // A Vec of length <=sz with strictly increasing values v, 0<v<=sz
        // For all i, 0<i<sz
        //   if ranks[order[i-1]]<ranks[order[i]], then i is element of next
        // sz is element of next
        // For all elements i of next
        //   i==sz || ranks[order[i-1]]<ranks[order[i]]
        std::vector<size_t> next_;

    public:
        size_t heap_size() const;
        //}

        /// Create a new ranking of `sz` items using comparison function cmp.
        template <class C>
        Ranking(size_t sz, C cmp)
        {
            // order is a permutation of 0..sz ...
            order_.resize(sz, 0);
            std::iota(order_.begin(), order_.end(), 0);
            // ... ordered by cmp
            std::sort(order_.begin(), order_.end(), cmp);

            ranks_.resize(sz, Rank(sz));
            next_.reserve(sz);
            if (sz > 0)
            {
                size_t prev = order_[0];
                Rank cur_rank = Rank(0);
                ranks_[prev] = cur_rank;

                for (size_t i = 1; i < sz; ++i)
                {
                    size_t cur = order_[i];
                    if (cmp(prev, cur))
                    {
                        next_.push_back(i);
                        cur_rank += 1;
                    }
                    ranks_[cur] = cur_rank;
                    prev = cur;
                }
                next_.push_back(sz);
            }
        }
        /// The size of the ranking, which is the value of sz given in new
        inline size_t len() const
        {
            return ranks_.size();
        }
        /// The count of ranks
        inline size_t rank_len() const
        {
            return this->next_.size();
        }
        /// The rank of an item referred to by an index
        /// # Precondition:
        /// 0 <= idx < self.len()
        /// # Guarantees:
        /// - 0 <= rank_of(idx) < self.rank_len()
        /// - For all i and j: cmp(i,j) = rank_of(i).cmp(rank_of(j))
        /// - For all r < self.rank_len(): there is an i such that rank_of(i) = r
        inline Rank rank_of(size_t idx) const
        {
            return this->ranks_[idx];
        }
        /// Return a vector of ranks of all (sz) elements
        ///
        /// ranks()\[i\] = self.rank_of(i)
        inline std::vector<Rank> const &ranks() const
        {
            return ranks_;
        }
        /// Return an iterator over all items of a rank.
        /// # Precondition:
        /// 0 <= rank < self.rank_len()
        /// # Guarantees:
        /// If iter.next() = Some(idx) then self.rank_of(idx) = rank
        std::pair<std::vector<size_t>::const_iterator, std::vector<size_t>::const_iterator> all_of_rank(Rank rank) const;

        /// Return an iterator over all items of an inclusive range of ranks.
        /// # Precondition:
        /// 0 <= start_rank <= end_rank < self.rank_len()
        /// # Guarantees:
        /// If iter.next() = Some(idx) then start_rank <= self.rank_of(idx) <= end_rank
        std::pair<std::vector<size_t>::const_iterator, std::vector<size_t>::const_iterator> all_of_ranks(Rank start_rank, Rank end_rank) const;
        /// Return an arbitrary item having a specific rank.
        /// # Precondition:
        /// 0 <= rank < self.rank_len()
        /// # Guarantees:
        /// self.rank_of(one_of_rank(rank)) = rank
        inline size_t one_of_rank(Rank rank) const
        {
            return order_[next_[rank.as_n()] - 1];
        }
        /// Return the number of items with a certain rank
        /// # Precondition:
        /// 0 <= rank < self.rank_len()
        inline size_t count_of_rank(Rank rank)
        {
            size_t r = rank.as_n();
            return r == 0
                       ? next_[0]
                       : next_[r] - next_[r - 1];
        }

        /// The index that occurs as element i in the ordered items
        /// Return the number of items with a rank between
        /// begin_rank and beyond_rank.
        /// # Precondition:
        /// 0 <= begin_rank <= beyond_rank <= self.rank_len()
        size_t count_of_rank_range(Rank begin_rank, Rank beyond_rank) const;
        /// The index that occurs as element i in the ordered items
        ///
        /// Returns value *self.oindexes\[i\]
        ///
        /// # Precondition:
        /// 0 <= i < self.len()
        inline size_t oindex(size_t i) const { return order_[i]; }
        /// Ordered indexes.
        /// A permutation of 0..self.len()
        ///
        /// For all i,j: if i<j then rank_of(oindexes()\[i]) <=  rank_of(oindexes()\[j])
        inline std::vector<size_t> const &oindexes() const
        {
            return order_;
        }
        /// An iterator that yields one index for every rank
        /// (so self.rank_len() elements in total)
        // pub fn one_per_rank_iter(&self)
        //         ->RankingIter <'_> { RankingIter::new (self, 0.into())
    };
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
/* namespace memory_size
{
    inline size_t heap_size_of(::ranking::Rank const &)
    {
        return 0;
    }
    inline size_t heap_size_of(::ranking::Ranking const &r)
    {
        return r.heap_size();
    }
} */

#endif
