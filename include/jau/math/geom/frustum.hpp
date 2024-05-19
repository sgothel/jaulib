/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2010-2024 Gothel Software e.K.
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
#ifndef JAU_FRUSTUM_HPP_
#define JAU_FRUSTUM_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <string>

#include <jau/float_math.hpp>
#include <jau/math/math_error.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/fov_hv_halves.hpp>
#include <jau/math/geom/aabbox3f.hpp>

namespace jau::math::geom {

    /** \addtogroup Math
     *
     *  @{
     */

/**
 * Providing frustum {@link #getPlanes() planes} derived by different inputs
 * ({@link #updateFrustumPlanes(float[], int) P*MV}, ..) used to classify objects
 * <ul>
 *   <li> {@link #classifyPoint(Vec3f) point} </li>
 *   <li> {@link #classifySphere(Vec3f, float) sphere} </li>
 * </ul>
 * and to test whether they are outside
 * <ul>
 *   <li> {@link #isOutside(Vec3f) point} </li>
 *   <li> {@link #isSphereOutside(Vec3f, float) sphere} </li>
 *   <li> {@link #isOutside(AABBox) bounding-box} </li>
 *   <li> {@link #isOutside(Cube) cube} </li>
 * </ul>
 *
 * <p>
 * Extracting the world-frustum planes from the P*Mv:
 * <pre>
 * Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix
 *   Gil Gribb <ggribb@ravensoft.com>
 *   Klaus Hartmann <k_hartmann@osnabrueck.netsurf.de>
 *   http://graphics.cs.ucf.edu/cap4720/fall2008/plane_extraction.pdf
 * </pre>
 * Classifying Point, Sphere and AABBox:
 * <pre>
 * Efficient View Frustum Culling
 *   Daniel Sýkora <sykorad@fel.cvut.cz>
 *   Josef Jelínek <jelinej1@fel.cvut.cz>
 *   http://www.cg.tuwien.ac.at/hostings/cescg/CESCG-2002/DSykoraJJelinek/index.html
 * </pre>
 * <pre>
 * Lighthouse3d.com
 * http://www.lighthouse3d.com/tutorials/view-frustum-culling/
 * </pre>
 *
 * Fundamentals about Planes, Half-Spaces and Frustum-Culling:<br/>
 * <pre>
 * Planes and Half-Spaces,  Max Wagner <mwagner@digipen.edu>
 * http://www.emeyex.com/site/tuts/PlanesHalfSpaces.pdf
 * </pre>
 * <pre>
 * Frustum Culling,  Max Wagner <mwagner@digipen.edu>
 * http://www.emeyex.com/site/tuts/FrustumCulling.pdf
 * </pre>
 * </p>
 */
class Frustum {
  public:
    /** Index for left plane: {@value} */
    constexpr static const int LEFT   = 0;
    /** Index for right plane: {@value} */
    constexpr static const int RIGHT  = 1;
    /** Index for bottom plane: {@value} */
    constexpr static const int BOTTOM = 2;
    /** Index for top plane: {@value} */
    constexpr static const int TOP    = 3;
    /** Index for near plane: {@value} */
    constexpr static const int NEAR   = 4;
    /** Index for far plane: {@value} */
    constexpr static const int FAR    = 5;

    /**
     * {@link Frustum} description by {@link #fovhv} and {@link #zNear}, {@link #zFar}.
     */
    class FovDesc {
      public:
        /** Field of view in both directions, may not be centered, either {@link FovHVHalves#inTangents} or radians. */
        FovHVHalves fovhv;
        /** Near Z */
        float zNear;
        /** Far Z */
        float zFar;
        /**
         * @param fovhv field of view in both directions, may not be centered, either {@link FovHVHalves#inTangents} or radians
         * @param zNear
         * @param zFar
         * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}.
         */
        FovDesc(const FovHVHalves& fovhv_, const float zNear_, const float zFar_)
        : fovhv(fovhv_), zNear(zNear_), zFar(zFar_)
        {
            if( zNear <= 0.0f || zFar <= zNear ) {
                throw IllegalArgumentError("Requirements zNear > 0 and zFar > zNear, but zNear "+std::to_string(zNear)+", zFar "+std::to_string(zFar), E_FILE_LINE);
            }
        }

