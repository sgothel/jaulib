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
#ifndef JAU_MATH_GEOM_GEOM3F2D_HPP_
#define JAU_MATH_GEOM_GEOM3F2D_HPP_

#include <limits>
#include "jau/int_types.hpp"
#include <jau/darray.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/geom/geom.hpp>
#include <jau/math/geom/aabbox3f.hpp>

namespace jau::math::geom {

    /** \addtogroup Math
     *
     *  @{
     */

    //
    // 2D geom using 3D data, ignoring Z and avoiding expensive polymorphism
    //

    typedef jau::darray<Vec3f, jau::nsize_t> Vec3fList;

    /**
     * Computes the area of a list of vertices via shoelace formula.
     *
     * This method is utilized e.g. to reliably compute the {@link Winding} of complex shapes.
     *
     * Implementation uses double precision.
     *
     * @param vertices
     * @return positive area if ccw else negative area value
     * @see #getWinding()
     */
    constexpr double area2D(const Vec3fList& vertices) noexcept {
        size_t n = vertices.size();
        double area = 0.0;
        for (size_t p = n - 1, q = 0; q < n; p = q++) {
            const Vec3f& pCoord = vertices[p];
            const Vec3f& qCoord = vertices[q];
            area += (double)pCoord.x * (double)qCoord.y - (double)qCoord.x * (double)pCoord.y;
        }
        return area;
    }

    /**
     * Compute the winding using the area2D() function over all vertices for complex shapes.
     *
     * Uses the {@link #area(List)} function over all points
     * on complex shapes for a reliable result!
     *
     * Implementation uses double precision.
     *
     * @param vertices array of Vertices
     * @return Winding::CCW or Winding::CLW
     * @see area2D()
     */
    constexpr Winding getArea2DWinding(const Vec3fList& vertices) noexcept {
        return area2D(vertices) >= 0 ? Winding::CCW : Winding::CW ;
    }


    constexpr double sqlend(double x, double y) noexcept {
        return x*x + y*y;
    }
    constexpr double triArea(double ax, double ay, double bx, double by, double cx, double cy) noexcept {
        return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    }
    constexpr double triArea(float ax, float ay, float bx, float by, float cx, float cy) noexcept {
        return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    }
    /**
     * Computes oriented double area of a triangle,
     * i.e. the 2x2 determinant with b-a and c-a per column.
     * <pre>
     *       | bx-ax, cx-ax |
     * det = | by-ay, cy-ay |
     * </pre>
     * @param a first vertex
     * @param b second vertex
     * @param c third vertex
     * @return area > 0 CCW, ..
     */
    constexpr double triArea2D(const Vec3f& a, const Vec3f& b, const Vec3f& c) noexcept {
        return triArea(a.x, a.y, b.x, b.y, c.x, c.y);
    }
    constexpr double inCircle2DVal(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d) noexcept {
        // Operation costs:
        // - 4x (triAreaVec2: 5+, 2*) -> 20+, 8*
        // - plus 7+, 12*             -> 27+, 20*
        return sqlend(a.x, a.y) * triArea2D(b, c, d) -
               sqlend(b.x, b.y) * triArea2D(a, c, d) +
               sqlend(c.x, c.y) * triArea2D(a, b, d) -
               sqlend(d.x, d.y) * triArea2D(a, b, c);
    }
    /**
     * Check if vertices in triangle circumcircle given {@code d} vertex, from paper by Guibas and Stolfi (1985).
     * <p>
     * Implementation uses double precision.
     * </p>
     * @param a triangle vertex 1
     * @param b triangle vertex 2
     * @param c triangle vertex 3
     * @param d vertex in question
     * @return true if the vertex d is inside the circle defined by the vertices a, b, c.
     */
    constexpr bool isInCircle2D(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d) noexcept {
        return inCircle2DVal(a, b, c, d) > std::numeric_limits<double>::epsilon();
    }

    /**
     * Check if points are in ccw order
     * <p>
     * Consider using {@link #getWinding(List)} using the {@link #area(List)} function over all points
     * on complex shapes for a reliable result!
     * </p>
     * @param a first vertex
     * @param b second vertex
     * @param c third vertex
     * @return true if the points a,b,c are in a ccw order
     * @see #getWinding(List)
     */
    constexpr bool is2DCCW(const Vec3f& a, const Vec3f& b, const Vec3f& c) noexcept {
        return triArea2D(a,b,c) > std::numeric_limits<double>::epsilon();
    }

