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

#include <jau/math/vec3f.hpp>
#include <jau/math/geom/aabbox3f.hpp>

namespace jau::math::geom {

    /** \addtogroup Math
     *
     *  @{
     */

    struct LineSeg3f {
        Point3f p0;
        Point3f p1;
        Point3f p2;

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
            const Vec3f e = (box.tr - box.bl) * 0.5f;
            const Vec3f aabb_center = (box.bl + box.tr) * 0.5f;
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

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_MATH_GEOM_GEOM3F_HPP_ */
