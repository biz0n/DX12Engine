#pragma once

#include <functional>
#include <iterator>

#include <vector>

namespace std
{
    constexpr size_t hash_combine_math(size_t seed, size_t hash)
    {
        return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

    constexpr size_t hash_combine_impl(size_t seed)
    {
        return seed;
    }

    template <typename Head, typename... Tail>
    constexpr size_t hash_combine_impl(size_t seed, const Head &head, Tail... tail)
    {
        std::hash<Head> hash;
        size_t newSeed = hash_combine_math(seed, hash(head));
        return hash_combine_impl(newSeed, tail...);
    }

    constexpr size_t hash_combine()
    {
        return 0;
    }

    template <typename... T>
    constexpr size_t hash_combine(T... t)
    {
        return hash_combine_impl(0, t...);
    }

    template <typename Iterator, typename T = typename std::iterator_traits<Iterator>::value_type>
    size_t hash_combine(Iterator begin, Iterator end)
    {
        std::hash<T> hash;

        size_t seed = 0;
        for (Iterator i = begin; i != end; i++)
        {
            seed = hash_combine_math(seed, hash(*i));
        }

        return seed;
    }

    template <typename T, size_t N>
    size_t hash_combine(const T (&x)[N])
    {
        std::hash<T> hash;
        size_t seed = 0;
        for (int i = 0; i < N; ++i)
        {
            seed = hash_combine_math(seed, hash(x[i]));
        }

        return seed;
    }

    

    template <typename T>
    auto operator<=>(const std::vector<T> &left, const std::vector<T> &right)
    {
        if (left < right)
        {
            return std::strong_ordering::less;
        }
        else if (left > right)
        {
            return std::strong_ordering::greater;
        }

        return std::strong_ordering::equal;
    }

    template <typename T, Size N>
    std::strong_ordering operator<=>(const std::array<T, N> &left, const std::array<T, N> &right)
    {
        if (left < right)
        {
            return std::strong_ordering::less;
        }
        else if (left > right)
        {
            return std::strong_ordering::greater;
        }

        return std::strong_ordering::equal;
    }
} // namespace std