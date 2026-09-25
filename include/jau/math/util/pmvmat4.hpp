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

#ifndef JAU_MATH_UTIL_PMVMAT4f_HPP_
#define JAU_MATH_UTIL_PMVMAT4f_HPP_

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <iostream>
#include <string>

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/enum_util.hpp>
#include <jau/functional.hpp>
#include <jau/math/geom/frustum.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/quaternion.hpp>
#include <jau/math/util/sstack.hpp>
#include <jau/math/util/syncbuffer.hpp>
#include <jau/type_concepts.hpp>

namespace jau::math::util {

    /** \addtogroup Math
     *
     *  @{
     */

    using namespace jau::enums;

    /** PMVMatrix4 modified core matrices */
    enum class PMVMod : uint32_t {
        none = 0,
        /** Bit value stating a modified getP() projection matrix (P), since last update() call. */
        proj = 1 << 0,
        /** Bit value stating a modified getMv() modelview matrix (Mv), since last update() call. */
        mv = 1 << 1,
        /** Bit value stating a modified getT() texture matrix (T), since last update() call. */
        text = 1 << 2,
        /** Bit value stating all is modified */
        all = proj | mv | text,
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(PMVMod, proj, mv, text);

    /** PMVMatrix4 derived matrices and values */
    enum class PMVData : uint32_t {
        none = 0,
        /** Bit value for getMvi() inverse modelview matrix (Mvi), updated via update(). */
        inv_mv = 1 << 1,
        /** Bit value for getMvit() inverse transposed modelview matrix (Mvit), updated via update(). */
        inv_tps_mv = 1 << 2,
        /** Bit value for getPi() inverse projection matrix (Pi), updated via update(). */
        inv_proj = 1 << 3,
        /** Bit value for getFrustum() frustum and updated by getFrustum(). */
        frustum = 1 << 4,
        /** Bit value for getPMv() pre-multiplied P x Mv, updated by getPMv(). */
        pre_pmv = 1 << 5,
        /** Bit value for getPMvi() pre-multiplied invert(P x Mv), updated by getPMvi(). */
        pre_pmvi = 1 << 6,
        /** Manual bits not covered by update() but getFrustum(), frustum, getPMv(), pre_pmv, getPMvi(), pre_pmvi, etc. */
        manual = frustum | pre_pmv | pre_pmvi
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(PMVData, inv_mv, inv_tps_mv, inv_proj,
                                           frustum, pre_pmv, pre_pmvi);

/**
 * PMVMatrix4 implements the basic computer graphics Matrix4 pack using
 * projection (P), modelview (Mv) and texture (T) Matrix4 operations.
 *
 * PMVMatrix4 provides the getMvi() inverse modelview matrix (Mvi) and
 * getMvit() inverse transposed modelview matrix (Mvit).
 * Frustum is also provided by getFrustum().
 *
 * To keep these derived values synchronized after mutable Mv operations like rotateMv(Quaternion)
 * users have to call update() before using Mvi and Mvit.
 *
 * All matrices are provided in column-major order,
 * as specified in the OpenGL fixed function pipeline, i.e. compatibility profile.
 * See Matrix4.
 *
 * Maintaining the inverse projection provides conversion to and from view space.
 *
 * Passing the view or inverse-view matrix to map-functions
 * allows conversion to and from world space.
 *
 * - view = V x M x Obj = Mv x Obj
 * - world = V' x Mv x Obj = V' x V x M x Obj = M x Obj
 * - clip = P x V x M x Obj = P x Mv x Obj
 * etc ..
 *
 * PMVMatrix4 can supplement com.jogamp.opengl.GL2ES2 GL2ES2 applications w/ the
 * lack of the described matrix functionality.
 *
 * <a name="storageDetails"><h5>Matrix storage details</h5></a>
 * The native data layout of the matrices are preserved, linear and can be utilized by `GLUniformData`
 * directly to be pushed to the GPU eventually via SyncMatrix4 and SyncMatrices4,
 * both SyncBuffer specializations for Matrix4.
 *
 * SyncBuffer's provided `sync_action_t` ensures that derived matrices, e.g. getMvi(), are updated before use.
 *
 * SyncBuffer's `sync_action_t` is called by `GLUniformData::getBuffer()`
 * i.e. before the data is pushed to the GPU.
 */
template <jau::req::packed_floating_point Value_type>
class PMVMatrix4 {
    public:
      typedef Value_type               value_type;
      typedef value_type*              pointer;
      typedef const value_type*        const_pointer;
      typedef value_type&              reference;
      typedef const value_type&        const_reference;
      typedef value_type*              iterator;
      typedef const value_type*        const_iterator;

      typedef Vector3F<value_type> Vec3;
      typedef Vector4F<value_type> Vec4;
      typedef Ray3F<value_type> Ray3;
      typedef Matrix4<value_type> Mat4;
      typedef SyncMatrices4<value_type> SyncMats4;

    private:
        Mat4 m_matP;
        Mat4 m_matMv;
        Mat4 m_matMvi;
        Mat4 m_matMvit;

        Mat4 m_matPi;
        Mat4 m_matTex;

        MatrixStack<value_type> m_stackMv, m_stackP, m_stackTex;

        PMVData m_requestBits; // may contain the requested bits: PMVData::inv_mv | PMVData::inv_proj | PMVData::inv_tps_mv

        PMVMod m_modifiedBits = PMVMod::all;
        PMVData m_dirtyBits = PMVData::none; // contains the dirty bits, i.e. hinting for update operation
        Mat4 m_matPMv;
        Mat4 m_matPMvi;
        bool m_matPMviOK;
        geom::Frustum m_frustum;

        constexpr static PMVData matToReq(PMVData req) noexcept {
            PMVData mask = PMVData::none;
            if( PMVData::none != ( req & ( PMVData::inv_mv | PMVData::inv_tps_mv ) ) ) {
                mask |= PMVData::inv_mv;
            }
            if( PMVData::none != ( req & PMVData::inv_tps_mv ) ) {
                mask |= PMVData::inv_tps_mv;
            }
            if( PMVData::none != ( req & PMVData::inv_proj ) ) {
                mask |= PMVData::inv_proj;
            }
            return mask;
        }

