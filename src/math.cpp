/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2014-2024 Gothel Software e.K.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cstdint>
#include <cinttypes>
#include <cstring>

#include <ctime>

#include <algorithm>

#include <jau/debug.hpp>
#include <jau/string_util.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/aabbox3f.hpp>
#include <jau/math/quaternion.hpp>
#include <jau/math/geom/frustum.hpp>
#include <jau/math/geom/plane/affine_transform.hpp>

jau::math::Mat4f& jau::math::Mat4f::setToRotation(const jau::math::Quaternion& q) noexcept {
    // pre-multiply scaled-reciprocal-magnitude to reduce multiplications
    const float norm = q.magnitudeSquared();
    if ( jau::is_zero(norm) ) {
        // identity matrix -> srecip = 0f
        load_identity();
        return *this;
    }
    float srecip;
    if ( jau::equals(1.0f, norm) ) {
        srecip = 2.0f;
    } else {
        srecip = 2.0f / norm;
    }

    const float x = q.x();
    const float y = q.y();
    const float z = q.z();
    const float w = q.w();

    const float xs = srecip * x;
    const float ys = srecip * y;
    const float zs = srecip * z;

    const float xx = x  * xs;
    const float xy = x  * ys;
    const float xz = x  * zs;
    const float xw = xs * w;
    const float yy = y  * ys;
    const float yz = y  * zs;
    const float yw = ys * w;
    const float zz = z  * zs;
    const float zw = zs * w;

    m00 = 1.0f - ( yy + zz );
    m01 =        ( xy - zw );
    m02 =        ( xz + yw );
    m03 = 0.0f;

    m10 =        ( xy + zw );
    m11 = 1.0f - ( xx + zz );
    m12 =        ( yz - xw );
    m13 = 0.0f;

    m20 =        ( xz - yw );
    m21 =        ( yz + xw );
    m22 = 1.0f - ( xx + yy );
    m23 = 0.0f;

    m30 = m31 = m32 = 0.0f;
    m33 = 1.0f;
    return *this;
}

jau::math::Quaternion& jau::math::Mat4f::getRotation(jau::math::Quaternion& res) const noexcept {
    return res.setFromMat3(*this);
}

jau::math::geom::Frustum& jau::math::Mat4f::getFrustum(jau::math::geom::Frustum& frustum) noexcept {
    return frustum.setFromMat4(*this);
}

std::string& jau::row_to_string(std::string& sb, const std::string& f,
                                const float a[],
                                const jau::nsize_t rows, const jau::nsize_t columns,
                                const bool rowMajorOrder, const jau::nsize_t row) noexcept {
  if(rowMajorOrder) {
      for(jau::nsize_t c=0; c<columns; ++c) {
          sb.append( jau::format_string(f.c_str(), a[ row*columns + c ] ) );
          sb.append(", ");
      }
  } else {
      for(jau::nsize_t r=0; r<columns; ++r) {
          sb.append( jau::format_string(f.c_str(), a[ row + r*rows ] ) );
          sb.append(", ");
      }
  }
  return sb;
}

std::string& jau::mat_to_string(std::string& sb, const std::string& rowPrefix, const std::string& f,
                                const float a[], const jau::nsize_t rows, const jau::nsize_t columns,
                                const bool rowMajorOrder) noexcept {
    sb.append(rowPrefix).append("{ ");
    for(jau::nsize_t i=0; i<rows; ++i) {
        if( 0 < i ) {
            sb.append(rowPrefix).append("  ");
        }
        row_to_string(sb, f, a, rows, columns, rowMajorOrder, i);
        sb.append("\n");
    }
    sb.append(rowPrefix).append("}").append("\n");
    return sb;
}

std::string jau::math::Mat4f::toString(const std::string& rowPrefix, const std::string& f) const noexcept {
    std::string sb;
    float tmp[16];
    get(tmp);
    return jau::mat_to_string(sb, rowPrefix, f, tmp, 4, 4, false /* rowMajorOrder */); // creates a copy-out!
}

jau::math::AABBox3f& jau::math::AABBox3f::resize(const jau::math::AABBox3f& newBox, const jau::math::geom::plane::AffineTransform& t, Vec3f& tmpV3) noexcept {
    /** test low */
    {
        const jau::math::Vec3f& newBL = t.transform(newBox.low(), tmpV3);
        if (newBL.x < m_lo.x) {
            m_lo.x = newBL.x;
        }
        if (newBL.y < m_lo.y) {
            m_lo.y = newBL.y;
        }
        if (newBL.z < m_lo.z) {
            m_lo.z = newBL.z;
        }
    }

    /** test high */
    {
        const jau::math::Vec3f& newTR = t.transform(newBox.high(), tmpV3);
        if (newTR.x > m_hi.x) {
            m_hi.x = newTR.x;
        }
        if (newTR.y > m_hi.y) {
            m_hi.y = newTR.y;
        }
        if (newTR.z > m_hi.z) {
            m_hi.z = newTR.z;
        }
    }
    computeCenter();
    return *this;
}
