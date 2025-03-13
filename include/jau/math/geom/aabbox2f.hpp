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
#ifndef JAU_MATH_GEOM_AABBOX2F_HPP_
#define JAU_MATH_GEOM_AABBOX2F_HPP_

#include <jau/math/vec2f.hpp>

namespace jau::math::geom {

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
        constexpr AABBox2f() noexcept {
            reset();
        }

        /**
         * Create an AABBox with given bl (low) and tr (high)
         */
        constexpr AABBox2f(const Point2f& bl_, const Point2f& tr_) noexcept
        : bl( bl_ ), tr( tr_ ) {
        }

        constexpr AABBox2f(const AABBox2f& o) noexcept = default;
        constexpr AABBox2f(AABBox2f&& o) noexcept = default;
        constexpr AABBox2f& operator=(const AABBox2f&) noexcept = default;
        constexpr AABBox2f& operator=(AABBox2f&&) noexcept = default;

        /**
         * Reset this box to the inverse low/high, allowing the next {@link #resize(float, float, float)} command to hit.
         * @return this AABBox for chaining
         */
        constexpr AABBox2f& reset() noexcept {
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

        constexpr AABBox2f box() const noexcept { return *this; }

        /**
         * Check if the point is bounded/contained by this AABBox
         * @return true if {x, y} belongs to (low.x, high.x) and y belong to (low.y, high.y)
         */
        constexpr bool contains(const float x, const float y) const noexcept {
            return bl.x<=x && x<=tr.x &&
                   bl.y<=y && y<=tr.y;
        }

        /**
         * Check if the point is bounded/contained by this AABBox
         * @return true if p belongs to (low.x, high.x) and y belong to (low.y, high.y)
         */
        constexpr bool contains(const Point2f& p) const noexcept { return contains(p.x, p.y); }

        constexpr bool intersects(const AABBox2f& o) const noexcept {
            /**
             * Traditional boolean equation leads to multiple branches,
             * using max/min approach allowing for branch-less optimizations.
             *
                return !( tr.x < o.bl.x ||
                          tr.y < o.bl.y ||
                          bl.x > o.tr.x ||
                          bl.y > o.tr.y );
             */
            const Point2f lo = max(bl, o.bl);
            const Point2f hi = min(tr, o.tr);
            return lo.x <= hi.x && lo.y <= hi.y;
        }

#if 0
        constexpr bool intersection(aabbox_t a, aabbox_t b){
            if(aabbox_t::intersects(a)){
                return false;
            }

        }
#endif

        /** Returns whether this aabbox2f fully contains given aabbox2f. */
        constexpr bool contains(const AABBox2f& o) const noexcept {
            return tr.x >= o.tr.x &&
                   tr.y >= o.tr.y &&
                   bl.x <= o.bl.x &&
                   bl.y <= o.bl.y;
        }
        std::string toString() const noexcept {
            return "aabb[bl " + bl.toString() +
                    ", tr " + tr.toString() +
                    "]"; }
    };

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_MATH_GEOM_AABBOX2F_HPP_ */