  public:


    /**
     * Creates an instance of PMVMatrix4.
     *
     * This constructor only sets up an instance w/o additional derived PMVData::inv_mv, PMVData::inv_proj or PMVData::inv_tps_mv PMVData::inv_tps_mv matrices.
     *
     * @see PMVMatrix4(int)
     */
    PMVMatrix4() noexcept
    : PMVMatrix4(PMVData::none) { }

    /**
     * Creates an instance of PMVMatrix4.
     *
     * Additional derived matrices can be requested via `derivedMatrices`, i.e.
     * - PMVData::inv_mv
     * - PMVData::inv_proj
     * - PMVData::inv_tps_mv
     *
     * Implementation uses native Matrix4 elements using column-order fields.
     * Derived matrices are updated at retrieval, e.g. getMvi(), or via synchronized access, e.g. makeSyncMvi(), to the actual Mat4 instances.
     *
     * @param derivedMatrices additional matrices can be requested by passing bits PMVData::inv_mv, PMVData::inv_proj and PMVData::inv_tps_mv
     * @see getReqBits()
     * @see isReqDirty()
     * @see getDirtyBits()
     * @see update()
     */
    PMVMatrix4(PMVData derivedMatrices) noexcept
    : m_requestBits( matToReq(derivedMatrices) )
    {
        m_matPMviOK = false;
        reset();
    }

    /** Returns the component's value_type signature */
    const jau::type_info& compSignature() const noexcept { return jau::static_ctti<value_type>(); }

    /** Return the number of Mat4 referenced by matrices() */
    static size_t matrixCount(PMVData derivedMatrices) noexcept {
        const PMVData requestBits = matToReq(derivedMatrices);
        {
            constexpr PMVData m = PMVData::inv_mv | PMVData::inv_tps_mv;
            if( m == ( m & requestBits ) ) {
                return 4; // P, Mv, Mvi and Mvit
            }
        }
        {
            constexpr PMVData m = PMVData::inv_mv;
            if( m == ( m & requestBits ) ) {
                return 3; // P, Mv, Mvi
            }
        }
        return 2; // P, Mv
    }
    /** Return the number of Mat4 referenced by matrices() */
    size_t matrixCount() const noexcept { return matrixCount(m_requestBits); }

    /**
     * Issues Mat4::loadIdentity() on all matrices and resets all internal states.
     */
    constexpr void reset() noexcept {
        m_matP.loadIdentity();
        m_matMv.loadIdentity();
        m_matTex.loadIdentity();

        m_modifiedBits = PMVMod::all;
        m_dirtyBits = m_requestBits | PMVData::manual;
    }

    //
    // Regular Mat4 access as well as their SyncedBuffer counterpart SyncedMatrix and SyncedMatrices
    //

    /**
     * Returns the GLMatrixFunc::GL_TEXTURE_MATRIX texture matrix (T).
     * <p>
     * Consider using setTextureDirty() if modifying the returned Mat4.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4& getT() noexcept { return m_matTex; }
    constexpr const Mat4& getT() const noexcept { return m_matTex; }

    /**
     * Returns the GLMatrixFunc::GL_PROJECTION_MATRIX projection matrix (P).
     * <p>
     * Consider using setProjectionDirty() if modifying the returned Mat4.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4& getP() noexcept { return m_matP; }
    constexpr const Mat4& getP() const noexcept { return m_matP; }

    /**
     * Returns the GLMatrixFunc::GL_MODELVIEW_MATRIX modelview matrix (Mv).
     * <p>
     * Consider using setModelviewDirty() if modifying the returned Mat4.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4& getMv() noexcept { return m_matMv; }
    constexpr const Mat4& getMv() const noexcept { return m_matMv; }

    /**
     * Returns the inverse GLMatrixFunc::GL_MODELVIEW_MATRIX modelview matrix (Pi) if requested.
     *
     * Requires PMVData::inv_proj to be requested in the ctor.
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    const Mat4& getPi() noexcept {
        updateImpl(false);
        return m_matPi;
    }

    /**
     * Returns the inverse GLMatrixFunc::GL_MODELVIEW_MATRIX modelview matrix (Mvi) if requested.
     *
     * Requires PMVData::inv_mv to be requested in the ctor.
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    const Mat4& getMvi() noexcept {
        updateImpl(false);
        return m_matMvi;
    }

    /**
     * Returns the inverse transposed GLMatrixFunc::GL_MODELVIEW_MATRIX modelview matrix (Mvit) if requested.
     *
     * Requires PMVData::inv_tps_mv to be requested in the ctor.
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    const Mat4& getMvit() noexcept {
        updateImpl(false);
        return m_matMvit;
    }

    /**
     * Returns a new SyncMatrix of GLMatrixFunc::GL_PROJECTION_MATRIX projection matrix (P).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMats4 makeSyncP() noexcept { return SyncMats4(m_matP, 1); }

    /**
     * Returns a new SyncMatrix of GLMatrixFunc::GL_MODELVIEW_MATRIX modelview matrix (Mv).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMats4 makeSyncMv() noexcept { return SyncMats4(m_matMv, 1); }

    /**
     * Returns a new SyncMatrices4f of 2 matrices within one FloatBuffer: getP() P and getMv() Mv.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    SyncMats4f makeSyncPMv() noexcept { return SyncMats4f(m_matP, 2); }

    /**
     * Returns a new SyncMatrix of GLMatrixFunc::GL_TEXTURE_MATRIX texture matrix (T).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    SyncMats4 makeSyncT() noexcept { return SyncMat4(m_matTex, 1); }

    /**
     * Returns a new SyncMatrix of inverse GLMatrixFunc::GL_MODELVIEW_MATRIX modelview matrix (Mvi) if requested.
     *
     * Requires PMVData::inv_mv to be requested in the ctor.
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    SyncMats4 makeSyncMvi() noexcept {
        return SyncMats4(m_matMvi, 1, jau::bind_member(this, &PMVMatrix4::updateImpl0));
    }

    /**
     * Returns a new SyncMatrix of inverse transposed GLMatrixFunc::GL_MODELVIEW_MATRIX modelview matrix (Mvit) if requested.
     *
     * Requires PMVData::inv_tps_mv to be requested in the ctor.
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    SyncMats4 makeSyncMvit() noexcept {
        return SyncMats4(m_matMvit, 1, jau::bind_member(this, &PMVMatrix4::updateImpl0));
    }

    /**
     * Returns a new SyncMatrices4f of 3 matrices within one FloatBuffer: getP() P, getMv() Mv and getMvi() Mvi if requested.
     *
     * Requires PMVData::inv_mv to be requested in the ctor.
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    SyncMats4f makeSyncPMvMvi() noexcept {
        return SyncMats4f(m_matP, 3, jau::bind_member(this, &PMVMatrix4::updateImpl0));
    }

    /**
     * Returns a new SyncMatrices4f of 4 matrices within one FloatBuffer: getP() P, getMv() Mv, getMvi() Mvi and getMvit() Mvit if requested.
     *
     * Requires PMVData::inv_mv and PMVData::inv_tps_mv to be requested in the ctor.
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    SyncMats4f makeSyncPMvMviMvit() noexcept {
        return SyncMats4f(m_matP, 4, jau::bind_member(this, &PMVMatrix4::updateImpl0));
    }

    /**
     * Returns a new SyncMatrices4f of either 4 matrices makeSyncPMvMviMvit(), 3 matrices makeSyncPMvMvi() or 2 matrices makeSyncPMv()
     * depending on requestedBits().
     *
     * See <a href="#storageDetails"> matrix storage details</a>.
     */
    SyncMats4f makeSyncPMvReq() noexcept {
        {
            constexpr PMVData m = PMVData::inv_mv | PMVData::inv_tps_mv;
            if( m == ( m & m_requestBits ) ) {
                return makeSyncPMvMviMvit(); // P, Mv, Mvi and Mvit
            }
        }
        {
            constexpr PMVData m = PMVData::inv_mv;
            if( m == ( m & m_requestBits ) ) {
                return makeSyncPMvMvi(); // P, Mv, Mvi
            }
        }
        return makeSyncPMv(); // P, Mv
    }

