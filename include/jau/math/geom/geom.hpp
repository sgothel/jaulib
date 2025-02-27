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

namespace jau::math::geom {

    /** \addtogroup Math
     *
     *  @{
     */

    enum class orientation_t {
        /** Collinear **/
        COL,
        /** Clockwise **/
        CLW,
        /** Counter-Clockwise **/
        CCW
    };

    /**@}*/

} // namespace jau::math::geom

#endif /*  JAU_MATH_GEOM_GEOM_HPP_ */
