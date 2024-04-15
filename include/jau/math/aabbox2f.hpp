/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2024 Gothel Software e.K.
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
#ifndef JAU_AABBOX2F_HPP_
#define JAU_AABBOX2F_HPP_

#include <jau/math/vec2f.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * Axis Aligned Bounding Box. Defined by two 3D coordinates (low and high)
     * The low being the the lower left corner of the box, and the high being the upper
     * right corner of the box.
     *
     * A few references for collision detection, intersections:
     * - http://www.realtimerendering.com/intersections.html
     * - http://www.codercorner.com/RayAABB.cpp
     * - http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter0.htm
     * - http://realtimecollisiondetection.net/files/levine_swept_sat.txt
     */
    class AABBox2f {
    public:
        /** bottom left (low) */
        Point2f bl;
        /** top right (high) */
        Point2f tr;

        /**
         * Create an Axis Aligned bounding box (AABBox)
         * where the low and and high MAX float Values.
         */
        AABBox2f() noexcept {
            reset();
        }

        /**
         * Create an AABBox with given bl (low) and tr (high)
         */
        AABBox2f(const Point2f& bl_, const Point2f& tr_) noexcept
        : bl( bl_ ), tr( tr_ ) {
        }

        constexpr AABBox2f(const AABBox2f& o) noexcept = default;
        constexpr AABBox2f(AABBox2f&& o) noexcept = default;
        AABBox2f& operator=(const AABBox2f&) noexcept = default;
        AABBox2f& operator=(AABBox2f&&) noexcept = default;

        /**
         * Reset this box to the inverse low/high, allowing the next {@link #resize(float, float, float)} command to hit.
         * @return this AABBox for chaining
         */
        AABBox2f& reset() noexcept {
            bl.x = std::numeric_limits<float>::max();
            bl.y = std::numeric_limits<float>::max();
            tr.x = -std::numeric_limits<float>::max();
            tr.y = -std::numeric_limits<float>::max();
            return *this;
        }

        /**
         * Resize the AABBox to encapsulate another AABox
         * @param newBox AABBox to be encapsulated in
         * @return this AABBox for chaining
         */
        AABBox2f& resize(const AABBox2f& o) noexcept {
            /** test bl (low) */
            if (o.bl.x < bl.x) {
                bl.x = o.bl.x;
            }
            if (o.bl.y < bl.y) {
                bl.y = o.bl.y;
            }

            /** test tr (high) */
            if (o.tr.x > tr.x) {
                tr.x = o.tr.x;
            }
            if (o.tr.y > tr.y) {
                tr.y = o.tr.y;
            }

            return *this;
        }

        /**
         * Resize the AABBox to encapsulate the passed coordinates.
         * @param x x-axis coordinate value
         * @param y y-axis coordinate value
         * @return this AABBox for chaining
         */
        AABBox2f& resize(float x, float y) noexcept {
            /** test bl (low) */
            if (x < bl.x) {
                bl.x = x;
            }
            if (y < bl.y) {
                bl.y = y;
            }

            /** test tr (high) */
            if (x > tr.x) {
                tr.x = x;
            }
            if (y > tr.y) {
                tr.y = y;
            }

            return *this;
        }

        /**
         * Resize the AABBox to encapsulate the passed point.
         * @param x x-axis coordinate value
         * @param y y-axis coordinate value
         * @return this AABBox for chaining
         */
        AABBox2f& resize(const Point2f& p) noexcept {
            return resize(p.x, p.y);
        }

        AABBox2f box() const noexcept { return *this; }

        /**
         * Check if the point is bounded/contained by this AABBox
         * @return true if {x, y} belongs to (low.x, high.x) and y belong to (low.y, high.y)
         */
        bool contains(const float x, const float y) const noexcept {
            return !( x<bl.x || x>tr.x ||
                      y<bl.y || y>tr.y );
        }

        /**
         * Check if the point is bounded/contained by this AABBox
         * @return true if p belongs to (low.x, high.x) and y belong to (low.y, high.y)
         */
        bool contains(const Point2f& p) const noexcept { return contains(p.x, p.y); }

        bool intersects(const AABBox2f& o) const noexcept {
            return !( tr.x < o.bl.x ||
                      tr.y < o.bl.y ||
                      bl.x > o.tr.x ||
                      bl.y > o.tr.y );
        }

#if 0
        bool intersection(aabbox_t a, aabbox_t b){
            if(aabbox_t::intersects(a)){
                return false;
            }

        }
#endif

        std::string toString() const noexcept {
            return "aabb[bl " + bl.toString() +
                    ", tr " + tr.toString() +
                    "]"; }
    };

    /**@}*/

} // namespace jau::math

#endif /*  JAU_AABBOX2F_HPP_ */
