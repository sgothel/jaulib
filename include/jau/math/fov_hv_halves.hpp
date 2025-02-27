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

#ifndef JAU_MATH_FOV_HV_HALVES_HPP_
#define JAU_MATH_FOV_HV_HALVES_HPP_

#include <cmath>
#include <cstdarg>
#include <string>

#include <jau/float_math.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

/**
 * Horizontal and vertical field of view (FOV) halves,
 * allowing a non-centered projection.
 * <p>
 * The values might be either in tangent or radians.
 * </p>
 */
class FovHVHalves {
  public:
    /** Half horizontal FOV from center to left, either in {@link #inTangents} or radians. */
    float left;
    /** Half horizontal FOV from center to right, either in {@link #inTangents} or radians. */
    float right;
    /** Half vertical FOV from center to top, either in {@link #inTangents} or radians. */
    float top;
    /** Half vertical FOV from center to bottom, either in {@link #inTangents} or radians. */
    float bottom;
    /** If true, values are in tangent, otherwise radians.*/
    bool inTangents;

    /**
     * Constructor for one {@link FovHVHalves} instance.
     * <p>
     * It is recommended to pass and store values in tangent
     * if used for perspective FOV calculations, since it will avoid conversion to tangent later on.
     * </p>
     * @param left_ half horizontal FOV, left side, in tangent or radians
     * @param right_ half horizontal FOV, right side, in tangent or radians
     * @param top_ half vertical FOV, top side, in tangent or radians
     * @param bottom_ half vertical FOV, bottom side, in tangent or radians
     * @param inTangents_ if true, values are in tangent, otherwise radians
     */
    constexpr FovHVHalves(const float left_, const float right_, const float top_, const float bottom_, const bool inTangents_) noexcept
    : left(left_), right(right_), top(top_), bottom(bottom_), inTangents(inTangents_) {}

    constexpr FovHVHalves(const FovHVHalves& o) noexcept = default;
    constexpr FovHVHalves(FovHVHalves&& o) noexcept = default;
    constexpr FovHVHalves& operator=(const FovHVHalves&) noexcept = default;
    constexpr FovHVHalves& operator=(FovHVHalves&&) noexcept = default;

    /**
     * Returns a symmetrical centered {@link FovHVHalves} instance in {@link #inTangents}, using:
     * <pre>
        halfHorizFovTan = tan( horizontalFov / 2f );
        halfVertFovTan  = tan( verticalFov / 2f );
     * </pre>
     * @param horizontalFov whole horizontal FOV in radians
     * @param verticalFov whole vertical FOV in radians
     */
    static FovHVHalves byRadians(const float horizontalFov, const float verticalFov) noexcept {
        const float halfHorizFovTan = std::tan(horizontalFov/2.0f);
        const float halfVertFovTan = std::tan(verticalFov/2.0f);
        return FovHVHalves(halfHorizFovTan, halfHorizFovTan, halfVertFovTan, halfVertFovTan, true);
    }

    /**
     * Returns a symmetrical centered {@link FovHVHalves} instance in {@link #inTangents}, using:
     * <pre>
        top  = bottom = tan( verticalFov / 2f );
        left =  right = aspect * top;
     * </pre>
     *
     * @param verticalFov vertical FOV in radians
     * @param aspect aspect ration width / height
     */
    static FovHVHalves byFovyRadianAndAspect(const float verticalFov, const float aspect) noexcept {
        const float halfVertFovTan = std::tan(verticalFov/2.0f);
        const float halfHorizFovTan = aspect * halfVertFovTan;
        return FovHVHalves(halfHorizFovTan, halfHorizFovTan,
                           halfVertFovTan, halfVertFovTan, true);
    }

