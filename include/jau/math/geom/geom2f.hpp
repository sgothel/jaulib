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
#ifndef JAU_GEOM2F_HPP_
#define JAU_GEOM2F_HPP_

#include <vector>
#include <memory>

#include <jau/math/vec2f.hpp>
#include <jau/math/geom/geom.hpp>
#include <jau/math/geom/aabbox2f.hpp>

namespace jau::math::geom {

    using namespace jau::math;

    /** \addtogroup Math
     *
     *  @{
     */

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
    constexpr double tri_area(const Point2f& a, const Point2f& b, const Point2f& c){
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    }

    /**
     * Return the orientation of the given point triplet a, b and c using triArea()
     */
    constexpr orientation_t orientation(const Point2f& a, const Point2f& b, const Point2f& c) noexcept {
        const double area = tri_area(a, b, c);
        if ( jau::is_zero( area ) ) {
            return orientation_t::COL;
        }
        return ( area > 0.0f ) ? orientation_t::CCW : orientation_t::CLW;
    }

    class LineSeg2f; // fwd

    /**
     * Geometric object
     */
    class Geom2f {
    public:
        virtual ~Geom2f() = default;

        virtual AABBox2f box() const noexcept = 0;
        virtual bool contains(const Point2f& o) const noexcept = 0;
        virtual bool intersects(const LineSeg2f & o) const noexcept = 0;
        virtual bool intersects(const AABBox2f& box) const noexcept = 0;
        virtual bool intersects(const Geom2f& o) const noexcept = 0;

        /**
         * Return whether this object intersects with the given line segment
         * and if intersecting, the crossing point (intersection), the normalized normal of the crossing surface and the reflection out vector.
         */
        virtual bool intersection(Vec2f& reflect_out, Vec2f& cross_normal, Point2f& cross_point, const LineSeg2f& in) const noexcept = 0;

        virtual std::string toString() const noexcept = 0;
    };
    typedef std::shared_ptr<Geom2f> Geom2f_ref;
    typedef std::vector<Geom2f_ref> Geom2f_list;

    class LineSeg2f : public Geom2f {
    public:
        Point2f p0;
        Point2f p1;

        constexpr LineSeg2f() noexcept
        : p0(), p1() {}

        constexpr LineSeg2f(const Point2f& p0_, const Point2f& p1_) noexcept
        : p0( p0_ ), p1( p1_ ) {}

        /**
         * Scale this line segment with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr LineSeg2f& operator*=(const float s ) noexcept {
            p0 *= s;
            p1 *= s;
            return *this;
        }

        /**
         * Return the length of this line segment, i.e. distance between both points.
         */
        constexpr float length() const noexcept {
            return p1.dist(p0);
        }

        /**
         * Return the angle of this line segment in radians
         */
        float angle() const noexcept {
            return (p1 - p0).angle();
        }

        /**
         * Return the angle between two line segments in radians
         */
        float angle(const LineSeg2f & o) const noexcept {
            const Vec2f a = p1 - p0;
            const Vec2f b = o.p1 - o.p0;
            return a.angle(b);
        }

        void add(float length) {
            // extend center points p0, p1 with radius in moving direction
            const float a_move = angle();
            Vec2f l_move_diff = Vec2f::from_length_angle(length, a_move);
            p0 -= l_move_diff;
            p1 += l_move_diff;
        }

        std::string toString() const noexcept override { return "L[" + p0.toString() + ", " + p1.toString() + "]"; }

        /**
         * Create an AABBox with given lineseg
         */
        AABBox2f box() const noexcept override {
            return AABBox2f().resize(p0).resize(p1);
        }

