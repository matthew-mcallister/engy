#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <ostream>

#include <immintrin.h>

#include "math/vector.h"

/// @brief SIMD-backed 4x4 matrix
struct Matrix4 {
    Vector4 columns[4];

    Matrix4() = default;
    Matrix4(Vector4 x, Vector4 y, Vector4 z, Vector4 w) : columns{x, y, z, w} {}

    Vector4 operator[](int index) const { return columns[index]; }
    Vector4 &operator[](int index) { return columns[index]; }

    Vector4 x() const { return columns[0]; }
    Vector4 y() const { return columns[1]; }
    Vector4 z() const { return columns[2]; }
    Vector4 w() const { return columns[3]; }

    Matrix4 operator+(Matrix4 other) const {
        return Matrix4(columns[0] + other[0], columns[1] + other[1],
                       columns[2] + other[2], columns[3] + other[3]);
    }

    Matrix4 operator-(Matrix4 other) const {
        return Matrix4(columns[0] - other[0], columns[1] - other[1],
                       columns[2] - other[2], columns[3] - other[3]);
    }

    Matrix4 operator*(float s) const {
        return Matrix4(s * columns[0], s * columns[1], s * columns[2],
                       s * columns[3]);
    }

    Vector4 operator*(Vector4 v) const {
        return v[0] * columns[0] + v[1] * columns[1] + v[2] * columns[2] +
               v[3] * columns[3];
    }

    Matrix4 operator*(Matrix4 other) const {
        return Matrix4(columns[0] * other[0], columns[1] * other[1],
                       columns[2] * other[2], columns[3] * other[3]);
    }

    Matrix4 operator/(float s) const {
        return Matrix4(columns[0] / s, columns[1] / s, columns[2] / s,
                       columns[3] / s);
    }

    Matrix4 transpose() const {
        return {{columns[0][0], columns[1][0], columns[2][0], columns[3][0]},
                {columns[0][1], columns[1][1], columns[2][1], columns[3][1]},
                {columns[0][2], columns[1][2], columns[2][2], columns[3][2]},
                {columns[0][3], columns[1][3], columns[2][3], columns[3][3]}};
    }

    // FIXME: Remove this once Matrix3 is implemented
    Matrix4 transpose3() const {
        return {{columns[0][0], columns[1][0], columns[2][0], 0},
                {columns[0][1], columns[1][1], columns[2][1], 0},
                {columns[0][2], columns[1][2], columns[2][2], 0},
                columns[3]};
    }

    Matrix4 offset(Vector3 offset) const {
        Matrix4 m = *this;
        m[3] = offset.xyz1();
        return m;
    }

    /// @brief Computes the inverse of a rigid transformation.
    ///
    /// The matrix must take the form
    ///   [ R | v ]
    ///   [---+---]
    ///   [ 0 | 1 ]
    /// where R is an orthogonal 3x3 matrix and v is a 3x1 vector. The
    /// reverse is then
    ///   [ R^T | -R^T v ]
    ///   [-----+--------]
    ///   [  0  |    1   ]
    Matrix4 rigid_inverse() const {
        auto inv = transpose3();

        auto v = inv[3];
        inv[3] = 0;
        v = -(inv * v);
        inv[3] = v;
        inv[3][3] = 1;

        return inv;
    }
};

inline Vector4 operator*(Vector4 v, Matrix4 mat) {
    return mat.transpose() * v;
}

inline Matrix4 operator*(float s, Matrix4 mat) {
    return Matrix4(s * mat[0], s * mat[1], s * mat[2], s * mat[3]);
}

std::ostream &operator<<(std::ostream &os, const Matrix4 &mat);

// TODO: Actually implement Matrix3.
// Unlike Vector3, which is the same size as Vector4, Matrix3 is smaller
// and faster than Matrix4.
typedef Matrix4 Matrix3;

#endif