        constexpr FovDesc(const FovDesc& o) noexcept = default;
        constexpr FovDesc(FovDesc&& o) noexcept = default;
        FovDesc& operator=(const FovDesc&) noexcept = default;
        FovDesc& operator=(FovDesc&&) noexcept = default;

        std::string toString() noexcept {
            return "FrustumFovDesc["+fovhv.toStringInDegrees()+", Z["+std::to_string(zNear)+" - "+std::to_string(zFar)+"]]";
        }
    };

    /**
     * Plane equation := dot(n, x - p) = 0 ->  Ax + By + Cz + d == 0
     * <p>
     * In order to work w/ {@link Frustum#isOutside(AABBox) isOutside(..)} methods,
     * the normals have to point to the inside of the frustum.
     * </p>
     */
    class Plane {
      public:
        /** Normal of the plane */
        jau::math::Vec3f n;

        /** Distance to origin */
        float d;

        constexpr Plane() noexcept
        : n(), d(0) {}

        constexpr Plane(const Plane& o) noexcept = default;
        constexpr Plane(Plane&& o) noexcept = default;
        constexpr Plane& operator=(const Plane&) noexcept = default;
        constexpr Plane& operator=(Plane&&) noexcept = default;

        /**
         * Setup of plane using 3 points. None of the three points are mutated.
         * <p>
         * Since this method may not properly define whether the normal points inside the frustum,
         * consider using {@link #set(Vec3f, Vec3f)}.
         * </p>
         * @param p0 point on plane, used as the shared start-point for vec(p0->p1) and vec(p0->p2)
         * @param p1 point on plane
         * @param p2 point on plane
         * @return this plane for chaining
         */
        constexpr Plane& set(const jau::math::Vec3f& p0, const jau::math::Vec3f& p1, const jau::math::Vec3f& p2) noexcept {
            const jau::math::Vec3f v = p1 - p0;
            const jau::math::Vec3f u = p2 - p0;
            n.cross(v, u).normalize();
            d = (jau::math::Vec3f(n) * -1.0f).dot(p0);
            return *this;
        }

        /**
         * Setup of plane using given normal and one point on plane. The given normal is mutated, the point not mutated.
         * @param n_ normal to plane pointing to the inside of this frustum
         * @param p0 point on plane, consider choosing the closest point to origin
         * @return this plane for chaining
         */
        constexpr Plane& set(const jau::math::Vec3f& n_, const jau::math::Vec3f& p0) noexcept {
            n = n_;
            d = (n * -1.0f).dot(p0);
            return *this;
        }

        /** Sets the given vec4f {@code out} to {@code ( n, d )}. Returns {@code out} for chaining. */
        constexpr jau::math::Vec4f& toVec4f(jau::math::Vec4f& out) const noexcept {
            out.set(n, d);
            return out;
        }

        /**
         * Sets the given {@code [float[off]..float[off+4])} {@code out} to {@code ( n, d )}.
         * @param out the {@code float[off+4]} output array
         */
        constexpr void toFloats(float out[/* off+4] */]) const noexcept {
            out[0] = n.x;
            out[1] = n.y;
            out[2] = n.z;
            out[3] = d;
        }

        /**
         * Return signed distance of plane to given point.
         * <ul>
         *   <li>If dist &lt; 0 , then the point p lies in the negative halfspace.</li>
         *   <li>If dist = 0 , then the point p lies in the plane.</li>
         *   <li>If dist &gt; 0 , then the point p lies in the positive halfspace.</li>
         * </ul>
         * A plane cuts 3D space into 2 half spaces.
         * <p>
         * Positive halfspace is where the plane’s normals vector points into.
         * </p>
         * <p>
         * Negative halfspace is the <i>other side</i> of the plane, i.e. *-1
         * </p>
         **/
        constexpr float distanceTo(const float x, const float y, const float z) const noexcept {
            return n.x * x + n.y * y + n.z * z + d;
        }

        /** Return distance of plane to given point, see {@link #distanceTo(float, float, float)}. */
        constexpr float distanceTo(const jau::math::Vec3f& p) const noexcept {
            return n.dot(p) + d;
        }

        std::string toString() const noexcept {
            return "Plane[ [ " + n.toString() + " ], " + std::to_string(d) + "]";
        }
    };

  private:
    /** Normalized planes[l, r, b, t, n, f] */
	Plane planes[6];