    private:
        bool is_on_line(const Point2f& p2) const noexcept {
            // Using the perp dot product (PDP),
            // which is the area of the parallelogram of the three points,
            // same as the area of the triangle defined by the three points, multiplied by 2.
            const float perpDotProduct = (p0.x - p2.x) * (p1.y - p2.y) - (p0.y - p2.y) * (p1.x - p2.x);
            return jau::is_zero( perpDotProduct );

        }
        bool is_on_line2(const Point2f& p2) const noexcept {
            if ( p2.x <= std::max(p0.x, p1.x) && p2.x >= std::min (p0.x, p1.x) &&
                    p2.y <= std::max(p0.y, p2.y) && p2.y >= std::min (p0.y, p1.y) )
            {
                return true;
            }
            return false;
        }

    public:
        /**
         * Test intersection between this line segment and the give point
         * @return true if the line segment contains the point, otherwise false
         */
        bool contains(const Point2f& p2) const noexcept override {
            if ( !( ( p0.x <= p2.x && p2.x <= p1.x ) || ( p1.x <= p2.x && p2.x <= p0.x ) ) ) {
                // not in x-range
                return false;
            }
            if ( !( ( p0.y <= p2.y && p2.y <= p1.y ) || ( p1.y <= p2.y && p2.y <= p0.y ) ) ) {
                // not in y-range
                return false;
            }
            return is_on_line(p2);
        }

    private:
        /**
         * See [p + t r = q + u s](https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282)
         * and [its terse C# implementation](https://www.codeproject.com/tips/862988/find-the-intersection-point-of-two-line-segments)
         */
        static bool intersects(Point2f& result,
                const Point2f& p, const Point2f& p2,
                const Point2f& q, const Point2f& q2, const bool do_collinear=false)
        {
            // Operations: 11+, 8*, 2 branches without collinear case
            constexpr const float eps = std::numeric_limits<float>::epsilon();
            const Vec2f r = p2 - p;
            const Vec2f s = q2 - q;
            const float rxs = r.cross(s);

            if ( jau::is_zero(rxs) ) {
                if ( do_collinear ) {
                    const Vec2f q_p = q - p;
                    const float qpxr = q_p.cross(r);
                    if ( jau::is_zero(qpxr) ) // disabled collinear case
                    {
                        // 1) r x s = 0 and (q - p) x r = 0, the two lines are collinear.

                        const Point2f p_q = p - q;
                        const float qp_dot_r = q_p.dot(r);
                        const float pq_dot_s = p_q.dot(s);
                        // if ( ( 0 <= qp_dot_r && qp_dot_r <= r.dot(r) ) ||
                        //      ( 0 <= pq_dot_s && pq_dot_s <= s.dot(s) ) )
                        if ( ( eps <= qp_dot_r && qp_dot_r - r.dot(r) <= eps ) ||
                             ( eps <= pq_dot_s && pq_dot_s - s.dot(s) <= eps ) )
                        {
                            // 1.1) 0 <= (q - p) · r <= r · r or 0 <= (p - q) · s <= s · s, the two lines are overlapping
                            // FIXME: result set to q2 endpoint, OK?
                            result = q2;
                            return true;
                        }

                        // 1.2 the two lines are collinear but disjoint.
                        return false;
                    } else {
                        // 2) r × s = 0 and (q − p) × r ≠ 0, the two lines are parallel and non-intersecting.
                        return false;
                    }
                } else {
                    // Not considering collinear case as an intersection
                    return false;
                }
            } else {
                // r x s != 0
                const Vec2f q_p = q - p;
                const float qpxr = q_p.cross(r);

                // p + t r = q + u s
                // (p + t r) × s = (q + u s) × s
                // t (r × s) = (q − p) × s, with s x s = 0
                // t = (q - p) x s / (r x s)
                const float t = q_p.cross(s) / rxs;

                // u = (p − q) × r / (s × r) = (q - p) x r / (r x s), with s × r = − r × s
                const float u = qpxr / rxs;

                // if ( (0 <= t && t <= 1) && (0 <= u && u <= 1) )
                if ( (eps <= t && t - 1 <= eps) && (eps <= u && u - 1 <= eps) )
                {
                    // 3) r × s ≠ 0 and 0 ≤ t ≤ 1 and 0 ≤ u ≤ 1, the two line segments meet at the point p + t * r = q + u * s.
                    result = p + (t * r); // == q + (u * s)
                    return true;
                }
            }

            return false;
        }