    //
    // Basic Mat4, Vec3 and Vec4 operations similar to GLMatrixFunc
    //

    /**
     * Returns multiplication result of getP() P and getMv() Mv matrix, i.e.
     * <pre>
     *    result = P x Mv
     * </pre>
     * @param result 4x4 matrix storage for result
     * @return given result matrix for chaining
     */
    constexpr Mat4& getMulPMv(Mat4& result) noexcept {
        return result.mul(m_matP, m_matMv);
    }

    /**
     * Returns multiplication result of getMv() Mv and getP() P matrix, i.e.
     * <pre>
     *    result = Mv x P
     * </pre>
     * @param result 4x4 matrix storage for result
     * @return given result matrix for chaining
     */
    constexpr Mat4& getMulMvP(Mat4& result) noexcept {
        return result.mul(m_matMv, m_matP);
    }

    /**
     * v_out = Mv x v_in
     * @param v_in input vector, can be v_out for in-place transformation
     * @param v_out output vector
     * @returns v_out for chaining
     */
    constexpr Vec4& mulWithMv(const Vec4& v_in, Vec4& v_out) noexcept {
        return m_matMv.mulVec4(v_in, v_out);
    }

    /**
     * v_inout = Mv x v_inout
     * @param v_inout input and output vector, i.e. in-place transformation
     * @returns v_inout for chaining
     */
    constexpr Vec4& mulWithMv(Vec4& v_inout) noexcept {
        return m_matMv.mulVec4(v_inout);
    }

    /**
     * v_out = Mv x v_in
     *
     * Affine 3f-vector transformation by 4x4 matrix, see Mat4::mulVec3(Vec3, Vec3).
     *
     * @param v_in input vector, can be v_out for in-place transformation
     * @param v_out output vector
     * @returns v_out for chaining
     */
    constexpr Vec3& mulWithMv(const Vec3& v_in, Vec3& v_out) noexcept {
        return m_matMv.mulVec3(v_in, v_out);
    }

    //
    // GLMatrixFunc alike functionality
    //

    /**
     * Load the getMv() modelview matrix with the provided values.
     */
    constexpr PMVMatrix4& loadMv(float values[]) noexcept {
        m_matMv.load(values);
        setModelviewDirty();
        return *this;
    }
    /**
     * Load the getMv() modelview matrix with the values of the given Mat4.
     */
    constexpr PMVMatrix4& loadMv(const Mat4& m) noexcept {
        m_matMv.load(m);
        setModelviewDirty();
        return *this;
    }
    /**
     * Load the getMv() modelview matrix with the values of the given Quaternion's rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMatrix4& loadMv(const Quat4f& quat) noexcept {
        quat.toMatrix(m_matMv);
        setModelviewDirty();
        return *this;
    }

    /**
     * Load the getP() projection matrix with the provided values.
     */
    constexpr PMVMatrix4& loadP(float values[]) noexcept {
        m_matP.load(values);
        setProjectionDirty();
        return *this;
    }
    /**
     * Load the getP() projection matrix with the values of the given Mat4.
     */
    constexpr PMVMatrix4& loadP(const Mat4& m) {
        m_matP.load(m);
        setProjectionDirty();
        return *this;
    }
    /**
     * Load the getP() projection matrix with the values of the given Quaternion's rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMatrix4& loadP(const Quat4f& quat) noexcept {
        quat.toMatrix(m_matP);
        setProjectionDirty();
        return *this;
    }

    /**
     * Load the getT() texture matrix with the provided values.
     */
    constexpr PMVMatrix4& loadT(float values[]) noexcept {
        m_matTex.load(values);
        setTextureDirty();
        return *this;
    }
    /**
     * Load the getT() texture matrix with the values of the given Mat4.
     */
    constexpr PMVMatrix4& loadT(Mat4& m) noexcept {
        m_matTex.load(m);
        setTextureDirty();
        return *this;
    }
    /**
     * Load the getT() texture matrix with the values of the given Quaternion's rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMatrix4& loadT(const Quat4f& quat) noexcept {
        quat.toMatrix(m_matTex);
        setTextureDirty();
        return *this;
    }

    /**
     * Load the getMv() modelview matrix with the values of the given Mat4.
     */
    constexpr PMVMatrix4& loadMvIdentity() noexcept {
        m_matMv.loadIdentity();
        setModelviewDirty();
        return *this;
    }

