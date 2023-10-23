#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

// TODO: Use portable compiler intrinsics?
#include <immintrin.h>

namespace detail {

/// Returns a vector where all 4 elements are equal to a · b.
inline __m128 dot(__m128 a, __m128 b) {
    auto prod = _mm_mul_ps(a, b);
    auto sum = _mm_hadd_ps(prod, prod);
    return _mm_hadd_ps(sum, sum);
}

} // namespace detail

class Vector4;
typedef Vector4 Vector3;

/// @brief SIMD-backed 4-vector
struct Vector4 {
    __m128 data;

    Vector4() : data{_mm_set1_ps(0)} {}
    Vector4(float s) : data{_mm_set1_ps(s)} {}
    Vector4(float x, float y, float z, float w) : data{x, y, z, w} {}
    Vector4(__m128 data) : data{data} {}

    float operator[](int index) const { return data[index]; }
    float &operator[](int index) { return data[index]; }

    float x() const { return data[0]; }
    float y() const { return data[1]; }
    float z() const { return data[2]; }
    float w() const { return data[3]; }

    float r() const { return data[0]; }
    float g() const { return data[1]; }
    float b() const { return data[2]; }
    float a() const { return data[3]; }

    float u() const { return data[0]; }
    float v() const { return data[1]; }
    float s() const { return data[2]; }
    float t() const { return data[3]; }

    Vector4 operator+(Vector4 other) const {
        return Vector4(_mm_add_ps(data, other.data));
    }

    Vector4 operator+(float scalar) const { return *this + Vector4{scalar}; }

    Vector4 operator-(Vector4 other) const {
        return Vector4(_mm_sub_ps(data, other.data));
    }

    Vector4 operator-(float scalar) const { return *this - Vector4{scalar}; }
    Vector4 operator-() const { return -data; }

    Vector4 operator*(Vector4 other) const {
        return Vector4(_mm_mul_ps(data, other.data));
    }

    Vector4 operator*(float scalar) const { return *this * Vector4{scalar}; }

    Vector4 operator/(Vector4 other) const {
        return Vector4(_mm_div_ps(data, other.data));
    }

    Vector4 operator/(float scalar) const { return *this / Vector4{scalar}; }

    float dot(Vector4 other) const { return detail::dot(data, other.data)[0]; }

    float length_sq() const { return dot(*this); }
    float length() const { return _mm_sqrt_ss(detail::dot(data, data))[0]; }
    Vector4 normalized() const {
        auto lensq = detail::dot(data, data);
        auto invlen = _mm_rsqrt_ps(lensq);
        return data * invlen;
    }

    Vector4 yzxw() const { return _mm_shuffle_ps(data, data, 0b11'00'10'01); }
    Vector4 zxyw() const { return _mm_shuffle_ps(data, data, 0b11'01'00'10); }
    Vector4 xyz0() const { return {data[0], data[1], data[2], 0}; }
    Vector4 xyz1() const { return {data[0], data[1], data[2], 1}; }

    Vector4 cross(Vector4 other) const {
        return yzxw() * other.zxyw() - zxyw() * other.yzxw();
    }

    Vector4 min(Vector4 other) const { return _mm_min_ps(data, other.data); }
    Vector4 max(Vector4 other) const { return _mm_max_ps(data, other.data); }

    // TODO: Replace once Vector3 is implemented
    bool lt3(Vector3 other) const {
        int cmp = _mm_movemask_ps(_mm_cmplt_ps(data, other.data));
        return (cmp & 0b111) == 0b111;
    }
    bool le3(Vector3 other) const {
        int cmp = _mm_movemask_ps(_mm_cmple_ps(data, other.data));
        return (cmp & 0b111) == 0b111;
    }
    bool gt3(Vector3 other) const { return !le3(other); }
    bool ge3(Vector3 other) const { return !lt3(other); }
};

inline Vector4 operator+(float scalar, Vector4 vector) {
    return Vector4{scalar} + vector;
}

inline Vector4 operator-(float scalar, Vector4 vector) {
    return Vector4{scalar} - vector;
}

inline Vector4 operator*(float scalar, Vector4 vector) {
    return Vector4{scalar} * vector;
}

inline Vector4 operator/(float scalar, Vector4 vector) {
    return Vector4{scalar} / vector;
}

inline Vector4 vec4() {
    return {};
}

inline Vector4 vec4(float s) {
    return {s};
}

inline Vector4 vec4(float x, float y, float z, float w) {
    return {x, y, z, w};
}

inline Vector3 vec3() {
    return {0};
}

inline Vector3 vec3(float s) {
    return {s, s, s, 0};
}

inline Vector3 vec3(float x, float y, float z) {
    return {x, y, z, 0};
}

#endif
