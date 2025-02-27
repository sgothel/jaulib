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

#ifndef JAU_MATH_FLOAT_UTIL_HPP_
#define JAU_MATH_FLOAT_UTIL_HPP_

#include <cmath>
#include <climits>

#include <jau/base_math.hpp>
#include <jau/string_util.hpp>

namespace jau::math::util {

    /** \addtogroup Math
     *
     *  @{
     */

      /**
       * Returns resolution of Z buffer of given parameter,
       * see <a href="http://www.sjbaker.org/steve/omniv/love_your_z_buffer.html">Love Your Z-Buffer</a>.
       * <pre>
       *  return z * z / ( zNear * (1&lt;&lt;zBits) - z )
       * </pre>
       * Examples:
       * <pre>
       * 1.5256461E-4 = 16 zBits, -0.2 zDist, 0.1 zNear
       * 6.1033297E-6 = 16 zBits, -1.0 zDist, 0.1 zNear
       * </pre>
       * @param zBits number of bits of Z precision, i.e. z-buffer depth
       * @param z distance from the eye to the object
       * @param zNear distance from eye to near clip plane
       * @return smallest resolvable Z separation at this range.
       */
      constexpr float getZBufferEpsilon(int zBits, float z, float zNear) noexcept {
          return z * z / ( zNear * float( 1 << zBits ) - z );
      }

      /**
       * Returns Z buffer value of given parameter,
       * see <a href="http://www.sjbaker.org/steve/omniv/love_your_z_buffer.html">Love Your Z-Buffer</a>.
       * <pre>
       *  float a = zFar / ( zFar - zNear )
       *  float b = zFar * zNear / ( zNear - zFar )
       *  return (int) ( (1&lt;&lt;zBits) * ( a + b / z ) )
       * </pre>
       * @param zBits number of bits of Z precision, i.e. z-buffer depth
       * @param z distance from the eye to the object
       * @param zNear distance from eye to near clip plane
       * @param zFar distance from eye to far clip plane
       * @return z buffer value
       */
      constexpr int getZBufferValue(int zBits, float z, float zNear, float zFar) noexcept {
          const float a = zFar / ( zFar - zNear );
          const float b = zFar * zNear / ( zNear - zFar );
          return (int) ( float(1<<zBits) * ( a + b / z ) );
      }

      /**
       * Returns orthogonal distance
       * (1f/zNear-1f/orthoZ) / (1f/zNear-1f/zFar);
       */
      constexpr float getOrthoWinZ(float orthoZ, float zNear, float zFar) noexcept {
          return (1.0f/zNear-1.0f/orthoZ) / (1.0f/zNear-1.0f/zFar);
      }

    /**@}*/

} // namespace jau

#endif /* JAU_MATH_FLOAT_UTIL_HPP_ */