  public:
	/**
	 * Creates an undefined instance w/o calculating the frustum.
	 * <p>
	 * Use one of the <code>update(..)</code> methods to set the {@link #getPlanes() planes}.
	 * </p>
	 * @see #updateByPlanes(Plane[])
	 * @see #updateFrustumPlanes(float[], int)
	 */
	constexpr Frustum() noexcept = default;

	constexpr Frustum(const Frustum& o) noexcept = default;
	constexpr Frustum(Frustum&& o) noexcept = default;
	constexpr Frustum& operator=(const Frustum&) noexcept = default;
	constexpr Frustum& operator=(Frustum&&) noexcept = default;

    /**
     * Sets each of the given {@link Vec4f}[6] {@code out} to {@link Plane#toVec4f(Vec4f)}
     * in the order {@link #LEFT}, {@link #RIGHT}, {@link #BOTTOM}, {@link #TOP}, {@link #NEAR}, {@link #FAR}.
     * @param out the jau::math::vec4f[6] output array
     * @return {@code out} for chaining
     */
	constexpr jau::math::Vec4f* getPlanes(jau::math::Vec4f out[/*6*/]) const noexcept {
        planes[LEFT  ].toVec4f(out[0]);
        planes[RIGHT ].toVec4f(out[1]);
        planes[BOTTOM].toVec4f(out[2]);
        planes[TOP   ].toVec4f(out[3]);
        planes[NEAR  ].toVec4f(out[4]);
        planes[FAR   ].toVec4f(out[5]);
        return out;
    }

    /** Sets the given {@code [float[off]..float[off+4*6])} {@code out} to {@code ( n, d )}. */
    /**
     * Sets each of the given {@code [float[off]..float[off+4*6])} {@code out} to {@link Plane#toFloats(float[], int)},
     * i.e. [n.x, n.y, n.z, d, ...].
     * <p>
     * Plane order is as follows: {@link #LEFT}, {@link #RIGHT}, {@link #BOTTOM}, {@link #TOP}, {@link #NEAR}, {@link #FAR}.
     * </p>
     * @param out the {@code float[off+4*6]} output array
     * @return {@code out} for chaining
     */
	constexpr void getPlanes(float out[/* off+4*6] */]) const noexcept {
        planes[LEFT  ].toFloats(out+4*0);
        planes[RIGHT ].toFloats(out+4*1);
        planes[BOTTOM].toFloats(out+4*2);
        planes[TOP   ].toFloats(out+4*3);
        planes[NEAR  ].toFloats(out+4*4);
        planes[FAR   ].toFloats(out+4*5);
    }

    /**
     * Copy the given <code>src</code> planes into this this instance's planes.
     * @param src the 6 source planes
     */
	constexpr void updateByPlanes(const Plane src[/*6*/]) noexcept {
        planes[0] = src[0];
        planes[1] = src[1];
        planes[2] = src[2];
        planes[3] = src[3];
        planes[4] = src[4];
        planes[5] = src[5];
    }

    /**
     * {@link Plane}s are ordered in the returned array as follows:
     * <ul>
     *   <li>{@link #LEFT}</li>
     *   <li>{@link #RIGHT}</li>
     *   <li>{@link #BOTTOM}</li>
     *   <li>{@link #TOP}</li>
     *   <li>{@link #NEAR}</li>
     *   <li>{@link #FAR}</li>
     * </ul>
     * <p>
     * {@link Plane}'s normals are pointing to the inside of the frustum
     * in order to work w/ {@link #isOutside(AABBox) isOutside(..)} methods.
     * </p>
     *
     * @return array of normalized {@link Plane}s, order see above.
     */
	constexpr Plane* getPlanes() noexcept { return planes; }

    /**
     * Calculate the frustum planes in world coordinates
     * using the passed {@link FovDesc}.
     * <p>
     * Operation Details:
     * <ul>
     *   <li>The given {@link FovDesc} will be transformed
     *       into the given perspective matrix (column major order) first,
     *       see {@link Matrix4f#setToPerspective(FovHVHalves, float, float)}.</li>
     *   <li>Then the perspective matrix is used to {@link Matrix4f#updateFrustumPlanes(Frustum)} this instance.</li>
     * </ul>
     * </p>
     * <p>
     * Frustum plane's normals will point to the inside of the viewing frustum,
     * as required by this class.
     * </p>
     *
     * @param m 4x4 matrix in column-major order (also result)
     * @param fovDesc {@link Frustum} {@link FovDesc}
     * @return given matrix for chaining
     * @see Matrix4f#setToPerspective(FovHVHalves, float, float)
     * @see Matrix4f#updateFrustumPlanes(Frustum)
     * @see Matrix4f#getFrustum(Frustum, FovDesc)
     */
    Mat4f& updateByFovDesc(jau::math::Mat4f& m, const FovDesc& fovDesc) noexcept {
        m.setToPerspective(fovDesc.fovhv, fovDesc.zNear, fovDesc.zFar);
        setFromMat(m);
        return m;
    }