    /**
     * Load the getP() projection matrix with the values of the given Mat4.
     */
    constexpr PMVMatrix4& loadPIdentity() noexcept {
        m_matP.loadIdentity();
        setProjectionDirty();
        return *this;
    }

    /**
     * Load the getT() texture matrix with the values of the given Mat4.
     */
    constexpr PMVMatrix4& loadTIdentity() noexcept {
        m_matTex.loadIdentity();
        setTextureDirty();
        return *this;
    }

    /**
     * Multiply the getMv() modelview matrix: [c] = [c] x [m]
     * @param m the right hand Mat4
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& mulMv(const Mat4& m) noexcept {
        m_matMv.mul( m );
        setModelviewDirty();
        return *this;
    }

    /**
     * Multiply the getP() projection matrix: [c] = [c] x [m]
     * @param m the right hand Mat4
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& mulP(const Mat4& m) noexcept {
        m_matP.mul( m );
        setProjectionDirty();
        return *this;
    }

    /**
     * Multiply the getT() texture matrix: [c] = [c] x [m]
     * @param m the right hand Mat4
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& mulT(const Mat4& m) noexcept {
        m_matTex.mul( m );
        setTextureDirty();
        return *this;
    }

    /**
     * Translate the getMv() modelview matrix.
     * @param x
     * @param y
     * @param z
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& translateMv(float x, float y, float z) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToTranslation(x, y, z) );
    }
    /**
     * Translate the getMv() modelview matrix.
     * @param t translation vec3
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& translateMv(const Vec3& t) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToTranslation(t) );
    }

    /**
     * Translate the getP() projection matrix.
     * @param x
     * @param y
     * @param z
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& translateP(float x, float y, float z) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToTranslation(x, y, z) );
    }
    /**
     * Translate the getP() projection matrix.
     * @param t translation vec3
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& translateP(const Vec3& t) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToTranslation(t) );
    }

    /**
     * Scale the getMv() modelview matrix.
     * @param x
     * @param y
     * @param z
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& scaleMv(float x, float y, float z) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToScale(x, y, z) );
    }
    /**
     * Scale the getMv() modelview matrix.
     * @param s scale vec4f
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& scaleMv(const Vec3& s) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToScale(s) );
    }

    /**
     * Scale the getP() projection matrix.
     * @param x
     * @param y
     * @param z
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& scaleP(float x, float y, float z) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToScale(x, y, z) );
    }
    /**
     * Scale the getP() projection matrix.
     * @param s scale vec4f
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& scaleP(const Vec3& s) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToScale(s) );
    }

    /**
     * Rotate the getMv() modelview matrix by the given axis and angle in radians.
     * <p>
     * Consider using rotateMv(Quaternion)
     * </p>
     * @param ang_rad angle in radians
     * @param axis rotation axis
     * @return *this instance of chaining
     * @see #rotateMv(Quaternion)
     */
    constexpr_cxx26 PMVMatrix4& rotateMv(const float ang_rad, const float x, const float y, const float z) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToRotationAxis(ang_rad, x, y, z) );
    }
    /**
     * Rotate the getMv() modelview matrix by the given axis and angle in radians.
     * <p>
     * Consider using rotateMv(Quaternion)
     * </p>
     * @param ang_rad angle in radians
     * @param axis rotation axis
     * @return *this instance of chaining
     * @see #rotateMv(Quaternion)
     */
    constexpr_cxx26 PMVMatrix4& rotateMv(const float ang_rad, const Vec3& axis) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToRotationAxis(ang_rad, axis) );
    }
    /**
     * Rotate the getMv() modelview matrix} with the given Quaternion's rotation Mat4::setToRotation(Quaternion) matrix representation.
     * @param quat the Quaternion
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& rotateMv(const Quat4f& quat) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( quat.toMatrix(mat4Tmp1) );
    }

    /**
     * Rotate the getP() projection matrix by the given axis and angle in radians.
     * <p>
     * Consider using rotateP(Quaternion)
     * </p>
     * @param ang_rad angle in radians
     * @param axis rotation axis
     * @return *this instance of chaining
     * @see #rotateP(Quaternion)
     */
    constexpr_cxx26 PMVMatrix4& rotateP(const float ang_rad, const float x, const float y, const float z) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToRotationAxis(ang_rad, x, y, z) );
    }
    /**
     * Rotate the getP() projection matrix by the given axis and angle in radians.
     * <p>
     * Consider using rotateP(Quaternion)
     * </p>
     * @param ang_rad angle in radians
     * @param axis rotation axis
     * @return *this instance of chaining
     * @see #rotateP(Quaternion)
     */
    constexpr_cxx26 PMVMatrix4& rotateP(const float ang_rad, const Vec3& axis) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToRotationAxis(ang_rad, axis) );
    }
    /**
     * Rotate the getP() projection matrix} with the given Quaternion's rotation Mat4::setToRotation(Quaternion) matrix representation.
     * @param quat the Quaternion
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& rotateP(const Quat4f& quat) noexcept {
        Mat4 mat4Tmp1;
        return mulP( quat.toMatrix(mat4Tmp1) );
    }

    /** Pop the getMv() modelview matrix from its stack. */
    constexpr_cxx20 PMVMatrix4& popMv() noexcept {
        m_stackMv.pop(m_matMv);
        setModelviewDirty();
        return *this;
    }
    /** Pop the getP() projection matrix from its stack. */
    constexpr_cxx20 PMVMatrix4& popP() noexcept {
        m_stackP.pop(m_matP);
        setProjectionDirty();
        return *this;
    }
    /** Pop the getT() texture matrix from its stack. */
    constexpr_cxx20 PMVMatrix4& popT() noexcept {
        m_stackTex.pop(m_matTex);
        setTextureDirty();
        return *this;
    }
    /** Push the getMv() modelview matrix to its stack, while preserving its values. */
    constexpr_cxx20 PMVMatrix4& pushMv() noexcept {
        m_stackMv.push(m_matMv);
        return *this;
    }
    /** Push the getP() projection matrix to its stack, while preserving its values. */
    constexpr_cxx20 PMVMatrix4& pushP() noexcept {
        m_stackP.push(m_matP);
        return *this;
    }
    /** Push the getT() texture matrix to its stack, while preserving its values. */
    constexpr_cxx20 PMVMatrix4& pushT() noexcept {
        m_stackTex.push(m_matTex);
        return *this;
    }

    /**
     * mulP(Mat4) Multiply the getP() projection matrix with the orthogonal matrix.
     * @param left
     * @param right
     * @param bottom
     * @param top
     * @param zNear
     * @param zFar
     * @see Mat4::setToOrtho(float, float, float, float, float, float)
     */
    constexpr void orthoP(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) noexcept {
        Mat4 mat4Tmp1;
        mulP( mat4Tmp1.setToOrtho(left, right, bottom, top, zNear, zFar) );
    }

    /**
     * mulP(Mat4) Multiply the getP() projection matrix with the frustum matrix.
     *
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     *                          or {@code left == right}, or {@code bottom == top}.
     * @see Mat4::setToFrustum(float, float, float, float, float, float)
     */
    void frustumP(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
        Mat4 mat4Tmp1;
        mulP( mat4Tmp1.setToFrustum(left, right, bottom, top, zNear, zFar) );
    }

    //
    // Extra functionality
    //

    /**
     * Set the getP() projection matrix to the perspective/frustum matrix.
     *
     * @param fovy_rad fov angle in radians
     * @param aspect aspect ratio width / height
     * @param zNear
     * @param zFar
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see Mat4::setToPerspective(float, float, float, float)
     */
    PMVMatrix4& setToPerspective(const float fovy_rad, const float aspect, const float zNear, const float zFar) {
        m_matP.setToPerspective(fovy_rad, aspect, zNear, zFar);
        setProjectionDirty();
        return *this;
    }

    /**
     * mulP(Mat4) Multiply the getP() projection matrix with the perspective/frustum matrix.
     *
     * @param fovy_rad fov angle in radians
     * @param aspect aspect ratio width / height
     * @param zNear
     * @param zFar
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see Mat4::setToPerspective(float, float, float, float)
     */
    PMVMatrix4& perspectiveP(const float fovy_rad, const float aspect, const float zNear, const float zFar) {
        Mat4 mat4Tmp1;
        mulP( mat4Tmp1.setToPerspective(fovy_rad, aspect, zNear, zFar) );
        return *this;
    }

    /**
     * Set the getMv() modelview matrix
     * to the eye, object and orientation (camera), i.e. Mat4::setToLookAt(Vec3, Vec3, Vec3, Mat4).
     */
    constexpr PMVMatrix4& setToLookAtMv(const Vec3& eye, const Vec3& center, const Vec3& up) noexcept {
        m_matMv.setToLookAt(eye, center, up);
        setModelviewDirty();
        return *this;
    }

    /**
     * mulMv(Mat4) Multiply the getMv() modelview matrix
     * with the eye, object and orientation (camera), i.e. Mat4::setToLookAt(Vec3, Vec3, Vec3, Mat4).
     */
    constexpr PMVMatrix4& lookAtMv(const Vec3& eye, const Vec3& center, const Vec3& up) noexcept {
        Mat4 mat4Tmp1;
        mulMv( mat4Tmp1.setToLookAt(eye, center, up) );
        return *this;
    }

    /**
     * Map object coordinates to window coordinates.
     *
     * Traditional <code>gluProject</code> implementation.
     *
     * @param objPos 3 component object coordinate
     * @param viewport Rect4i viewport
     * @param winPos 3 component window coordinate, the result
     * @return true if successful, otherwise false (z is 1)
     */
    bool mapObjToWin(const Vec3& objPos, const Recti& viewport, Vec3& winPos) const noexcept {
        return Mat4::mapObjToWin(objPos, m_matMv, m_matP, viewport, winPos);
    }

    /**
     * Map world coordinates to window coordinates.
     *
     * - world = M x Obj
     * - win   = P x V x World = P x V' x Mv
     * - V' x V x M = M, with Mv = V x M
     *
     * @param objPos 3 component object coordinate
     * @param matV the view matrix
     * @param viewport Rect4i viewport
     * @param winPos 3 component window coordinate, the result
     * @return true if successful, otherwise false (z is 1)
     */
    bool mapWorldToWin(const Vec3& objPos, const Mat4& matV, const Recti& viewport, Vec3& winPos) const noexcept {
        return Mat4::mapWorldToWin(objPos, matV, m_matP, viewport, winPos);
    }

    /**
     * Map view coordinates ( Mv x object ) to window coordinates.
     *
     * @param view view position, 3 component vector
     * @param mP projection matrix
     * @param viewport Rect4i viewport
     * @param winPos 3 component window coordinate, the result
     * @return true if successful, otherwise false (z is 1)
     */
    bool mapViewToWin(const Vec3& view, const Recti& viewport, Vec3& winPos) const noexcept {
        return Mat4::mapViewToWin(view, m_matP, viewport, winPos);
    }

    /**
     * Map window coordinates to object coordinates.
     * <p>
     * Traditional <code>gluUnProject</code> implementation.
     * </p>
     *
     * @param winx
     * @param winy
     * @param winz
     * @param viewport Rect4i viewport
     * @param objPos 3 component object coordinate, the result
     * @return true if successful, otherwise false (failed to invert matrix, or becomes infinity due to zero z)
     */
    bool mapWinToObj(const float winx, const float winy, const float winz,
                     const Recti& viewport, Vec3& objPos) noexcept {
        if( Mat4::mapWinToAny(winx, winy, winz, getPMvi(), viewport, objPos) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Map window coordinates to object coordinates.
     *
     * The PMVData::inv_proj must have been request in the constructor.
     *
     * - Pv' = P' x V', using getPi()
     * - V' x V x M = M, with Mv = V x M
     *
     * @param winx
     * @param winy
     * @param winz
     * @param matVi the inverse view matrix
     * @param viewport Rect4i viewport
     * @param objPos 3 component object coordinate, the result
     * @return true if successful, otherwise false (failed to invert matrix, or becomes infinity due to zero z)
     */
    bool mapWinToWorld(const float winx, const float winy, const float winz,
                       const Mat4& matVi, const Recti& viewport, Vec3& objPos) noexcept {
        Mat4 invPv;
        invPv.mul(getPi(), matVi);
        if( Mat4::mapWinToAny(winx, winy, winz, invPv, viewport, objPos) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Map window coordinates to view coordinates.
     *
     * The PMVData::inv_proj must have been request in the constructor.
     *
     * @param winx
     * @param winy
     * @param winz
     * @param viewport Rect4i viewport
     * @param objPos 3 component object coordinate, the result
     * @return true if successful, otherwise false (failed to invert matrix, or becomes infinity due to zero z)
     */
    bool mapWinToView(const float winx, const float winy, const float winz,
                       const Recti& viewport, Vec3& objPos) noexcept {
        if( Mat4::mapWinToAny(winx, winy, winz, getPi(), viewport, objPos) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Map window coordinates to object coordinates.
     * <p>
     * Traditional <code>gluUnProject4</code> implementation.
     * </p>
     *
     * @param winx
     * @param winy
     * @param winz
     * @param clipw
     * @param viewport Rect4i viewport
     * @param near
     * @param far
     * @param objPos 4 component object coordinate, the result
     * @return true if successful, otherwise false (failed to invert matrix, or becomes infinity due to zero z)
     */
    bool mapWinToObj4(const float winx, const float winy, const float winz, const float clipw,
                      const Recti& viewport, const float near, const float far, Vec4& objPos) const noexcept {
        if( Mat4::mapWinToObj4(winx, winy, winz, clipw, getPMvi(), viewport, near, far, objPos) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a Ray in object space.
     *
     * The resulting Ray maybe used for <i>picking</i>
     * using a AABBox::getRayIntersection(Vec3, Ray, float, bool) bounding box
     * of a shape also in object space.
     *
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * - see jau::math::util::getZBufferEpsilon()
     * - see jau::math::util::getZBufferValue()
     * - see jau::math::util::getOrthoWinZ()
     * @param winx
     * @param winy
     * @param winz0
     * @param winz1
     * @param viewport
     * @param ray storage for the resulting Ray
     * @return true if successful, otherwise false (failed to invert matrix, or becomes z is infinity)
     */
    bool mapWinToObjRay(const float winx, const float winy, const float winz0, const float winz1,
                        const Recti& viewport, Ray3f& ray) noexcept {
        return Mat4::mapWinToAnyRay(winx, winy, winz0, winz1, getPMvi(), viewport, ray);
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a Ray in world space.
     *
     * The resulting Ray maybe used for <i>picking</i>
     * using a AABBox::getRayIntersection(Vec3, Ray, float, bool) bounding box
     * of a shape also in world space.
     *
     * The PMVData::inv_proj must have been request in the constructor.
     *
     * - Pv' = P' x V', using getPi()
     * - V' x V x M = M, with Mv = V x M
     *
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * - see jau::math::util::getZBufferEpsilon()
     * - see jau::math::util::getZBufferValue()
     * - see jau::math::util::getOrthoWinZ()
     * @param winx
     * @param winy
     * @param winz0
     * @param winz1
     * @param matVi the inverse view matrix
     * @param viewport
     * @param ray storage for the resulting Ray
     * @return true if successful, otherwise false (failed to invert matrix, or becomes z is infinity)
     *
     * @see PMVData::inv_proj
     * @see setView()
     */
    bool mapWinToWorldRay(const float winx, const float winy, const float winz0, const float winz1,
                          const Mat4& matVi, const Recti& viewport, Ray3f& ray) noexcept {
        Mat4 invPv;
        invPv.mul(getPi(), matVi);
        return Mat4::mapWinToAnyRay(winx, winy, winz0, winz1, invPv, viewport, ray);
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a Ray in view space.
     *
     * The resulting Ray maybe used for <i>picking</i>
     * using a AABBox::getRayIntersection(Vec3, Ray, float, bool) bounding box
     * of a shape also in view space.
     *
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * - see jau::math::util::getZBufferEpsilon()
     * - see jau::math::util::getZBufferValue()
     * - see jau::math::util::getOrthoWinZ()
     * @param winx
     * @param winy
     * @param winz0
     * @param winz1
     * @param viewport
     * @param ray storage for the resulting Ray
     * @return true if successful, otherwise false (failed to invert matrix, or becomes z is infinity)
     */
    bool mapWinToViewRay(const float winx, const float winy, const float winz0, const float winz1,
                        const Recti& viewport, Ray3f& ray) noexcept {
        return Mat4::mapWinToAnyRay(winx, winy, winz0, winz1, getPi(), viewport, ray);
    }

    std::string& toString(std::string& sb, const std::string& f) const noexcept {
        int count = 3; // P, Mv, T

        sb.append("PMVMatrix4[req").append(to_string(m_requestBits))
          .append(", mod1").append(to_string(m_dirtyBits))
          .append(", mod2").append(to_string(m_modifiedBits))
          .append(", Projection\n");
        m_matP.toString(sb, f);
        sb.append(", Modelview\n");
        m_matMv.toString(sb, f);
        sb.append(", Texture\n");
        m_matTex.toString(sb, f);
        {
            sb.append(", P * Mv\n");
            m_matPMv.toString(sb, f);
            ++count;
        }
        {
            sb.append(", P * Mv\n");
            m_matPMvi.toString(sb, f);
            ++count;
        }
        if( is_set(m_requestBits, PMVData::inv_mv) ) {
            sb.append(", Inverse Modelview\n");
            m_matMvi.toString(sb, f);
            ++count;
        }
        if( is_set(m_requestBits, PMVData::inv_tps_mv) ) {
            sb.append(", Inverse Transposed Modelview\n");
            m_matMvit.toString(sb, f);
            ++count;
        }
        int tmpCount = 1;
        if( true ) { // null != mat4Tmp2 ) {
            ++tmpCount;
        }
        sb.append(", matrices "+std::to_string(count)+" + "+std::to_string(tmpCount)+" temp = "+std::to_string(count+tmpCount)+"]");
        return sb;
    }

    std::string toString() const noexcept {
        std::string sb;
        toString(sb, "%13.9f");
        return sb;
    }

    /**
     * Returns the modified bits due to mutable operations..
     * <p>
     * A modified bit is set, if the corresponding matrix had been modified by a mutable operation
     * since last update() or getModifiedBits(bool) getModifiedBits(true) call.
     * </p>
     * @param clear if true, clears the modified bits, otherwise leaves them untouched.
     *
     * @see PMVState::MODIFIED_PROJECTION
     * @see PMVState::MODIFIED_MODELVIEW
     * @see PMVState::MODIFIED_TEXTURE
     * @see getDirtyBits()
     * @see isReqDirty()
     */
    constexpr PMVMod getModifiedBits(const bool clear) noexcept {
        const PMVMod r = m_modifiedBits;
        if(clear) {
            m_modifiedBits = PMVMod::none;
        }
        return r;
    }

    /**
     * Returns the dirty bits due to mutable operations,
     * i.e.
     * - PMVData::inv_mv (if requested)
     * - PMVData::inv_proj (if requested)
     * - PMVData::inv_tps_mv (if requested)
     * - PMVData::frustum (always, cleared via getFrustum()
     * <p>
     * A dirty bit is set, if the corresponding matrix had been modified by a mutable operation
     * since last update() call and requested in the constructor PMVMatrix4(int).
     * </p>
     * <p>
     * update() clears the dirty state for the matrices and getFrustum() for FRUSTUM.
     * </p>
     *
     * @see #isReqDirty()
     * @see PMVData::inv_mv
     * @see PMVData::inv_proj
     * @see PMVData::inv_tps_mv
     * @see PMVMats::FRUSTUM
     * @see PMVMatrix4(PMVMats)
     * @see getMvi()
     * @see getMvit()
     * @see makeSyncPMvMvi()
     * @see makeSyncPMvMviMvit()
     * @see getFrustum()
     */
    constexpr PMVData getDirtyBits() noexcept {
        return m_dirtyBits;
    }

    /**
     * Returns true if the one of the getReqBits() requested bits are are set dirty due to mutable operations,
     * i.e. at least one of
     * - PMVData::inv_mv
     * - PMVData::inv_proj
     * - PMVData::inv_tps_mv
     * <p>
     * A dirty bit is set, if the corresponding matrix had been modified by a mutable operation
     * since last update() call and requested in the constructor PMVMatrix4(int).
     * </p>
     * <p>
     * update() clears the dirty state for the matrices and getFrustum() for FRUSTUM.
     * </p>
     *
     * @see PMVData::inv_mv
     * @see PMVData::inv_proj
     * @see PMVData::inv_tps_mv
     * @see PMVMatrix4(int)
     * @see getMvi()
     * @see getMvit()
     * @see makeSyncPMvMvi()
     * @see makeSyncPMvMviMvit()
     */
    constexpr bool isReqDirty() noexcept {
        return has_any(m_dirtyBits, m_requestBits);
    }

    /**
     * Sets the getMv() Modelview (Mv) matrix dirty and modified,
     * i.e. adds PMVData::inv_mv | PMVData::inv_tps_mv | MANUAL_BITS to getDirtyBits() dirty bits.
     * @see #isReqDirty()
     */
    constexpr void setModelviewDirty() noexcept {
        m_dirtyBits |= PMVData::inv_mv | PMVData::inv_tps_mv | PMVData::manual ;
        m_modifiedBits |= PMVMod::mv;
    }

    /**
     * Sets the getP() Projection (P) matrix dirty and modified,
     * i.e. adds PMVData::inv_proj | MANUAL_BITS to getDirtyBits() dirty bits.
     */
    constexpr void setProjectionDirty() noexcept {
        m_dirtyBits |= PMVData::inv_proj | PMVData::manual ;
        m_modifiedBits |= PMVMod::proj;
    }

    /**
     * Sets the getT() Texture (T) matrix modified.
     */
    constexpr void setTextureDirty() noexcept {
        m_modifiedBits |= PMVMod::text;
    }

    /**
     * Returns the request bit mask, which uses bit values equal to the dirty mask
     * and may contain
     * - PMVData::inv_mv
     * - PMVData::inv_proj
     * - PMVData::inv_tps_mv
     *
     * The request bit mask is set by in the constructor PMVMatrix4(PMVMats).
     *
     * @see PMVData::inv_mv
     * @see PMVData::inv_proj
     * @see PMVData::inv_tps_mv
     * @see PMVMatrix4(PMVMats)
     * @see getMvi()
     * @see getMvit()
     * @see makeSyncPMvMvi()
     * @see makeSyncPMvMviMvit()
     * @see getFrustum()
     */
    constexpr PMVData requestedBits() noexcept { return m_requestBits; }

    /**
     * Returns the pre-multiplied projection x modelview, P x Mv.
     * <p>
     * This Mat4 instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * update() has no effect on this Mat4.
     * </p>
     * <p>
     * This pre-multipled P x Mv is considered dirty, if its corresponding
     * getP() P matrix} or getMv() Mv matrix has been modified since its last update.
     * </p>
     * @see #update()
     */
    constexpr Mat4& getPMv() noexcept {
        if( is_set(m_dirtyBits, PMVData::pre_pmv) ) {
            m_matPMv.mul(m_matP, m_matMv);
            m_dirtyBits &= ~PMVData::pre_pmv;
        }
        return m_matPMv;
    }

    /**
     * Returns the pre-multiplied inverse projection x modelview,
     * if Mat4::invert(Mat4) succeeded, otherwise `null`.
     * <p>
     * This Mat4 instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * update() has no effect on this Mat4.
     * </p>
     * <p>
     * This pre-multipled invert(P x Mv) is considered dirty, if its corresponding
     * getP() P matrix or getMv() Mv matrix has been modified since its last update.
     * </p>
     * @see #update()
     */
    constexpr Mat4& getPMvi() noexcept {
        if( is_set(m_dirtyBits, PMVData::pre_pmvi) ) {
            Mat4& mPMv = getPMv();
            m_matPMviOK = m_matPMvi.invert(mPMv);
            m_dirtyBits &= ~PMVData::pre_pmvi;
        }
        return m_matPMvi; // matPMviOK ? matPMvi : null; // FIXME
    }

    /**
     * Returns the frustum, derived from projection x modelview.
     * <p>
     * This Frustum instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * update() has no effect on this Frustum.
     * </p>
     * <p>
     * The Frustum is considered dirty, if its corresponding
     * getP() P matrix or getMv() Mv matrix has been modified since its last update.
     * </p>
     * @see #update()
     */
    jau::math::geom::Frustum getFrustum() noexcept {
        if( is_set(m_dirtyBits, PMVData::frustum) ) {
            m_frustum.setFromMat(getPMv());
            m_dirtyBits &= ~PMVData::frustum;
        }
        return m_frustum;
    }

    /**
     * Update the derived getMvi() inverse modelview (Mvi),
     * getMvit() inverse transposed modelview (Mvit) matrices
     * <b>if</b> they isReqDirty() are dirty <b>and</b>
     * requested via the constructor PMVMatrix4(int).<br/>
     * Hence updates the following dirty bits.
     * - PMVData::inv_mv
     * - PMVData::inv_proj
     * - PMVData::inv_tps_mv
     *
     * The Frustum is updated only via getFrustum() separately.

     * The Mvi and Mvit matrices are considered dirty, if their corresponding
     * getMv() Mv matrix has been modified since their last update.

     * Method is automatically called by SyncMat4 and SyncMatrices4f
     * instances SyncAction as retrieved by e.g. makeSyncMvit().
     * This ensures an automatic update cycle if used with com.jogamp.opengl.GLUniformData.

     * Method may be called manually in case mutable operations has been called
     * and caller operates on already fetched references, i.e. not calling
     * getMvi(), getMvit() anymore.

     * Method clears the modified bits like getModifiedBits(bool) getModifiedBits(true),
     * which are set by any mutable operation. The modified bits have no impact
     * on this method, but the return value.
     *
     * @return true if any matrix has been modified since last update call or
     *         if the derived matrices Mvi and Mvit were updated, otherwise false.
     *         In other words, method returns true if any matrix used by the caller must be updated,
     *         e.g. uniforms in a shader program.
     *
     * @see getModifiedBits(bool)
     * @see isReqDirty()
     * @see PMVData::inv_mv
     * @see PMVData::inv_proj
     * @see PMVData::inv_tps_mv
     * @see PMVMatrix4(PMVMats)
     * @see getMvi()
     * @see getMvit()
     * @see makeSyncPMvMvi()
     * @see makeSyncPMvMviMvit()
     */
    bool update() noexcept {
        return updateImpl(true);
    }

    //
    // private
    //

  private:
    void updateImpl0() noexcept { updateImpl(false); }
    bool updateImpl(bool clearModBits) noexcept {
        bool mod = has_any(m_modifiedBits);
        if( clearModBits ) {
            m_modifiedBits = PMVMod::none;
            mod = false;
        }
        if( has_any( m_requestBits & ( m_dirtyBits & PMVData::inv_proj ) ) ) { // only if requested & dirty
            if( !m_matPi.invert(m_matP) ) {
                jau_DBG_ERR_PRINT("Invalid source P matrix, can't compute inverse: %s", m_matP.toString().c_str(), E_FILE_LINE);
                // still continue with other derived matrices
            } else {
                mod = true;
            }
            m_dirtyBits &= ~PMVData::inv_proj;
        }
        if( has_any( m_requestBits & ( m_dirtyBits & ( PMVData::inv_mv | PMVData::inv_tps_mv ) ) ) ) { // only if requested & dirty
            if( !m_matMvi.invert(m_matMv) ) {
                jau_DBG_ERR_PRINT("Invalid source Mv matrix, can't compute inverse: %s", m_matMv.toString().c_str(), E_FILE_LINE);
                m_dirtyBits &= ~(PMVData::inv_mv | PMVData::inv_tps_mv);
                return mod; // no successful update as we abort due to inversion failure, skip PMVData::inv_tps_mv as well
            }
            m_dirtyBits &= ~PMVData::inv_mv;
            mod = true;
        }
        if( has_any( m_requestBits & ( m_dirtyBits & PMVData::inv_tps_mv ) ) ) { // only if requested & dirty
            m_matMvit.transpose(m_matMvi);
            m_dirtyBits &= ~PMVData::inv_tps_mv;
            mod = true;
        }
        return mod;
    }
};

template <jau::req::packed_floating_point Value_type>
inline std::ostream& operator<<(std::ostream& out, const PMVMatrix4<Value_type>& v) noexcept {
    return out << v.toString();
}

typedef PMVMatrix4<float> PMVMat4f;

 /**@}*/

 } // namespace jau::math::util

 #endif // JAU_MATH_UTIL_PMVMAT4f_HPP_
