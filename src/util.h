#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <cstdint>
#include <span>

template<typename T>
std::span<const char> as_bytes(std::span<T> span) {
    return {(const char *)span.data(), span.size() * sizeof(T)};
}

template<typename T>
std::span<const char> as_bytes(std::span<const T> span) {
    return {(const char *)span.data(), span.size() * sizeof(T)};
}

#endif