    /**
     * Calculate the frustum planes in world coordinates
     * using the given column major order matrix, usually a projection (P) or premultiplied P*MV matrix.
     * <p>
     * Frustum plane's normals will point to the inside of the viewing frustum,
     * as required by this class.
     * </p>
     */
    Frustum& setFromMat(const jau::math::Mat4f& m) noexcept {
        // Left:   a = m41 + m11, b = m42 + m12, c = m43 + m13, d = m44 + m14  - [1..4] column-major
        // Left:   a = m30 + m00, b = m31 + m01, c = m32 + m02, d = m33 + m03  - [0..3] column-major
        {
            geom::Frustum::Plane& p = planes[geom::Frustum::LEFT];
            p.n.set( m.m30 + m.m00,
                     m.m31 + m.m01,
                     m.m32 + m.m02 );
            p.d    = m.m33 + m.m03;
        }

        // Right:  a = m41 - m11, b = m42 - m12, c = m43 - m13, d = m44 - m14  - [1..4] column-major
        // Right:  a = m30 - m00, b = m31 - m01, c = m32 - m02, d = m33 - m03  - [0..3] column-major
        {
            geom::Frustum::Plane& p = planes[geom::Frustum::RIGHT];
            p.n.set( m.m30 - m.m00,
                     m.m31 - m.m01,
                     m.m32 - m.m02 );
            p.d    = m.m33 - m.m03;
        }

        // Bottom: a = m41 + m21, b = m42 + m22, c = m43 + m23, d = m44 + m24  - [1..4] column-major
        // Bottom: a = m30 + m10, b = m31 + m11, c = m32 + m12, d = m33 + m13  - [0..3] column-major
        {
            geom::Frustum::Plane& p = planes[geom::Frustum::BOTTOM];
            p.n.set( m.m30 + m.m10,
                     m.m31 + m.m11,
                     m.m32 + m.m12 );
            p.d    = m.m33 + m.m13;
        }

        // Top:   a = m41 - m21, b = m42 - m22, c = m43 - m23, d = m44 - m24  - [1..4] column-major
        // Top:   a = m30 - m10, b = m31 - m11, c = m32 - m12, d = m33 - m13  - [0..3] column-major
        {
            geom::Frustum::Plane& p = planes[geom::Frustum::TOP];
            p.n.set( m.m30 - m.m10,
                     m.m31 - m.m11,
                     m.m32 - m.m12 );
            p.d    = m.m33 - m.m13;
        }

        // Near:  a = m41m31, b = m42m32, c = m43m33, d = m44m34  - [1..4] column-major
        // Near:  a = m30m20, b = m31m21, c = m32m22, d = m33m23  - [0..3] column-major
        {
            geom::Frustum::Plane& p = planes[geom::Frustum::NEAR];
            p.n.set( m.m30 + m.m20,
                     m.m31 + m.m21,
                     m.m32 + m.m22 );
            p.d    = m.m33 + m.m23;
        }

        // Far:   a = m41 - m31, b = m42 - m32, c = m43 - m33, d = m44 - m34  - [1..4] column-major
        // Far:   a = m30 - m20, b = m31 - m21, c = m32m22, d = m33m23  - [0..3] column-major
        {
            geom::Frustum::Plane& p = planes[geom::Frustum::FAR];
            p.n.set( m.m30 - m.m20,
                     m.m31 - m.m21,
                     m.m32 - m.m22 );
            p.d    = m.m33 - m.m23;
        }

        // Normalize all planes
        for (int i = 0; i < 6; ++i) {
            geom::Frustum::Plane& p = planes[i];
            const float invLen = 1.0f / p.n.length();
            p.n *= invLen;
            p.d *= invLen;
        }
        return *this;
    }

