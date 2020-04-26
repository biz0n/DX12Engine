#pragma once

#include <tuple>
#include <optional>
#include <cstdint>
#include <string>
#include <memory>
#include <tchar.h>
#include <wrl.h>

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using float32 = float_t;
using float64 = double_t;

using char8 = char;
using uchar8 = unsigned char;

using String = std::string;
using TString = std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>>;

using Size = std::size_t;
using Index = std::size_t;

using Byte = std::byte;

template <typename T>
using Optional = std::optional<T>;

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <class T, class... Args>
inline SharedPtr<T> MakeShared(Args &&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <class T, class... Args>
inline UniquePtr<T> MakeUnique(Args &&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}