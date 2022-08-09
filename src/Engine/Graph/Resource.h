#pragma once

#include <Name.h>
#include <Hash.h>
#include <compare>

namespace Engine::Graph
{
    class Resource
    {
        public:
            Resource(Name name, int32 subresource);
            auto operator<=>(const Resource& other) const = default;

            Name Id;
            int32 Subresource;
    };
}

namespace std
{
    template <>
    struct hash<Engine::Graph::Resource>
    {
        size_t operator()(const Engine::Graph::Resource& key) const
        {
            return std::hash_combine(key.Id, key.Subresource);
        }
    };
} // namespace std