  private:
    static bool intersects(const Plane& p, const AABBox3f& box) noexcept {
	    const Vec3f& lo = box.low();
	    const Vec3f& hi = box.high();

		return p.distanceTo(lo.x, lo.y, lo.z) > 0.0f ||
		       p.distanceTo(hi.x, lo.y, lo.z) > 0.0f ||
		       p.distanceTo(lo.x, hi.y, lo.z) > 0.0f ||
		       p.distanceTo(hi.x, hi.y, lo.z) > 0.0f ||
		       p.distanceTo(lo.x, lo.y, hi.z) > 0.0f ||
		       p.distanceTo(hi.x, lo.y, hi.z) > 0.0f ||
		       p.distanceTo(lo.x, hi.y, hi.z) > 0.0f ||
		       p.distanceTo(hi.x, hi.y, hi.z) > 0.0f;
	}

  public:

	/**
	 * Returns whether the given {@link AABBox} is completely outside of this frustum.
	 * <p>
	 * Note: If method returns false, the box may only be partially inside, i.e. intersects with this frustum
	 * </p>
	 */
    bool isOutside(const AABBox3f&& box) const noexcept {
        return !intersects(planes[0], box) ||
               !intersects(planes[1], box) ||
               !intersects(planes[2], box) ||
               !intersects(planes[3], box) ||
               !intersects(planes[4], box) ||
               !intersects(planes[5], box);
    }

    enum class location_t { OUTSIDE, INSIDE, INTERSECT };

    /**
     * Classifies the given {@link Vec3f} point whether it is outside, inside or on a plane of this frustum.
     *
     * @param p the point
     * @return {@link Location} of point related to frustum planes
     */
    location_t classifyPoint(const Vec3f& p) const noexcept {
        location_t res = location_t::INSIDE;

        for (int i = 0; i < 6; ++i) {
            const float d = planes[i].distanceTo(p);
            if ( d < 0.0f ) {
                return location_t::OUTSIDE;
            } else if ( d == 0.0f ) {
                res = location_t::INTERSECT;
            }
        }
        return res;
    }

    /**
     * Returns whether the given {@link Vec3f} point is completely outside of this frustum.
     *
     * @param p the point
     * @return true if outside of the frustum, otherwise inside or on a plane
     */
    bool isOutside(const Vec3f& p) const noexcept {
        return planes[0].distanceTo(p) < 0.0f ||
               planes[1].distanceTo(p) < 0.0f ||
               planes[2].distanceTo(p) < 0.0f ||
               planes[3].distanceTo(p) < 0.0f ||
               planes[4].distanceTo(p) < 0.0f ||
               planes[5].distanceTo(p) < 0.0f;
    }

    /**
     * Classifies the given sphere whether it is is outside, intersecting or inside of this frustum.
     *
     * @param p center of the sphere
     * @param radius radius of the sphere
     * @return {@link Location} of point related to frustum planes
     */
    location_t classifySphere(const Vec3f& p, const float radius) const noexcept {
        location_t res = location_t::INSIDE; // fully inside

        for (int i = 0; i < 6; ++i) {
            const float d = planes[i].distanceTo(p);
            if ( d < -radius ) {
                // fully outside
                return location_t::OUTSIDE;
            } else if (d < radius ) {
                // intersecting
                res = location_t::INTERSECT;
            }
        }
        return res;
    }

    /**
     * Returns whether the given sphere is completely outside of this frustum.
     *
     * @param p center of the sphere
     * @param radius radius of the sphere
     * @return true if outside of the frustum, otherwise inside or intersecting
     */
    bool isSphereOutside(const Vec3f& p, const float radius) const noexcept {
        return location_t::OUTSIDE == classifySphere(p, radius);
    }

    std::string toString() {
        std::string s;
        s.append("Frustum[Planes[").append("\n")
        .append(" L: ").append(planes[0].toString()).append(",\n")
        .append(" R: ").append(planes[1].toString()).append(",\n")
        .append(" B: ").append(planes[2].toString()).append(",\n")
        .append(" T: ").append(planes[3].toString()).append(",\n")
        .append(" N: ").append(planes[4].toString()).append(",\n")
        .append(" F: ").append(planes[5].toString()).append("],\n")
        .append("]");
        return s;
    }

};

/**@}*/

} // namespace jau::math::geom

#endif // JAU_FRUSTUM_HPP_

