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
        Name(Name &&other);

        Name &operator=(const Name &other);
        Name &operator=(Name &&other);

        uint32 id() const { return mId; }

        const String &string() const;

        auto operator<=>(const Name &other) const = default;

    private:
        uint32 mId;
    };

    class NameRegistry
    {
    public:
        static const uint32 INVALID_ID = UINT32_MAX;
        static NameRegistry &Instance();

    private:
        NameRegistry();
        ~NameRegistry();

    public:
        uint32 GetId(const String &name);
        const String &GetName(uint32 id) const;

    private:
        std::vector<String> mNames;
        std::unordered_map<String, uint32> mNameToId;
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