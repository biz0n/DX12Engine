#pragma once

#include <Types.h>

#include <unordered_map>
#include <vector>
#include <compare>

namespace Engine
{
    class Name
    {
    public:
        Name();
        Name(const String &name);
        Name(const char *name);

        Name(const Name &other);
        Name(Name &&other) noexcept;

        Name &operator=(const Name &other) noexcept;
        Name &operator=(Name &&other) noexcept;

        uint32 id() const { return mId; }

        bool isValid() const;

        const String &string() const;

        const char* c_str() const;

        auto operator<=>(const Name &other) const = default;

    private:
        uint32 mId;
    };
} // namespace Engine

namespace std
{
    template <>
    struct hash<Engine::Name>
    {
        size_t operator()(const Engine::Name &key) const
        {
            return std::hash<uint32>{}(key.id());
        }
    };
} // namespace std