    /**
     * Returns a custom symmetry {@link FovHVHalves} instance {@link #inTangents}, using:
     * <pre>
        left   = tan( horizontalFov * horizCenterFromLeft )
        right  = tan( horizontalFov * ( 1f - horizCenterFromLeft ) )
        top    = tan( verticalFov   * vertCenterFromTop )
        bottom = tan( verticalFov   * (1f - vertCenterFromTop ) )
     * </pre>
     * @param horizontalFov whole horizontal FOV in radians
     * @param horizCenterFromLeft horizontal center from left in [0..1]
     * @param verticalFov whole vertical FOV in radians
     * @param vertCenterFromTop vertical center from top in [0..1]
     */
    static FovHVHalves byRadians(const float horizontalFov, const float horizCenterFromLeft,
                                 const float verticalFov, const float vertCenterFromTop) noexcept {
        return FovHVHalves(std::tan(horizontalFov * horizCenterFromLeft),
                           std::tan(horizontalFov * ( 1.0f - horizCenterFromLeft )),
                           std::tan(verticalFov   * vertCenterFromTop),
                           std::tan(verticalFov   * (1.0f - vertCenterFromTop )),
                           true);
    }

    /**
     * Returns a custom symmetry {@link FovHVHalves} instance {@link #inTangents},
     * via computing the <code>horizontalFov</code> using:
     * <pre>
        halfVertFovTan  = tan( verticalFov / 2f );
        halfHorizFovTan = aspect * halfVertFovTan;
        horizontalFov   = atan( halfHorizFovTan ) * 2f;
        return {@link #byRadians(float, float, float, float) byRadians}(horizontalFov, horizCenterFromLeft, verticalFov, vertCenterFromTop)
     * </pre>
     * @param verticalFov whole vertical FOV in radians
     * @param vertCenterFromTop vertical center from top in [0..1]
     * @param aspect aspect ration width / height
     * @param horizCenterFromLeft horizontal center from left in [0..1]
     */
    static FovHVHalves byFovyRadianAndAspect(const float verticalFov, const float vertCenterFromTop,
                                             const float aspect, const float horizCenterFromLeft) noexcept {
        const float halfVertFovTan = std::tan(verticalFov/2.0f);
        const float halfHorizFovTan = aspect * halfVertFovTan;
        const float horizontalFov = std::atan(halfHorizFovTan) * 2.0f;
        return byRadians(horizontalFov, horizCenterFromLeft, verticalFov, vertCenterFromTop);
    }

    /**
     * Returns this instance <i>in tangent</i> values.
     * <p>
     * If this instance is {@link #inTangents} already, method returns a copy of this instance,
     * otherwise a newly created instance w/ converted values to tangent.
     * </p>
     */
    FovHVHalves toTangents() const noexcept {
        if( inTangents ) {
            return *this;
        } else {
            return FovHVHalves(std::tan(left), std::tan(right), std::tan(top), std::tan(bottom), true);
        }
    }

    /** Returns the full horizontal FOV, i.e. {@link #left} + {@link #right}, either in {@link #inTangents} or radians. */
    float horzFov() const noexcept { return left+right; }

    /** Returns the full vertical FOV, i.e. {@link #top} + {@link #bottom}, either in {@link #inTangents} or radians. */
    float vertFov() const noexcept { return top+bottom; }

    std::string toString() const noexcept {
        return "FovHVH["+(inTangents?std::string("tangents"):std::string("radians"))+": "+std::to_string(left)+" l, "+std::to_string(right)+" r, "+std::to_string(top)+" t, "+std::to_string(bottom)+" b]";
    }

    std::string toStringInDegrees() const noexcept {
        const float f = 180.0f / M_PI;
        std::string storedAs = inTangents?"tangents":"radians";
        if( inTangents ) {
            const float aleft = std::atan(left);
            const float aright = std::atan(right);
            const float atop = std::atan(top);
            const float abottom = std::atan(bottom);
            return "FovHVH[degrees: "+std::to_string(aleft*f)+" l, "+std::to_string(aright*f)+" r, "+std::to_string(atop*f)+" t, "+std::to_string(abottom*f)+" b, stored-as: "+storedAs+"]";
        } else {
            return "FovHVH[degrees: "+std::to_string(left*f)+" l, "+std::to_string(right*f)+" r, "+std::to_string(top*f)+" t, "+std::to_string(bottom*f)+" b, stored-as: "+storedAs+"]";
        }
    }
};

/**@}*/

} // namespace jau::math

#endif // JAU_MATH_FOV_HV_HALVES_HPP_

