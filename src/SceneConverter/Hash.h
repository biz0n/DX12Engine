#pragma once

#include <functional>

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
    constexpr size_t hash_combine_impl(size_t seed, const Head& head, Tail... tail)
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
} // namespace std