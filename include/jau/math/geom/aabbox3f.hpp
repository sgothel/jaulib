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
#ifndef JAU_MATH_GEOM_AABBOX3F_HPP_
#define JAU_MATH_GEOM_AABBOX3F_HPP_

#include <jau/functional.hpp>
#include <jau/math/vec3f.hpp>

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
    class AABBox3f {
        private:
            /** bottom left (low) */
            Point3f m_lo;
            /** top right (high) */
            Point3f m_hi;
            /** center */
            Point3f m_center;

        public:
            /**
             * Create an Axis Aligned bounding box (aabbox3f)
             * where the low and and high MAX float Values.
             */
            AABBox3f() noexcept {
                reset();
            }

            /**
             * Create an aabbox3f with given bl (low) and tr (high)
             */
            AABBox3f(const Point3f& bl_, const Point3f& tr_) noexcept
            : m_lo( bl_ ), m_hi( tr_ ) {
            }

            constexpr AABBox3f(const AABBox3f& o) noexcept = default;
            constexpr AABBox3f(AABBox3f&& o) noexcept = default;
            AABBox3f& operator=(const AABBox3f&) noexcept = default;
            AABBox3f& operator=(AABBox3f&&) noexcept = default;

        private:
            void setHigh(const float hx, const float hy, const float hz) noexcept {
                m_hi.set(hx, hy, hz);
            }

            void setLow(const float lx, const float ly, const float lz) noexcept {
                m_lo.set(lx, ly, lz);
            }

            void computeCenter() noexcept {
                ( ( m_center = m_hi ) += m_lo ) *= 0.5f;
            }

        public:
            /**
             * Reset this box to the inverse low/high, allowing the next {@link #resize(float, float, float)} command to hit.
             * @return this aabbox3f for chaining
             */
            AABBox3f& reset() noexcept {
                setLow(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
                setHigh(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
                m_center.set(0, 0, 0);
                return *this;
            }

            /** Returns the maximum right-top-near (xyz) coordinate */
            const Point3f& high() const noexcept { return m_hi; }

            /** Returns the minimum left-bottom-far (xyz) coordinate */
            const Point3f& low() const noexcept { return m_lo; }

            /** Returns computed center of this aabbox3f of low() and high(). */
            const Point3f& center() const noexcept { return m_center; }

            /**
             * Get the size of this aabbox3f where the size is represented by the
             * length of the vector between low and high.
             * @return a float representing the size of the aabbox3f
             */
            float size() const noexcept { return m_lo.dist(m_hi); }

            float width() const noexcept { return m_hi.x - m_lo.x; }

            float height() const noexcept { return m_hi.y - m_lo.y; }

            float depth() const noexcept { return m_hi.z - m_lo.z; }

            /** Returns the volume, i.e. width * height * depth */
            float volume() const noexcept { return width() * height() * depth(); }

            /** Return true if {@link #getVolume()} is {@link FloatUtil#isZero(float)}, considering epsilon. */
            bool hasZeroVolume() const noexcept { return jau::is_zero( volume() ); }

            /** Returns the assumed 2D area, i.e. width * height while assuming low and high lies on same plane. */
            float area2D() const noexcept { return width() * height(); }

            /** Return true if {@link #get2DArea()} is {@link FloatUtil#isZero(float)}, considering epsilon. */
            bool hasZeroArea2D() const noexcept { return jau::is_zero( area2D() ); }

            /**
             * Set size of the aabbox3f specifying the coordinates
             * of the low and high.
             *
             * @param low min xyz-coordinates
             * @param high max xyz-coordinates
             * @return this aabbox3f for chaining
             */
            AABBox3f& setSize(const float low[], const float high[]) noexcept {
                return setSize(low[0],low[1],low[2], high[0],high[1],high[2]);
            }

            /**
             * Set size of the aabbox3f specifying the coordinates
             * of the low and high.
             *
             * @param lx min x-coordinate
             * @param ly min y-coordnate
             * @param lz min z-coordinate
             * @param hx max x-coordinate
             * @param hy max y-coordinate
             * @param hz max z-coordinate
             * @return this aabbox3f for chaining
             */
            AABBox3f& setSize(const float lx, const float ly, const float lz,
                              const float hx, const float hy, const float hz) noexcept {
                m_lo.set(lx, ly, lz);
                m_hi.set(hx, hy, hz);
                computeCenter();
                return *this;
            }

            /**
             * Set size of the aabbox3f specifying the coordinates
             * of the low and high.
             *
             * @param low min xyz-coordinates
             * @param high max xyz-coordinates
             * @return this aabbox3f for chaining
             */
            AABBox3f& setSize(const Vec3f& low, const Vec3f& high) noexcept {
                m_lo = low;
                m_hi = high;
                computeCenter();
                return *this;
            }

            /**
             * Resize width of this aabbox3f with explicit left- and right delta values
             * @param deltaLeft positive value will expand width, otherwise shrink width
             * @param deltaRight positive value will expand width, otherwise shrink width
             * @return this aabbox3f for chaining
             */
            AABBox3f& resizeWidth(const float deltaLeft, const float deltaRight) noexcept {
                bool mod = false;
                if( !jau::is_zero(deltaLeft) ) {
                    m_lo.x -= deltaLeft;
                    mod = true;
                }
                if( !jau::is_zero(deltaRight) ) {
                    m_hi.x += deltaRight;
                    mod = true;
                }
                if( mod ) {
                    computeCenter();
                }
                return *this;
            }

            /**
             * Resize height of this aabbox3f with explicit bottom- and top delta values
             * @param deltaBottom positive value will expand height, otherwise shrink height
             * @param deltaTop positive value will expand height, otherwise shrink height
             * @return this aabbox3f for chaining
             */
            AABBox3f& resizeHeight(const float deltaBottom, const float deltaTop) noexcept {
                bool mod = false;
                if( !jau::is_zero(deltaBottom) ) {
                    m_lo.y -= deltaBottom;
                    mod = true;
                }
                if( !jau::is_zero(deltaTop) ) {
                    m_lo.y += deltaTop;
                    mod = true;
                }
                if( mod ) {
                    computeCenter();
                }
                return *this;
            }

            /**
             * Resize the aabbox3f to encapsulate another AABox
             * @param newBox aabbox3f to be encapsulated in
             * @return this aabbox3f for chaining
             */
            AABBox3f& resize(const AABBox3f& o) noexcept {
                /** test bl (low) */
                if (o.m_lo.x < m_lo.x) {
                    m_lo.x = o.m_lo.x;
                }
                if (o.m_lo.y < m_lo.y) {
                    m_lo.y = o.m_lo.y;
                }
                if (o.m_lo.z < m_lo.z) {
                    m_lo.z = o.m_lo.z;
                }

                /** test tr (high) */
                if (o.m_hi.x > m_hi.x) {
                    m_hi.x = o.m_hi.x;
                }
                if (o.m_hi.y > m_hi.y) {
                    m_hi.y = o.m_hi.y;
                }
                if (o.m_hi.z > m_hi.z) {
                    m_hi.z = o.m_hi.z;
                }
                computeCenter();
                return *this;
            }

            /**
             * General purpose Vec3f transform function
             */
            typedef jau::function<jau::math::Vec3f(const jau::math::Vec3f&)> transform_vec3f_func;

            /**
             * Resize the aabbox3f to encapsulate another AABox, which will be <i>transformed</i> on the fly first.
             * @param newBox aabbox3f to be encapsulated in
             * @param transform the transform function, applied on <i>newBox</i> on the fly
             * @param tmpV3 temporary storage
             * @return this aabbox3f for chaining
             */
            AABBox3f& resize(const AABBox3f& newBox, transform_vec3f_func& transform) noexcept {
                /** test low */
                {
                    const jau::math::Vec3f newBL = transform(newBox.low());
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
                    const jau::math::Vec3f newTR = transform(newBox.high());
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

            /**
             * Resize the aabbox3f to encapsulate the passed
             * xyz-coordinates.
             * @param x x-axis coordinate value
             * @param y y-axis coordinate value
             * @param z z-axis coordinate value
             * @return this aabbox3f for chaining
             */
            AABBox3f& resize(const float x, const float y, const float z) noexcept {
                /** test low */
                if (x < m_lo.x) {
                    m_lo.x = x;
                }
                if (y < m_lo.y) {
                    m_lo.y = y;
                }
                if (z < m_lo.z) {
                    m_lo.z = z;
                }

                /** test high */
                if (x > m_hi.x) {
                    m_hi.x = x;
                }
                if (y > m_hi.y) {
                    m_hi.y = y;
                }
                if (z > m_hi.z) {
                    m_hi.z = z;
                }
                computeCenter();
                return *this;
            }

            /**
             * Resize the aabbox3f to encapsulate the passed
             * xyz-coordinates.
             * @param xyz xyz-axis coordinate values
             * @return this aabbox3f for chaining
             */
            AABBox3f& resize(const float xyz[]) noexcept {
                return resize(xyz[0], xyz[1], xyz[2]);
            }

            /**
             * Resize the aabbox3f to encapsulate the passed
             * xyz-coordinates.
             * @param xyz xyz-axis coordinate values
             * @return this aabbox3f for chaining
             */
            AABBox3f& resize(const Point3f& p) noexcept {
                return resize(p.x, p.y, p.z);
            }

            /**
             * Check if the 2D point is bounded/contained by this aabbox3f
             * @return true if {x, y} belongs to {low, high}
             */
            bool contains(const float x, const float y) const noexcept {
                return !( x<m_lo.x || x>m_hi.x ||
                          y<m_lo.y || y>m_hi.y );
            }

            /**
             * Check if the 2D point is bounded/contained by this aabbox3f
             * @return true if p belongs to {low, high}
             */
            bool contains(const Point2f& p) const noexcept { return contains(p.x, p.y); }

            /**
             * Check if the 3D point is bounded/contained by this aabbox3f
             * @return true if {x, y, z} belongs to {low, high}
             */
            bool contains(const float x, const float y, const float z) const noexcept {
                return m_lo.x<=x && x<=m_hi.x &&
                       m_lo.y<=y && y<=m_hi.y &&
                       m_lo.z<=z && z<=m_hi.z;
            }

            /**
             * Check if the 3D point is bounded/contained by this aabbox3f
             * @return true if p belongs to (low.x, high.x) and y belong to (low.y, high.y)
             */
            bool contains(const Point3f& p) const noexcept { return contains(p.x, p.y, p.z); }

            /** Returns whether this aabbox3f intersects (partially contains) given aabbox3f. */
            bool intersects(const AABBox3f& o) const noexcept {
                /**
                 * Traditional boolean equation leads to multiple branches,
                 * using max/min approach allowing for branch-less optimizations.
                 *
                    return !( m_hi.x < o.m_lo.x ||
                              m_hi.y < o.m_lo.y ||
                              m_hi.z < o.m_lo.z ||
                              m_lo.x > o.m_hi.x ||
                              m_lo.y > o.m_hi.y ||
                              m_lo.z > o.m_hi.z );
                */
                const Point3f lo = max(m_lo, o.m_lo);
                const Point3f hi = min(m_hi, o.m_hi);
                return lo.x <= hi.x && lo.y <= hi.y && lo.z <= hi.z;
            }

            /** Returns whether this aabbox3f fully contains given aabbox3f. */
            bool contains(const AABBox3f& o) const noexcept {
                return m_hi.x >= o.m_hi.x &&
                       m_hi.y >= o.m_hi.y &&
                       m_hi.z >= o.m_hi.z &&
                       m_lo.x <= o.m_lo.x &&
                       m_lo.y <= o.m_lo.y &&
                       m_lo.z <= o.m_lo.z;
            }

            /**
             * Check if there is a common region between this AABBox and the passed
             * 2D region irrespective of z range
             * @param x lower left x-coord
             * @param y lower left y-coord
             * @param w width
             * @param h hight
             * @return true if this AABBox might have a common region with this 2D region
             */
            bool intersects2DRegion(const float x, const float y, const float w, const float h) const noexcept {
                if (w <= 0 || h <= 0) {
                    return false;
                }
                const float _w = width();
                const float _h = height();
                if (_w <= 0 || _h <= 0) {
                    return false;
                }
                const float x0 = m_lo.x;
                const float y0 = m_lo.y;
                return (x >= x0 &&
                        y >= y0 &&
                        x + w <= x0 + _w &&
                        y + h <= y0 + _h);
            }

            /**
             * Check if {@link Ray} intersects this bounding box.
             * <p>
             * Versions uses the SAT[1], testing 6 axes.
             * Original code for OBBs from MAGIC.
             * Rewritten for AABBs and reorganized for early exits[2].
             * </p>
             * <pre>
             * [1] SAT = Separating Axis Theorem
             * [2] http://www.codercorner.com/RayAABB.cpp
             * </pre>
             * @param ray
             * @return
             */
            bool intersectsRay(const Ray3f ray) const noexcept {
                // diff[XYZ] -> ray.orig - center
                //  ext[XYZ] -> extend high - center

                const float dirX  = ray.dir.x;
                const float diffX = ray.orig.x - m_center.x;
                const float extX  = m_hi.x - m_center.x;
                if( std::abs(diffX) > extX && diffX*dirX >= 0.0f ) return false;

                const float dirY  = ray.dir.y;
                const float diffY = ray.orig.y - m_center.y;
                const float extY  = m_hi.y - m_center.y;
                if( std::abs(diffY) > extY && diffY*dirY >= 0.0f ) return false;

                const float dirZ  = ray.dir.z;
                const float diffZ = ray.orig.z - m_center.z;
                const float extZ  = m_hi.z - m_center.z;
                if( std::abs(diffZ) > extZ && diffZ*dirZ >= 0.0f ) return false;

                const float absDirY = std::abs(dirY);
                const float absDirZ = std::abs(dirZ);

                float f = dirY * diffZ - dirZ * diffY;
                if( std::abs(f) > extY*absDirZ + extZ*absDirY ) return false;

                const float absDirX = std::abs(dirX);

                f = dirZ * diffX - dirX * diffZ;
                if( std::abs(f) > extX*absDirZ + extZ*absDirX ) return false;

                f = dirX * diffY - dirY * diffX;
                if( std::abs(f) > extX*absDirY + extY*absDirX ) return false;

                return true;
            }

            /**
             * Return intersection of a {@link Ray} with this bounding box,
             * or false if none exist.
             * <p>
             * <ul>
             *  <li>Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990 [2]</li>
             *  <li>Optimized code by Pierre Terdiman, 2000 (~20-30% faster on my Celeron 500)</li>
             *  <li>Epsilon value added by Klaus Hartmann.</li>
             * </ul>
             * </p>
             * <p>
             * Method is based on the requirements:
             * <ul>
             *  <li>the integer representation of 0.0f is 0x00000000</li>
             *  <li>the sign bit of the float is the most significant one</li>
             * </ul>
             * </p>
             * <p>
             * Report bugs: p.terdiman@codercorner.com (original author)
             * </p>
             * <pre>
             * [1] http://www.codercorner.com/RayAABB.cpp
             * [2] http://tog.acm.org/resources/GraphicsGems/gems/RayBox.c
             * </pre>
             * @param result vec3
             * @param ray
             * @param epsilon
             * @param assumeIntersection if true, method assumes an intersection, i.e. by pre-checking via {@link #intersectsRay(Ray)}.
             *                           In this case method will not validate a possible non-intersection and just computes
             *                           coordinates.
             * @return true with having intersection coordinates stored in result, or false if none exists
             */
            bool getRayIntersection(Vec3f& result, const Ray3f& ray, const float epsilon,
                                    const bool assumeIntersection) {
                float maxT[] = { -1.0f, -1.0f, -1.0f };

                const Vec3f& origin = ray.orig;
                const Vec3f& dir = ray.dir;

                bool inside = true;

                /**
                 * Use unrolled version below...
                 *
                 * Find candidate planes.
                    for(int i=0; i<3; i++) {
                        const float origin_i = origin.get(i);
                        const float dir_i = dir.get(i);
                        const float bl_i = bl.get(i);
                        const float tr_i = tr.get(i);
                        if(origin_i < bl_i) {
                            result.set(i, bl_i);
                            inside    = false;

                            // Calculate T distances to candidate planes
                            if( 0 != jau::bit_value(dir_i) ) {
                                maxT[i] = (bl_i - origin_i) / dir_i;
                            }
                        } else if(origin_i > tr_i) {
                            result.set(i, tr_i);
                            inside    = false;

                            // Calculate T distances to candidate planes
                            if( 0 != jau::bit_value(dir_i) ) {
                                maxT[i] = (tr_i - origin_i) / dir_i;
                            }
                        }
                    }
                */
                // Find candidate planes, unrolled
                {
                    if(origin.x < m_lo.x) {
                        result.x = m_lo.x;
                        inside    = false;

                        // Calculate T distances to candidate planes
                        if( 0 != jau::bit_value(dir.x) ) {
                            maxT[0] = (m_lo.x - origin.x) / dir.x;
                        }
                    } else if(origin.x > m_hi.x) {
                        result.x = m_hi.x;
                        inside    = false;

                        // Calculate T distances to candidate planes
                        if( 0 != jau::bit_value(dir.x) ) {
                            maxT[0] = (m_hi.x - origin.x) / dir.x;
                        }
                    }
                }
                {
                    if(origin.y < m_lo.y) {
                        result.y = m_lo.y;
                        inside    = false;

                        // Calculate T distances to candidate planes
                        if( 0 != jau::bit_value(dir.y) ) {
                            maxT[1] = (m_lo.y - origin.y) / dir.y;
                        }
                    } else if(origin.y > m_hi.y) {
                        result.y = m_hi.y;
                        inside    = false;

                        // Calculate T distances to candidate planes
                        if( 0 != jau::bit_value(dir.y) ) {
                            maxT[1] = (m_hi.y - origin.y) / dir.y;
                        }
                    }
                }
                {
                    if(origin.z < m_lo.z) {
                        result.z = m_lo.z;
                        inside    = false;

                        // Calculate T distances to candidate planes
                        if( 0 != jau::bit_value(dir.z) ) {
                            maxT[2] = (m_lo.z - origin.z) / dir.z;
                        }
                    } else if(origin.z > m_hi.z) {
                        result.z = m_hi.z;
                        inside    = false;

                        // Calculate T distances to candidate planes
                        if( 0 != jau::bit_value(dir.z) ) {
                            maxT[2] = (m_hi.z - origin.z) / dir.z;
                        }
                    }
                }

                // Ray origin inside bounding box
                if(inside) {
                    result = origin;
                    return true;
                }

                // Get largest of the maxT's for final choice of intersection
                int whichPlane = 0;
                if(maxT[1] > maxT[whichPlane]) { whichPlane = 1; }
                if(maxT[2] > maxT[whichPlane]) { whichPlane = 2; }

                if( !assumeIntersection ) {
                    // Check final candidate actually inside box
                    if( 0 != ( jau::bit_value(maxT[whichPlane]) & jau::float_iec559_sign_bit ) ) {
                        return false;
                    }

                    /** Use unrolled version below ..
                    for(int i=0; i<3; i++) {
                        if( i!=whichPlane ) {
                            result[i] = origin[i] + maxT[whichPlane] * dir[i];
                            if(result[i] < minB[i] - epsilon || result[i] > maxB[i] + epsilon) { return false; }
                            // if(result[i] < minB[i] || result[i] > maxB[i] ) { return false; }
                        }
                    } */
                    switch( whichPlane ) {
                        case 0:
                            result.y = origin.y + maxT[whichPlane] * dir.y;
                            if(result.y < m_lo.y - epsilon || result.y > m_hi.y + epsilon) { return false; }
                            result.z = origin.z + maxT[whichPlane] * dir.z;
                            if(result.z < m_lo.z - epsilon || result.z > m_hi.z + epsilon) { return false; }
                            break;
                        case 1:
                            result.x = origin.x + maxT[whichPlane] * dir.x;
                            if(result.x < m_lo.x - epsilon || result.x > m_hi.x + epsilon) { return false; }
                            result.z = origin.z + maxT[whichPlane] * dir.z;
                            if(result.z < m_lo.z - epsilon || result.z > m_hi.z + epsilon) { return false; }
                            break;
                        case 2:
                            result.x = origin.x + maxT[whichPlane] * dir.x;
                            if(result.x < m_lo.x - epsilon || result.x > m_hi.x + epsilon) { return false; }
                            result.y = origin.y + maxT[whichPlane] * dir.y;
                            if(result.y < m_lo.y - epsilon || result.y > m_hi.y + epsilon) { return false; }
                            break;
                        default:
                            throw InternalError("XXX", E_FILE_LINE);
                    }
                } else {
                    switch( whichPlane ) {
                        case 0:
                            result.y = origin.y + maxT[whichPlane] * dir.y;
                            result.z = origin.z + maxT[whichPlane] * dir.z;
                            break;
                        case 1:
                            result.x = origin.x + maxT[whichPlane] * dir.x;
                            result.z = origin.z + maxT[whichPlane] * dir.z;
                            break;
                        case 2:
                            result.x = origin.x + maxT[whichPlane] * dir.x;
                            result.y = origin.y + maxT[whichPlane] * dir.y;
                            break;
                        default:
                            throw InternalError("XXX", E_FILE_LINE);
                    }
                }
                return true; // ray hits box
            }

            std::string toString() const noexcept {
                return "aabb[bl " + m_lo.toString() +
                       ", tr " + m_hi.toString() +
                       "]"; }
    };

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_MATH_GEOM_AABBOX3F_HPP_ */