    public:

        /**
         * Compute intersection between two lines segments
         * @param result storage for the intersection coordinates if the lines intersect, otherwise unchanged
         * @param o the other line segment.
         * @return true if the line segments intersect, otherwise false
         */
        bool intersects(Point2f& result, const LineSeg2f & o) const noexcept {
            return intersects(result, p0, p1, o.p0, o.p1);
        }

        /**
         * Return true if this line segment intersect with the given line segment
         * @param o the other line segment.
         * @return true if both intersect, otherwise false
         */
        bool intersects(const LineSeg2f & o) const noexcept override {
            Point2f result;
            return intersects(result, p0, p1, o.p0, o.p1);
#if 0
            const pixel::orientation_t or_1 = orientation (p0, p1, o.p0);
            const pixel::orientation_t or_2 = orientation (p0, p1, o.p1);
            const pixel::orientation_t or_3 = orientation(o.p0, o.p1, p0);
            const pixel::orientation_t or_4 = orientation(o.p0, o.p1, p1);

            // General case : Points are non-collinear.
            if (or_1 != or_2 && or_3 != or_4) {
                pixel::log_printf("i-g-0: %s with %s\n", toString().c_str(), o.toString().c_str());
                return true;
            }

#if 0
            // Special case : Points are collinear.

            // If points p0, p1 and o.p0 are collinear, check if point o.p0 lies on this segment p0-p1
            if (or_1 == pixel::orientation_t::COL && is_on_line (o.p0)) {
                pixel::log_printf("i-s-1: %s with %s\n", toString().c_str(), o.toString().c_str());
                return true;
            }

            // If points p0, p1 and o.p1 are collinear, check if point o.p1 lies on this segment p0-p1
            if (or_2 == pixel::orientation_t::COL && is_on_line (o.p1)) {
                pixel::log_printf("i-s-2: %s with %s\n", toString().c_str(), o.toString().c_str());
                return true;
            }

            // If points o.p0, o.p1 and p0 are collinear, check if point p0 lies on given segment o.p0-o.p1
            if (or_3 == pixel::orientation_t::COL && o.is_on_line (p0)) {
                pixel::log_printf("i-s-3: %s with %s\n", toString().c_str(), o.toString().c_str());
                return true;
            }

            // If points o.p0, o.p1 and p1 are collinear, check if point p1 lies on given segment o.p0-o.p1
            if (or_4 == pixel::orientation_t::COL && o.is_on_line (p1)) {
                pixel::log_printf("i-s-4: %s with %s\n", toString().c_str(), o.toString().c_str());
                return true;
            }
#endif

            return false;
#endif
        }

        /**
         * Returns minimum distance between this line segment and given point p
         * <p>
         * See [Shortest distance between a point and a line segment](https://stackoverflow.com/a/1501725)
         * </p>
         * <p>
         * Slightly more expensive than intersects().
         * </p>
         */
        float distance(Point2f p) const noexcept {
            // Operations: 15+, 9*, 1-sqrt, 3 branches
            const float l2 = p1.dist_sq(p0); // i.e. |p1-p0|^2 -  avoid a sqrt
            if( l2 < std::numeric_limits<float>::epsilon() ) {
                return p.dist(p1);   // p1 == p0 case
            }
            // Consider the line extending the segment, parameterized as p0 + t (p1 - p0).
            // We find projection of point p onto the line.
            // It falls where t = [(p-p0) . (p1-p0)] / |p1-p0|^2
            // We clamp t from [0,1] to handle points outside the line segment.
            Vec2f pv = p - p0;
            Vec2f wv = p1 - p0;
            const float t = std::max(0.0f, std::min(1.0f, pv.dot(wv) / l2));
            const Vec2f projection = p0 + t * (p1 - p0);  // Projection falls on the segment
            return p.dist(projection);
        }