    /**
     * Compute the winding of the 3 given points
     * <p>
     * Consider using {@link #getWinding(List)} using the {@link #area(List)} function over all points
     * on complex shapes for a reliable result!
     * </p>
     * @param a first vertex
     * @param b second vertex
     * @param c third vertex
     * @return {@link Winding#CCW} or {@link Winding#CW}
     * @see #getWinding(List)
     */
    constexpr Winding get2DWinding(const Vec3f& a, const Vec3f& b, const Vec3f& c) noexcept {
        return is2DCCW(a,b,c) ? Winding::CCW : Winding::CW ;
    }

    /**
     * 2D line segment intersection test w/o considering collinear-case
     * <p>
     * See [p + t r = q + u s](https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282)
     * and [its terse C# implementation](https://www.codeproject.com/tips/862988/find-the-intersection-point-of-two-line-segments)
     * </p>
     * <p>
     * Implementation uses float precision.
     * </p>
     * @param p vertex 1 of first segment
     * @param p2 vertex 2 of first segment
     * @param q vertex 1 of second segment
     * @param q2 vertex 2 of second segment
     * @return true if line segments are intersecting, otherwise false
     */
    constexpr bool testSeg2SegIntersection2D(const Vec3f& p, const Vec3f& p2, const Vec3f& q, const Vec3f& q2) noexcept
    {
        // Operations: 11+, 8*, 2 branches
        const float rx = p2.x - p.x; // p2.minus(p)
        const float ry = p2.y - p.y;
        const float sx = q2.x - q.x; // q2.minus(q)
        const float sy = q2.y - q.y;
        const float rxs = rx * sy - ry * sx; // r.cross(s)

        constexpr float eps = std::numeric_limits<float>::epsilon();

        if ( jau::is_zero(rxs, eps) ) {
            // Not considering collinear case as an intersection
            return false;
        } else {
            // r x s != 0
            const float q_px = q.x - p.x; // q.minus(p)
            const float q_py = q.y - p.y;
            const float qpxr = q_px * ry - q_py * rx; // q_p.cross(r)

            // p + t r = q + u s
            // (p + t r) × s = (q + u s) × s
            // t (r × s) = (q − p) × s, with s x s = 0
            // t = (q - p) x s / (r x s)
            const float t = ( q_px * sy - q_py * sx ) / rxs; // q_p.cross(s) / rxs

            // u = (p − q) × r / (s × r) = (q - p) x r / (r x s), with s × r = − r × s
            const float u = qpxr / rxs;

            // if ( (0 <= t && t <= 1) && (0 <= u && u <= 1) )
            if ( (eps <= t && t - 1 <= eps) && (eps <= u && u - 1 <= eps) )
            {
                // 3) r × s ≠ 0 and 0 ≤ t ≤ 1 and 0 ≤ u ≤ 1, the two line segments meet at the point p + t * r = q + u * s.
                return true;
            }
        }
        return false;
    }

    /**
     * Check if a segment intersects with a triangle using {@link FloatUtil#EPSILON} w/o considering collinear-case
     * <p>
     * Implementation uses float precision.
     * </p>
     * @param a vertex 1 of the triangle
     * @param b vertex 2 of the triangle
     * @param c vertex 3 of the triangle
     * @param d vertex 1 of first segment
     * @param e vertex 2 of first segment
     * @return true if the segment intersects at least one segment of the triangle, false otherwise
     * @see #testSeg2SegIntersection(Vert2fImmutable, Vert2fImmutable, Vert2fImmutable, Vert2fImmutable, float, boolean)
     */
    constexpr bool testTri2SegIntersection2D(const Vec3f& a, const Vec3f& b, const Vec3f& c,
                                             const Vec3f& d, const Vec3f& e) {
        return testSeg2SegIntersection2D(a, b, d, e) ||
               testSeg2SegIntersection2D(b, c, d, e) ||
               testSeg2SegIntersection2D(a, c, d, e) ;
    }

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_MATH_GEOM_GEOM3F2D_HPP_ */
