#ifndef MEM_SIZE_25610935
#define MEM_SIZE_25610935

#include <stddef.h>
#include <vector>
#include <unordered_map>
#include <optional>
#include <numeric>

namespace memory_size
{

    inline size_t heap_size_of(bool) { return 0; }
    inline size_t heap_size_of(signed char) { return 0; }
    inline size_t heap_size_of(unsigned char) { return 0; }
    inline size_t heap_size_of(short) { return 0; }
    inline size_t heap_size_of(unsigned short) { return 0; }
    inline size_t heap_size_of(int) { return 0; }
    inline size_t heap_size_of(unsigned int) { return 0; }
    inline size_t heap_size_of(long) { return 0; }
    inline size_t heap_size_of(unsigned long) { return 0; }
    inline size_t heap_size_of(long long) { return 0; }
    inline size_t heap_size_of(unsigned long long) { return 0; }
    inline size_t heap_size_of(double) { return 0; }
    inline size_t heap_size_of(float) { return 0; }

    template <class T>
    size_t heap_size_of(T const &t);

    template <class F, class S>
    size_t heap_size_of(std::pair<F, S> const &p)
    {
        return heap_size_of(p.first) + heap_size_of(p.second);
    }

    template <class T>
    size_t heap_size_of(std::optional<T> const &o)
    {
        return o ? heap_size_of(*o) : 0;
    }

    namespace
    {

        template <class T>
        class heap_adder;
    }

    template <class T>
    size_t heap_size_of(std::vector<T> const &v)
    {
        return v.capacity() * sizeof(T) + std::reduce(v.begin(), v.end(), size_t(0), heap_adder<T>());
    }

    template <>
    inline size_t heap_size_of<>(std::vector<size_t> const &v)
    {
        return v.capacity() * sizeof(size_t);
    }

    template <class K, class V>
    size_t heap_size_of(std::unordered_map<K, V> const &m)
    {
        // The first term should probably be computed in terms of
        // bucket_count and bucket_size.
        // Using size is probably an underestimation.
        typedef typename std::unordered_map<K, V>::value_type vt;
        return m.size() * (sizeof(K) + sizeof(V)) + std::reduce(m.begin(), m.end(), size_t(0), heap_adder<vt>());
    }

    template <class T>
    size_t heap_size_of(T const &t)
    {
        return t.heap_size();
    }

    template <class T>
    size_t total_size_of(T const &t)
    {
        return sizeof(T) + heap_size_of(t);
    }

    namespace
    {

        template <class T>
        class heap_adder
        {
        public:
            inline size_t operator()(size_t a, size_t b)
            {
                return a + b;
            }
            inline size_t operator()(size_t a, T const &t)
            {
                return a + heap_size_of(t);
            }
            inline size_t operator()(T const &t, size_t a)
            {
                return a + heap_size_of(t);
            }
            inline size_t operator()(T const &t1, T const &t2)
            {
                return heap_size_of(t1) + heap_size_of(t2);
            }
        };

    }

}

#endif