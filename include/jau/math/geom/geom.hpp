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
#ifndef JAU_MATH_GEOM_GEOM_HPP_
#define JAU_MATH_GEOM_GEOM_HPP_

#include <cstdint>
#include <jau/enum_util.hpp>

namespace jau::math::geom {
    using namespace jau::enums;

    /** \addtogroup Math
     *
     *  @{
     */

    enum class orientation_t : uint16_t {
        /** Collinear **/
        COL,
        /** Clockwise **/
        CW,
        /** Counter-Clockwise **/
        CCW
    };
    JAU_MAKE_ENUM_STRING(orientation_t, COL, CW, CCW);

    enum class Winding : uint16_t {
        /** Clockwise **/
        CW,
        /** Counter-Clockwise **/
        CCW
    };
    JAU_MAKE_ENUM_STRING(Winding, CW, CCW);

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_MATH_GEOM_GEOM_HPP_ */