        bool intersects(const AABBox2f& box) const noexcept override {
            // separating axis theorem.
            const Vec2f d = (p1 - p0) * 0.5f; // half lineseg direction
            const Vec2f e = (box.tr - box.bl) * 0.5f;
            const Vec2f aabb_center = (box.bl + box.tr) * 0.5f;
            const Vec2f lseg_center = p0 + d;
            const Vec2f c = lseg_center - aabb_center;
            const Vec2f ad(std::abs(d.x), std::abs(d.y));
            if (std::abs(c.x) > e.x + ad.x) {
                return false;
            }
            if (std::abs(c.y) > e.y + ad.y) {
                return false;
            }
            /**
                    if (std::abs(d.y * c.z - d.z * c.y) > e.y * ad.z + e.z * ad.y + std::numeric_limits<float>::epsilon()) {
                        return false;
                    }
                    if (std::abs(d.z * c.x - d.x * c.z) > e.z * ad.x + e.x * ad.z + std::numeric_limits<float>::epsilon()) {
                        return false;
                    }
             */
            if (std::abs(d.x * c.y - d.y * c.x) > e.x * ad.y + e.y * ad.x + std::numeric_limits<float>::epsilon()) {
                return false;
            }
            return true;
        }

        bool intersects(const Geom2f& o) const noexcept override {
            return intersects(o.box());
        }

        bool intersection(Vec2f& reflect_out, Vec2f& cross_normal, Point2f& cross_point, const LineSeg2f& in) const noexcept override {
            if( intersects(cross_point, in) ) {
                cross_normal = (p1 - p0).normal_ccw().normalize();
                const Vec2f v_in = cross_point - in.p0;
                reflect_out = v_in - ( 2.0f * v_in.dot(cross_normal) * cross_normal );
                return true;
            }
            return false;
        }

