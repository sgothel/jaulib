/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#ifndef JAU_MATH_GEOM_GEOM3F_HPP_
#define JAU_MATH_GEOM_GEOM3F_HPP_

#include <limits>
#include <jau/darray.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/geom/geom.hpp>
#include <jau/math/geom/aabbox3f.hpp>

namespace jau::math::geom {

    /** \addtogroup Math
     *
     *  @{
     */

    struct LineSeg3f {
        Point3f p0;
        Point3f p1;

        /**
         * Scale this line segment with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr LineSeg3f& operator*=(const float s ) noexcept {
            p0 *= s;
            p1 *= s;
            return *this;
        }

        std::string toString() const noexcept { return "L[" + p0.toString() + ", " + p1.toString() + "]"; }

        /**
         * Compute intersection between two lines segments
         * @param result storage for the intersection coordinates if the lines intersect, otherwise unchanged
         * @param a vertex 1 of first line
         * @param b vertex 2 of first line
         * @param c vertex 1 of second line
         * @param d vertex 2 of second line
         * @return true if the line segments intersect, otherwise false
         */
        bool intersects(Point3f& result, const LineSeg3f & o) {
            const float determinant = ( p0.x - p1.x ) * ( o.p0.y - o.p1.y) - ( p0.y - p1.y ) * ( o.p0.x - o.p1.x );
            if( 0 == determinant ) {
                return false;
            }
            const float alpha = p0.x * p1.y - p0.y * p1.x;
            const float beta = o.p0.x*o.p1.y-o.p0.y*o.p1.y;
            const float xi = ((o.p0.x-o.p1.x)*alpha-(p0.x-p1.x)*beta)/determinant;

            const float gamma0 = (xi - p0.x) / (p1.x - p0.x);
            const float gamma1 = (xi - o.p0.x) / (o.p1.x - o.p0.x);
            if(gamma0 <= 0 || gamma0 >= 1 || gamma1 <= 0 || gamma1 >= 1) {
                return false;
            }
            const float yi = ((o.p0.y-o.p1.y)*alpha-(p0.y-p1.y)*beta)/determinant;
            result.x = xi;
            result.y = yi;
            return true;
        }

        /**
         * Compute intersection between two lines segments
         * @param a vertex 1 of first line
         * @param b vertex 2 of first line
         * @param c vertex 1 of second line
         * @param d vertex 2 of second line
         * @return true if the line segments intersect, otherwise false
         */
        bool intersects(const LineSeg3f & o) {
            const float determinant = ( p0.x - p1.x ) * ( o.p0.y - o.p1.y) - ( p0.y - p1.y ) * ( o.p0.x - o.p1.x );
            if( 0 == determinant ) {
                return false;
            }
            const float alpha = p0.x * p1.y - p0.y * p1.x;
            const float beta = o.p0.x*o.p1.y-o.p0.y*o.p1.y;
            const float xi = ((o.p0.x-o.p1.x)*alpha-(p0.x-p1.x)*beta)/determinant;

            const float gamma0 = (xi - p0.x) / (p1.x - p0.x);
            const float gamma1 = (xi - o.p0.x) / (o.p1.x - o.p0.x);
            if(gamma0 <= 0 || gamma0 >= 1 || gamma1 <= 0 || gamma1 >= 1) {
                return false;
            }
            return true;
        }

        bool intersects(const AABBox3f& box) const noexcept {
            // separating axis theorem.
            const Vec3f d = (p1 - p0) * 0.5f; // half lineseg direction
            const Vec3f e = (box.high() - box.low()) * 0.5f;
            const Vec3f aabb_center = (box.low() + box.high()) * 0.5f;
            const Vec3f lseg_center = p0 + d;
            const Vec3f c = lseg_center - aabb_center;
             Vec3f ad(std::abs(d.x), std::abs(d.y), std::abs(d.z));
             if (std::abs(c.x) > e.x + ad.x)
                 return false;
             if (std::abs(c.y) > e.y + ad.y)
                 return false;
             if (std::abs(c.z) > e.z + ad.z)
                 return false;
             if (std::abs(d.y * c.z - d.z * c.y) > e.y * ad.z + e.z * ad.y + std::numeric_limits<float>::epsilon()) {
                 return false;
             }
             if (std::abs(d.z * c.x - d.x * c.z) > e.z * ad.x + e.x * ad.z + std::numeric_limits<float>::epsilon()) {
                 return false;
             }
             if (std::abs(d.x * c.y - d.y * c.x) > e.x * ad.y + e.y * ad.x + std::numeric_limits<float>::epsilon()) {
                 return false;
             }
             return true;
        }

    };

    constexpr Vec3f midpoint(const Vec3f& a, const Vec3f& b) noexcept {
        Vec3f r(a); (r+=b)*=0.5f; return r;
    }

    /**
     * Check if one of three vertices are in triangle using barycentric coordinates computation.
     * @param a first triangle vertex
     * @param b second triangle vertex
     * @param c third triangle vertex
     * @param p1 the vertex in question
     * @param p2 the vertex in question
     * @param p3 the vertex in question
     * @return true if p1 or p2 or p3 is in triangle (a, b, c), false otherwise.
     */
    constexpr bool isInTriangle3(const Vec3f& a, const Vec3f& b, const Vec3f& c,
                                 const Vec3f& p1, const Vec3f& p2, const Vec3f& p3) noexcept
    {
        // Compute vectors
        Vec3f ac = c - a; // v0
        Vec3f ab = b - a; // v1

        // Compute dot products
        const float dotAC_AC = ac.dot(ac);
        const float dotAC_AB = ac.dot(ab);
        const float dotAB_AB = ab.dot(ab);

        // Compute barycentric coordinates
        const float invDenom = 1 / (dotAC_AC * dotAB_AB - dotAC_AB * dotAC_AB);
        {
            Vec3f ap = p1 - a; // v2
            const float dotAC_AP1 = ac.dot(ap);
            const float dotAB_AP1 = ab.dot(ap);
            const float u = (dotAB_AB * dotAC_AP1 - dotAC_AB * dotAB_AP1) * invDenom;
            const float v = (dotAC_AC * dotAB_AP1 - dotAC_AB * dotAC_AP1) * invDenom;

            // Check if point is in triangle
            if ( (u >= 0) && (v >= 0) && (u + v < 1) ) {
                return true;
            }
        }

        {
            Vec3f ap = p2 - a; // v2
            const float dotAC_AP2 = ac.dot(ap);
            const float dotAB_AP2 = ab.dot(ap);
            const float u = (dotAB_AB * dotAC_AP2 - dotAC_AB * dotAB_AP2) * invDenom;
            const float v = (dotAC_AC * dotAB_AP2 - dotAC_AB * dotAC_AP2) * invDenom;

            // Check if point is in triangle
            if ( (u >= 0) && (v >= 0) && (u + v < 1) ) {
                return true;
            }
        }

        {
            Vec3f ap = p3 - a; // v3
            const float dotAC_AP3 = ac.dot(ap);
            const float dotAB_AP3 = ab.dot(ap);
            const float u = (dotAB_AB * dotAC_AP3 - dotAC_AB * dotAB_AP3) * invDenom;
            const float v = (dotAC_AC * dotAB_AP3 - dotAC_AB * dotAC_AP3) * invDenom;

            // Check if point is in triangle
            if ( (u >= 0) && (v >= 0) && (u + v < 1) ) {
                return true;
            }
        }
        return false;
    }

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_MATH_GEOM_GEOM3F_HPP_ */