        bool intersection(const AABBox2f& box, Vec2f& reflect_out, Vec2f& cross_normal, Point2f& cross_point) const noexcept {
            const Point2f tl(box.bl.x, box.tr.y);
            const Point2f br(box.tr.x, box.bl.y);
            {
                const LineSeg2f lt(tl, box.tr);
                const LineSeg2f lb(box.bl, br);
                const float dt = lt.distance( p0 );
                const float db = lb.distance( p0 );
                if( dt < db ) {
                    if( lt.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                    if( lb.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                } else {
                    if( lb.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                    if( lt.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                }
            }
            {
                const LineSeg2f lr(br, box.tr);
                const LineSeg2f ll(box.bl, tl);
                const float dr = lr.distance( p0 );
                const float dl = ll.distance( p0 );
                if( dr < dl ) {
                    if( lr.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                    if( ll.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                } else {
                    if( ll.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                    if( lr.intersection(reflect_out, cross_normal, cross_point, *this) ) {
                        return true;
                    }
                }
            }
            return false;
        }

    };


    /**
     * Animated geometric object
     * - movable
     * - rotatable
     * - time based mutation, i.e. tick()'able
     */
    class AGeom2f : public Geom2f {
    public:
        virtual void rotate(const float rad) noexcept = 0;
        virtual void move_dir(const float d) noexcept = 0;
        virtual void move(const Point2f& d) noexcept = 0;
        virtual void move(const float dx, const float dy) noexcept = 0;
        virtual bool tick(const float dt) noexcept { (void)dt; return true; }
    };
    typedef std::shared_ptr<AGeom2f> AGeom2f_ref;
    typedef std::vector<AGeom2f_ref> AGeom2f_list;

    class Disk2f : public AGeom2f {
    public:
        /**
         * Imagine a circle ;-)
         *
         *     ---------
         *    |    |r   |
         *    |    |    |
         *    |    c    |
         *    |         |
         *     ---------
         */
        /** m_center */
        Point2f center;
        float radius;
        /** direction angle in radians */
        float dir_angle;

        Disk2f(const Point2f& c_, const float r_)
        : center( c_), radius(r_), dir_angle(0.0f) {}

        Disk2f(float x, float y, const float r_)
        : center(x, y), radius(r_), dir_angle(0.0f) {}

        std::string toString() const noexcept override {
            return "disk[c " + center.toString() +
                    ", r " + std::to_string(radius) +
                    "]"; }

        void set_center(const Point2f& p) {
            center = p;
        }

        AABBox2f box() const noexcept override {
            Point2f bl = { center.x - radius, center.y - radius};
            Point2f tr = { center.x + radius, center.y + radius};
            return AABBox2f(bl, tr);
        }

        bool contains(const Point2f& o) const noexcept override {
            return center.dist(o) <= radius;
            // return box().contains(o);
        }

        bool intersects(const LineSeg2f & o) const noexcept override {
            return o.intersects(box());
        }

        bool intersects(const AABBox2f& o) const noexcept override {
            return box().intersects(o);
        }

        bool intersects(const Geom2f& o) const noexcept override {
            return box().intersects(o.box());
        }

        bool intersection(Vec2f& reflect_out, Vec2f& cross_normal, Point2f& cross_point, const LineSeg2f& in) const noexcept override {
            if( !in.intersects( box() ) ) {
                return false;
            }
            cross_point = center; // use center
            const float dx = in.p1.x - in.p0.x;
            const float dy = in.p1.y - in.p0.y;
            cross_normal = Vec2f(-dy, dx).normalize();
            const Vec2f v_in = in.p1 - in.p0;
            // reflect_out = v_in - ( 2.0f * v_in.dot(cross_normal) * cross_normal ); // TODO: check if cross_normal is OK for this case
            reflect_out = -1.0f * v_in;
            return true;
        }

        void rotate(const float rad) noexcept override {
            dir_angle += rad;
        }

        void move_dir(const float d) noexcept override {
            Point2f dir { d, 0 };
            dir.rotate(dir_angle);
            center += dir;
        }

        void move(const Point2f& d) noexcept override {
            center += d;
        }
        void move(const float dx, const float dy) noexcept override {
            center.add(dx, dy);
        }
    };
    typedef std::shared_ptr<Disk2f> Disk2f_ref;

    class Rect2f : public AGeom2f {
    public:
        /**
         * Unrotated, clockwise (CW):
         *
         *   (a)-----(b)
         *    |       |
         *    |       |
         *    |       |
         *   (c)-----(d)
         */
        /** Unrotated top-left */
        Point2f p_a;
        /** Unrotated top-right */
        Point2f p_b;
        /** Unrotated bottom-left */
        Point2f p_c;
        /** Unrotated bottom_right */
        Point2f p_d;
        Point2f p_center;
        /** direction angle in radians */
        float dir_angle;

    public:
        Rect2f(const Point2f& tl_, const float width, const float height, const float radians) noexcept
        {
            p_a = tl_;
            p_b = { p_a.x + width, p_a.y };
            p_c = { p_a.x        , p_a.y - height};
            p_d = { p_a.x + width, p_a.y - height};
            p_center = { p_a.x + width/2  , p_a.y - height/2  };
            dir_angle = 0.0f;
            rotate(radians);
        }

        Rect2f(const Point2f& tl_, const float width, const float height) noexcept{
            p_a = tl_;
            p_b = { p_a.x + width, p_a.y };
            p_c = { p_a.x        , p_a.y - height};
            p_d = { p_a.x + width, p_a.y - height};
            p_center = { p_a.x + width/2  , p_a.y - height/2  };
            dir_angle = 0.0f;
        }


        Rect2f(const Point2f& tl_, const Point2f& tr_, const Point2f& bl_, const Point2f& br_) noexcept
        : p_a(tl_), p_b(tr_), p_c(bl_), p_d(br_)
        {
            p_center = { ( p_a.x + p_b.x ) / 2.0f  , ( p_a.y + p_c.y ) / 2.0f  };
            dir_angle = 0.0f;
        }

        AABBox2f box() const noexcept override {
            return AABBox2f().resize(p_a).resize(p_b).resize(p_c).resize(p_d);
        }

        void move_dir(const float d) noexcept override {
            Point2f dir { d, 0 };
            dir.rotate(dir_angle);
            p_a += dir;
            p_b += dir;
            p_c += dir;
            p_d += dir;
            p_center += dir;
        }

        void move(const Point2f& d) noexcept override {
            p_a += d;
            p_b += d;
            p_c += d;
            p_d += d;
            p_center += d;
        }
        void move(const float dx, const float dy) noexcept override {
            p_a.add(dx, dy);
            p_b.add(dx, dy);
            p_c.add(dx, dy);
            p_d.add(dx, dy);
            p_center.add(dx, dy);
        }

        void rotate(const float radians) noexcept override {
            rotate(radians, p_center);
        }
        void rotate(const float radians, const Point2f& p) noexcept {
            const float cos = std::cos(radians);
            const float sin = std::sin(radians);
            p_a.rotate(sin, cos, p);
            p_b.rotate(sin, cos, p);
            p_c.rotate(sin, cos, p);
            p_d.rotate(sin, cos, p);
            dir_angle += radians;
        }

        void set_top_left(const Point2f& p) {
            // FIXME: Since m_p_a is unknown to be top-left ...
            const float dx = p.x - p_a.x;
            const float dy = p.y - p_a.y;
            move( dx, dy );
        }

        bool contains(const Point2f& o) const noexcept override {
            return box().contains(o);
        }

        bool intersects(const LineSeg2f & o) const noexcept override {
            return o.intersects(box());
        }

        bool intersects(const AABBox2f& o) const noexcept override {
            return box().intersects(o);
        }

        bool intersects(const Geom2f& o) const noexcept override {
            return box().intersects(o.box());
        }

        bool intersection(Vec2f& reflect_out, Vec2f& cross_normal, Point2f& cross_point, const LineSeg2f& in) const noexcept override {
            {
                // tl .. tr
                const LineSeg2f l(p_a, p_b);
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            {
                // bl .. br
                const LineSeg2f l(p_c, p_d);
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            {
                // br .. tr
                const LineSeg2f l(p_d, p_b);
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            {
                // bl .. tl
                const LineSeg2f l(p_c, p_a);
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            return false;
        }

        bool intersection(Vec2f& reflect_out, Vec2f& cross_normal, Point2f& cross_point, const LineSeg2f& in, const float in_radius) const noexcept {
            {
                // tl .. tr
                LineSeg2f l(p_a, p_b);
                const Vec2f added_size = (l.p1 - l.p0).normal_ccw().normalize() * in_radius;
                l.p0 += added_size;
                l.p1 += added_size;
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            {
                // bl .. br
                LineSeg2f l(p_c, p_d);
                const Vec2f added_size = (l.p1 - l.p0).normal_ccw().normalize() * in_radius;
                l.p0 += added_size;
                l.p1 += added_size;
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            {
                // br .. tr
                LineSeg2f l(p_d, p_b);
                const Vec2f added_size = (l.p1 - l.p0).normal_ccw().normalize() * in_radius;
                l.p0 += added_size;
                l.p1 += added_size;
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            {
                // bl .. tl
                LineSeg2f l(p_c, p_a);
                const Vec2f added_size = (l.p1 - l.p0).normal_ccw().normalize() * in_radius;
                l.p0 += added_size;
                l.p1 += added_size;
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
            }
            return false;
        }

        std::string toString() const noexcept override {
            return "rect[a " + p_a.toString() +
                    ", b " + p_b.toString() +
                    ", c " + p_c.toString() +
                    ", d " + p_d.toString() +
                    "]";
        }
    };
    typedef std::shared_ptr<Rect2f> Rect2f_ref;

    /**
     * A clockwise (CW) polyline
     */
    class LineStrip2f : public AGeom2f {
    public:
        std::vector<Point2f> p_list;
        Point2f p_center;
        /** direction angle in radians */
        float dir_angle;

    public:
        LineStrip2f() noexcept
        : p_list(), p_center(), dir_angle(0.0f) {
        }

        LineStrip2f(const Point2f& center, const float angle) noexcept
        : p_list(), p_center(center), dir_angle(angle) {
        }

        void normalize_center() noexcept {
            Point2f c;
            int n = 0;
            for(size_t i=0; i<p_list.size()-1; ++i) {
                c += p_list[i];
                n++;
            }
            // skip first == last case
            if( p_list[p_list.size()-1] != p_list[0] ) {
                c += p_list[p_list.size()-1];
                n++;
            }
            this->p_center = c / static_cast<float>(n);
        }

        AABBox2f box() const noexcept override {
            AABBox2f box;
            for(const Point2f& p : p_list) {
                box.resize(p);
            }
            return box;
        }

        void move_dir(const float d) noexcept override {
            Point2f dir { d, 0 };
            dir.rotate(dir_angle);
            for(Point2f& p : p_list) {
                p += dir;
            }
            p_center += dir;
        }

        void move(const Point2f& d) noexcept override {
            for(Point2f& p : p_list) {
                p += d;
            }
            p_center += d;
        }
        void move(const float dx, const float dy) noexcept override {
            for(Point2f& p : p_list) {
                p.add(dx, dy);
            }
            p_center.add(dx, dy);
        }

        void rotate(const float radians) noexcept override {
            const float cos = std::cos(radians);
            const float sin = std::sin(radians);
#if 0
            // pre-ranged loop
            for(size_t i=0; i<p_list.size(); ++i) {
                point2f_t& p = p_list[i];
                p.rotate(sin, cos, p_center);
            }
#endif
            for(Point2f& p : p_list) {
                p.rotate(sin, cos, p_center);
            }
            dir_angle += radians;
        }

        void set_center(const Point2f& p) {
            const float dx = p.x - p_center.x;
            const float dy = p.y - p_center.y;
            move( dx, dy );
        }

        bool contains(const Point2f& o) const noexcept override {
            return box().contains(o);
        }

        bool intersects(const LineSeg2f & o) const noexcept override {
            return o.intersects(box());
        }

        bool intersects(const AABBox2f& o) const noexcept override {
            return box().intersects(o);
        }

        bool intersects(const Geom2f& o) const noexcept override {
            return box().intersects(o.box());
        }

        bool intersects_lineonly(const LineSeg2f & o) const noexcept {
            if( p_list.size() < 2 ) {
                return false;
            }
            Point2f p0 = p_list[0];
            for(size_t i=1; i<p_list.size(); ++i) {
                const Point2f& p1 = p_list[i];
                const LineSeg2f l(p0, p1);
                if( l.intersects(o) ) {
                    return true;
                }
                p0 = p1;
            }
            return false;
        }

        bool intersection(Vec2f& reflect_out, Vec2f& cross_normal, Point2f& cross_point,
                const LineSeg2f& in) const noexcept override {
            if( p_list.size() < 2 ) {
                return false;
            }
            Point2f p0 = p_list[0];
            for(size_t i=1; i<p_list.size(); ++i) {
                const Point2f& p1 = p_list[i];
                const LineSeg2f l(p0, p1);
                if( l.intersection(reflect_out, cross_normal, cross_point, in) ) {
                    return true;
                }
                p0 = p1;
            }
            return false;
        }

        std::string toString() const noexcept override {
            return "linestrip[center " + p_center.toString() +
                    ", points " + std::to_string(p_list.size())+"]"; }
    };
    typedef std::shared_ptr<LineStrip2f> LineStrip2f_ref;

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_GEOM2F_HPP_ */
