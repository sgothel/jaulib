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

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cassert>
#include <string>
#include <iostream>

#include <jau/functional.hpp>
#include <jau/basic_types.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/quaternion.hpp>
#include <jau/math/geom/frustum.hpp>
#include <jau/math/util/syncbuffer.hpp>
#include <jau/math/util/sstack.hpp>

namespace jau::math::util {

    /** \addtogroup Math
     *
     *  @{
     */


/**
 * PMVMatrix4 implements the basic computer graphics Matrix4 pack using
 * projection (P), modelview (Mv) and texture (T) Matrix4 operations.
 *
 * PMVMatrix4 provides the {@link #getMvi() inverse modelview matrix (Mvi)} and
 * {@link #getMvit() inverse transposed modelview matrix (Mvit)}.
 * {@link Frustum} is also provided by {@link #getFrustum()}.
 *
 * To keep these derived values synchronized after mutable Mv operations like {@link #rotateMv(Quaternion)}
 * users have to call {@link #update()} before using Mvi and Mvit.
 *
 * All matrices are provided in column-major order,
 * as specified in the OpenGL fixed function pipeline, i.e. compatibility profile.
 * See Matrix4.
 *
 * PMVMatrix4 can supplement {@link com.jogamp.opengl.GL2ES2 GL2ES2} applications w/ the
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

template<typename Value_type,
         std::enable_if_t<std::is_floating_point_v<Value_type>, bool> = true>
class PMVMatrix4 {
    public:
      typedef Value_type               value_type;
      typedef value_type*              pointer;
      typedef const value_type*        const_pointer;
      typedef value_type&              reference;
      typedef const value_type&        const_reference;
      typedef value_type*              iterator;
      typedef const value_type*        const_iterator;

      typedef Vector3F<value_type, std::is_floating_point_v<Value_type>> Vec3;
      typedef Vector4F<value_type, std::is_floating_point_v<Value_type>> Vec4;
      typedef Ray3F<value_type, std::is_floating_point_v<Value_type>> Ray3;
      typedef Matrix4<value_type, std::is_floating_point_v<Value_type>> Mat4;
      typedef SyncMatrix4<value_type, std::is_floating_point_v<Value_type>> SyncMat4;
      typedef SyncMatrices4<value_type, std::is_floating_point_v<Value_type>> SyncMats4;

    private:
        class PMVSync1 : public SyncMat4 {
          private:
            const Mat4& m_mat;
            sync_action_t m_sync;

          public:
            PMVSync1(const Mat4& m, sync_action_t s) noexcept
            : m_mat(m), m_sync( std::move(s) )
            { }

            PMVSync1(const Mat4& m) noexcept
            : m_mat(m), m_sync( jau::bind_free(sync_action_fptr(nullptr)) )
            { }

            sync_action_t& action() noexcept override { return m_sync; }
            const Mat4& matrix() const noexcept override { return m_mat; }
        };

        class PMVSyncN : public SyncMats4 {
          private:
            const Mat4& m_mat;
            size_t m_count;
            sync_action_t m_sync;

          public:
            PMVSyncN(const Mat4& m, size_t count, sync_action_t s) noexcept
            : m_mat(m), m_count(count), m_sync( std::move(s) )
            { }
            PMVSyncN(const Mat4& m, size_t count) noexcept
            : m_mat(m), m_count(count), m_sync( jau::bind_free(sync_action_fptr(nullptr)) )
            { }

            sync_action_t& action() noexcept override { return m_sync; }
            const Mat4* matrices() const noexcept override { return &m_mat; }
            size_t matrixCount() const noexcept override { return m_count; }
        };

        Mat4 m_matP;
        Mat4 m_matMv;
        Mat4 m_matMvi;
        Mat4 m_matMvit;

        Mat4 m_matTex;

        MatrixStack<value_type> m_stackMv, m_stackP, m_stackTex;

        uint32_t m_requestBits; // may contain the requested bits: INVERSE_MODELVIEW | INVERSE_TRANSPOSED_MODELVIEW

        PMVSync1 m_syncP = PMVSync1(m_matP);
        PMVSync1 m_syncMv = PMVSync1(m_matMv);
        PMVSync1 m_syncTex = PMVSync1(m_matTex);

        PMVSync1 m_syncMvi = PMVSync1(m_matMvi, jau::bind_member(this, &PMVMatrix4::updateImpl0));
        PMVSync1 m_syncMvit = PMVSync1(m_matMvit, jau::bind_member(this, &PMVMatrix4::updateImpl0));

        PMVSyncN m_syncP_Mv = PMVSyncN(m_matP, 2);
        PMVSyncN m_syncP_Mv_Mvi = PMVSyncN(m_matP, 3, jau::bind_member(this, &PMVMatrix4::updateImpl0));
        PMVSyncN m_syncP_Mv_Mvi_Mvit = PMVSyncN(m_matP, 4, jau::bind_member(this, &PMVMatrix4::updateImpl0));

        uint32_t m_modifiedBits = MODIFIED_ALL;
        uint32_t m_dirtyBits = 0; // contains the dirty bits, i.e. hinting for update operation
        Mat4 m_matPMv;
        Mat4 m_matPMvi;
        bool m_matPMviOK;
        geom::Frustum m_frustum;

        constexpr static uint32_t matToReq(uint32_t req) noexcept {
            int mask = 0;
            if( 0 != ( req & ( INVERSE_MODELVIEW | INVERSE_TRANSPOSED_MODELVIEW ) ) ) {
                mask |= INVERSE_MODELVIEW;
            }
            if( 0 != ( req & INVERSE_TRANSPOSED_MODELVIEW ) ) {
                mask |= INVERSE_TRANSPOSED_MODELVIEW;
            }
            return mask;
        }

  public:

    /** Bit value stating a modified {@link #getP() projection matrix (P)}, since last {@link #update()} call. */
    constexpr static const uint32_t MODIFIED_PROJECTION = 1 << 0;
    /** Bit value stating a modified {@link #getMv() modelview matrix (Mv)}, since last {@link #update()} call. */
    constexpr static const uint32_t MODIFIED_MODELVIEW = 1 << 1;
    /** Bit value stating a modified {@link #getT() texture matrix (T)}, since last {@link #update()} call. */
    constexpr static const uint32_t MODIFIED_TEXTURE = 1 << 2;
    /** Bit value stating all is modified */
    constexpr static const uint32_t MODIFIED_ALL = MODIFIED_PROJECTION | MODIFIED_MODELVIEW | MODIFIED_TEXTURE;
    /** Bit value for {@link #getMvi() inverse modelview matrix (Mvi)}, updated via {@link #update()}. */
    constexpr static const uint32_t INVERSE_MODELVIEW = 1 << 1;
    /** Bit value for {@link #getMvit() inverse transposed modelview matrix (Mvit)}, updated via {@link #update()}. */
    constexpr static const uint32_t INVERSE_TRANSPOSED_MODELVIEW = 1 << 2;
    /** Bit value for {@link #getFrustum() frustum} and updated by {@link #getFrustum()}. */
    constexpr static const uint32_t FRUSTUM = 1 << 3;
    /** Bit value for {@link #getPMv() pre-multiplied P * Mv}, updated by {@link #getPMv()}. */
    constexpr static const uint32_t PREMUL_PMV = 1 << 4;
    /** Bit value for {@link #getPMvi() pre-multiplied invert(P * Mv)}, updated by {@link #getPMvi()}. */
    constexpr static const uint32_t PREMUL_PMVI = 1 << 5;
    /** Manual bits not covered by {@link #update()} but {@link #getFrustum()}, {@link #FRUSTUM}, {@link #getPMv()}, {@link #PREMUL_PMV}, {@link #getPMvi()}, {@link #PREMUL_PMVI}, etc. */
    constexpr static const uint32_t MANUAL_BITS = FRUSTUM | PREMUL_PMV | PREMUL_PMVI;

    /**
     * Creates an instance of PMVMatrix4.
     *
     * This constructor only sets up an instance w/o additional derived INVERSE_MODELVIEW or INVERSE_TRANSPOSED_MODELVIEW matrices.
     *
     * @see #PMVMatrix4(int)
     */
    PMVMatrix4() noexcept
    : PMVMatrix4(0) { }

    /**
     * Creates an instance of PMVMatrix4.
     *
     * Additional derived matrices can be requested via `derivedMatrices`, i.e.
     * - INVERSE_MODELVIEW
     * - INVERSE_TRANSPOSED_MODELVIEW
     *
     * Implementation uses native Matrix4 elements using column-order fields.
     * Derived matrices are updated at retrieval, e.g. getMvi(), or via synchronized access, e.g. getSyncMvi(), to the actual Mat4 instances.
     *
     * @param derivedMatrices additional matrices can be requested by passing bits {@link #INVERSE_MODELVIEW} and {@link #INVERSE_TRANSPOSED_MODELVIEW}.
     * @see #getReqBits()
     * @see #isReqDirty()
     * @see #getDirtyBits()
     * @see #update()
     */
    PMVMatrix4(int derivedMatrices) noexcept
    : m_requestBits( matToReq(derivedMatrices) )
    {
        m_matPMviOK = false;
        reset();
    }

    /**
     * Issues {@link Mat4#loadIdentity()} on all matrices and resets all internal states.
     */
    constexpr void reset() noexcept {
        m_matP.loadIdentity();
        m_matMv.loadIdentity();
        m_matTex.loadIdentity();

        m_modifiedBits = MODIFIED_ALL;
        m_dirtyBits = m_requestBits | MANUAL_BITS;
    }

    //
    // Regular Mat4 access as well as their SyncedBuffer counterpart SyncedMatrix and SyncedMatrices
    //

    /**
     * Returns the {@link GLMatrixFunc#GL_TEXTURE_MATRIX texture matrix} (T).
     * <p>
     * Consider using {@link #setTextureDirty()} if modifying the returned {@link Mat4}.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4& getT() noexcept { return m_matTex; }
    constexpr const Mat4& getT() const noexcept { return m_matTex; }

    /**
     * Returns the {@link SyncMatrix} of {@link GLMatrixFunc#GL_TEXTURE_MATRIX texture matrix} (T).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMat4& getSyncT() noexcept { return m_syncTex; }
    constexpr const SyncMats4& getSyncT() const noexcept { return m_syncTex; }

    /**
     * Returns the {@link GLMatrixFunc#GL_PROJECTION_MATRIX projection matrix} (P).
     * <p>
     * Consider using {@link #setProjectionDirty()} if modifying the returned {@link Mat4}.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4& getP() noexcept { return m_matP; }
    constexpr const Mat4& getP() const noexcept { return m_matP; }

    /**
     * Returns the {@link SyncMatrix} of {@link GLMatrixFunc#GL_PROJECTION_MATRIX projection matrix} (P).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMat4& getSyncP() noexcept { return m_syncP; }
    constexpr const SyncMats4& getSyncP() const noexcept { return m_syncP; }

    /**
     * Returns the {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mv).
     * <p>
     * Consider using {@link #setModelviewDirty()} if modifying the returned {@link Mat4}.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4& getMv() noexcept { return m_matMv; }
    constexpr const Mat4& getMv() const noexcept { return m_matMv; }

    /**
     * Returns the {@link SyncMatrix} of {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mv).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMat4& getSyncMv() noexcept { return m_syncMv; }
    constexpr const SyncMats4& getSyncMv() const noexcept { return m_syncMv; }

    /**
     * Returns {@link SyncMatrices4f} of 2 matrices within one FloatBuffer: {@link #getP() P} and {@link #getMv() Mv}.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMats4f& getSyncPMv() noexcept { return m_syncP_Mv; }
    constexpr const SyncMats4f& getSyncPMv() const noexcept { return m_syncP_Mv; }

    /**
     * Returns the inverse {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvi) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_MODELVIEW} has not been requested in ctor {@link #PMVMatrix4(int)}.
     */
    Mat4& getMvi() {
        if( 0 == ( INVERSE_MODELVIEW & m_requestBits ) ) { // FIXME
            throw jau::IllegalArgumentError("Not requested in ctor", E_FILE_LINE);
        }
        updateImpl(false);
        return m_matMvi;
    }

    /**
     * Returns the {@link SyncMatrix} of inverse {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvi) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_MODELVIEW} has not been requested in ctor {@link #PMVMatrix4(int)}.
     */
    SyncMat4& getSyncMvi() {
        if( 0 == ( INVERSE_MODELVIEW & m_requestBits ) ) { // FIXME
            throw jau::IllegalArgumentError("Not requested in ctor", E_FILE_LINE);
        }
        return m_syncMvi;
    }

    /**
     * Returns the inverse transposed {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvit) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_TRANSPOSED_MODELVIEW} has not been requested in ctor {@link #PMVMatrix4(int)}.
     */
    Mat4& getMvit() {
        if( 0 == ( INVERSE_TRANSPOSED_MODELVIEW & m_requestBits ) ) { // FIXME
            throw jau::IllegalArgumentError("Not requested in ctor", E_FILE_LINE);
        }
        updateImpl(false);
        return m_matMvit;
    }

    /**
     * Returns the {@link SyncMatrix} of inverse transposed {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvit) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_TRANSPOSED_MODELVIEW} has not been requested in ctor {@link #PMVMatrix4(int)}.
     */
    SyncMat4& getSyncMvit() {
        if( 0 == ( INVERSE_TRANSPOSED_MODELVIEW & m_requestBits ) ) { // FIXME
            throw jau::IllegalArgumentError("Not requested in ctor", E_FILE_LINE);
        }
        return m_syncMvit;
    }

    /**
     * Returns {@link SyncMatrices4f} of 3 matrices within one FloatBuffer: {@link #getP() P}, {@link #getMv() Mv} and {@link #getMvi() Mvi} if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_MODELVIEW} has not been requested in ctor {@link #PMVMatrix4(int)}.
     */
    SyncMats4f& getSyncPMvMvi() {
        if( 0 == ( INVERSE_MODELVIEW & m_requestBits ) ) { // FIXME
            throw jau::IllegalArgumentError("Not requested in ctor", E_FILE_LINE);
        }
        return m_syncP_Mv_Mvi;
    }

    /**
     * Returns {@link SyncMatrices4f} of 4 matrices within one FloatBuffer: {@link #getP() P}, {@link #getMv() Mv}, {@link #getMvi() Mvi} and {@link #getMvit() Mvit} if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_TRANSPOSED_MODELVIEW} has not been requested in ctor {@link #PMVMatrix4(int)}.
     */
    SyncMats4f& getSyncPMvMviMvit() {
        if( 0 == ( INVERSE_TRANSPOSED_MODELVIEW & m_requestBits ) ) { // FIXME
            throw jau::IllegalArgumentError("Not requested in ctor", E_FILE_LINE);
        }
        return m_syncP_Mv_Mvi_Mvit;
    }

    //
    // Basic Mat4, Vec3 and Vec4 operations similar to GLMatrixFunc
    //

    /**
     * Returns multiplication result of {@link #getP() P} and {@link #getMv() Mv} matrix, i.e.
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
     * Returns multiplication result of {@link #getMv() Mv} and {@link #getP() P} matrix, i.e.
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
     * v_out = Mv * v_in
     * @param v_in input vector, can be v_out for in-place transformation
     * @param v_out output vector
     * @returns v_out for chaining
     */
    constexpr Vec4& mulWithMv(const Vec4& v_in, Vec4& v_out) noexcept {
        return m_matMv.mulVec4(v_in, v_out);
    }

    /**
     * v_inout = Mv * v_inout
     * @param v_inout input and output vector, i.e. in-place transformation
     * @returns v_inout for chaining
     */
    constexpr Vec4& mulWithMv(Vec4& v_inout) noexcept {
        return m_matMv.mulVec4(v_inout);
    }

    /**
     * v_out = Mv * v_in
     *
     * Affine 3f-vector transformation by 4x4 matrix, see {@link Mat4#mulVec3(Vec3, Vec3)}.
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
     * Load the {@link #getMv() modelview matrix} with the provided values.
     */
    constexpr PMVMatrix4& loadMv(float values[]) noexcept {
        m_matMv.load(values);
        setModelviewDirty();
        return *this;
    }
    /**
     * Load the {@link #getMv() modelview matrix} with the values of the given {@link Mat4}.
     */
    constexpr PMVMatrix4& loadMv(const Mat4& m) noexcept {
        m_matMv.load(m);
        setModelviewDirty();
        return *this;
    }
    /**
     * Load the {@link #getMv() modelview matrix} with the values of the given {@link Quaternion}'s rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMatrix4& loadMv(const Quat4f& quat) noexcept {
        quat.toMatrix(m_matMv);
        setModelviewDirty();
        return *this;
    }

    /**
     * Load the {@link #getP() projection matrix} with the provided values.
     */
    constexpr PMVMatrix4& loadP(float values[]) noexcept {
        m_matP.load(values);
        setProjectionDirty();
        return *this;
    }
    /**
     * Load the {@link #getP() projection matrix} with the values of the given {@link Mat4}.
     */
    constexpr PMVMatrix4& loadP(const Mat4& m) {
        m_matP.load(m);
        setProjectionDirty();
        return *this;
    }
    /**
     * Load the {@link #getP() projection matrix} with the values of the given {@link Quaternion}'s rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMatrix4& loadP(const Quat4f& quat) noexcept {
        quat.toMatrix(m_matP);
        setProjectionDirty();
        return *this;
    }

    /**
     * Load the {@link #getT() texture matrix} with the provided values.
     */
    constexpr PMVMatrix4& loadT(float values[]) noexcept {
        m_matTex.load(values);
        setTextureDirty();
        return *this;
    }
    /**
     * Load the {@link #getT() texture matrix} with the values of the given {@link Mat4}.
     */
    constexpr PMVMatrix4& loadT(Mat4& m) noexcept {
        m_matTex.load(m);
        setTextureDirty();
        return *this;
    }
    /**
     * Load the {@link #getT() texture matrix} with the values of the given {@link Quaternion}'s rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMatrix4& loadT(const Quat4f& quat) noexcept {
        quat.toMatrix(m_matTex);
        setTextureDirty();
        return *this;
    }

    /**
     * Load the {@link #getMv() modelview matrix} with the values of the given {@link Mat4}.
     */
    constexpr PMVMatrix4& loadMvIdentity() noexcept {
        m_matMv.loadIdentity();
        setModelviewDirty();
        return *this;
    }

    /**
     * Load the {@link #getP() projection matrix} with the values of the given {@link Mat4}.
     */
    constexpr PMVMatrix4& loadPIdentity() noexcept {
        m_matP.loadIdentity();
        setProjectionDirty();
        return *this;
    }

    /**
     * Load the {@link #getT() texture matrix} with the values of the given {@link Mat4}.
     */
    constexpr PMVMatrix4& loadTIdentity() noexcept {
        m_matTex.loadIdentity();
        setTextureDirty();
        return *this;
    }

    /**
     * Multiply the {@link #getMv() modelview matrix}: [c] = [c] x [m]
     * @param m the right hand Mat4
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& mulMv(const Mat4& m) noexcept {
        m_matMv.mul( m );
        setModelviewDirty();
        return *this;
    }

    /**
     * Multiply the {@link #getP() projection matrix}: [c] = [c] x [m]
     * @param m the right hand Mat4
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& mulP(const Mat4& m) noexcept {
        m_matP.mul( m );
        setProjectionDirty();
        return *this;
    }

    /**
     * Multiply the {@link #getT() texture matrix}: [c] = [c] x [m]
     * @param m the right hand Mat4
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& mulT(const Mat4& m) noexcept {
        m_matTex.mul( m );
        setTextureDirty();
        return *this;
    }

    /**
     * Translate the {@link #getMv() modelview matrix}.
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
     * Translate the {@link #getMv() modelview matrix}.
     * @param t translation vec3
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& translateMv(const Vec3& t) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToTranslation(t) );
    }

    /**
     * Translate the {@link #getP() projection matrix}.
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
     * Translate the {@link #getP() projection matrix}.
     * @param t translation vec3
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& translateP(const Vec3& t) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToTranslation(t) );
    }

    /**
     * Scale the {@link #getMv() modelview matrix}.
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
     * Scale the {@link #getMv() modelview matrix}.
     * @param s scale vec4f
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& scaleMv(const Vec3& s) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( mat4Tmp1.setToScale(s) );
    }

    /**
     * Scale the {@link #getP() projection matrix}.
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
     * Scale the {@link #getP() projection matrix}.
     * @param s scale vec4f
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& scaleP(const Vec3& s) noexcept {
        Mat4 mat4Tmp1;
        return mulP( mat4Tmp1.setToScale(s) );
    }

    /**
     * Rotate the {@link #getMv() modelview matrix} by the given axis and angle in radians.
     * <p>
     * Consider using {@link #rotateMv(Quaternion)}
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
     * Rotate the {@link #getMv() modelview matrix} by the given axis and angle in radians.
     * <p>
     * Consider using {@link #rotateMv(Quaternion)}
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
     * Rotate the {@link #getMv() modelview matrix} with the given {@link Quaternion}'s rotation {@link Mat4#setToRotation(Quaternion) matrix representation}.
     * @param quat the {@link Quaternion}
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& rotateMv(const Quat4f& quat) noexcept {
        Mat4 mat4Tmp1;
        return mulMv( quat.toMatrix(mat4Tmp1) );
    }

    /**
     * Rotate the {@link #getP() projection matrix} by the given axis and angle in radians.
     * <p>
     * Consider using {@link #rotateP(Quaternion)}
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
     * Rotate the {@link #getP() projection matrix} by the given axis and angle in radians.
     * <p>
     * Consider using {@link #rotateP(Quaternion)}
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
     * Rotate the {@link #getP() projection matrix} with the given {@link Quaternion}'s rotation {@link Mat4#setToRotation(Quaternion) matrix representation}.
     * @param quat the {@link Quaternion}
     * @return *this instance of chaining
     */
    constexpr PMVMatrix4& rotateP(const Quat4f& quat) noexcept {
        Mat4 mat4Tmp1;
        return mulP( quat.toMatrix(mat4Tmp1) );
    }

    /** Pop the {@link #getMv() modelview matrix} from its stack. */
    constexpr_cxx20 PMVMatrix4& popMv() noexcept {
        m_stackMv.pop(m_matMv);
        setModelviewDirty();
        return *this;
    }
    /** Pop the {@link #getP() projection matrix} from its stack. */
    constexpr_cxx20 PMVMatrix4& popP() noexcept {
        m_stackP.pop(m_matP);
        setProjectionDirty();
        return *this;
    }
    /** Pop the {@link #getT() texture matrix} from its stack. */
    constexpr_cxx20 PMVMatrix4& popT() noexcept {
        m_stackTex.pop(m_matTex);
        setTextureDirty();
        return *this;
    }
    /** Push the {@link #getMv() modelview matrix} to its stack, while preserving its values. */
    constexpr_cxx20 PMVMatrix4& pushMv() noexcept {
        m_stackMv.push(m_matMv);
        return *this;
    }
    /** Push the {@link #getP() projection matrix} to its stack, while preserving its values. */
    constexpr_cxx20 PMVMatrix4& pushP() noexcept {
        m_stackP.push(m_matP);
        return *this;
    }
    /** Push the {@link #getT() texture matrix} to its stack, while preserving its values. */
    constexpr_cxx20 PMVMatrix4& pushT() noexcept {
        m_stackTex.push(m_matTex);
        return *this;
    }

    /**
     * {@link #mulP(Mat4) Multiply} the {@link #getP() projection matrix} with the orthogonal matrix.
     * @param left
     * @param right
     * @param bottom
     * @param top
     * @param zNear
     * @param zFar
     * @see Mat4#setToOrtho(float, float, float, float, float, float)
     */
    constexpr void orthoP(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) noexcept {
        Mat4 mat4Tmp1;
        mulP( mat4Tmp1.setToOrtho(left, right, bottom, top, zNear, zFar) );
    }

    /**
     * {@link #mulP(Mat4) Multiply} the {@link #getP() projection matrix} with the frustum matrix.
     *
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     *                          or {@code left == right}, or {@code bottom == top}.
     * @see Mat4#setToFrustum(float, float, float, float, float, float)
     */
    void frustumP(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
        Mat4 mat4Tmp1;
        mulP( mat4Tmp1.setToFrustum(left, right, bottom, top, zNear, zFar) );
    }

    //
    // Extra functionality
    //

    /**
     * {@link #mulP(Mat4) Multiply} the {@link #getP() projection matrix} with the perspective/frustum matrix.
     *
     * @param fovy_rad fov angle in radians
     * @param aspect aspect ratio width / height
     * @param zNear
     * @param zFar
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see Mat4#setToPerspective(float, float, float, float)
     */
    PMVMatrix4& perspectiveP(const float fovy_rad, const float aspect, const float zNear, const float zFar) {
        Mat4 mat4Tmp1;
        mulP( mat4Tmp1.setToPerspective(fovy_rad, aspect, zNear, zFar) );
        return *this;
    }

    /**
     * {@link #mulP(Mat4) Multiply} the {@link #getP() projection matrix}
     * with the eye, object and orientation, i.e. {@link Mat4#setToLookAt(Vec3, Vec3, Vec3, Mat4)}.
     */
    constexpr PMVMatrix4& lookAtP(const Vec3& eye, const Vec3& center, const Vec3& up) noexcept {
        Mat4 mat4Tmp2;
        Mat4 mat4Tmp1;
        mulP( mat4Tmp1.setToLookAt(eye, center, up, mat4Tmp2) );
        return *this;
    }

    /**
     * Map object coordinates to window coordinates.
     * <p>
     * Traditional <code>gluProject</code> implementation.
     * </p>
     *
     * @param objPos 3 component object coordinate
     * @param viewport Rect4i viewport
     * @param winPos 3 component window coordinate, the result
     * @return true if successful, otherwise false (z is 1)
     */
    bool mapObjToWin(const Vec3& objPos, const Recti& viewport, Vec3& winPos) noexcept {
        return Mat4::mapObjToWin(objPos, m_matMv, m_matP, viewport, winPos);
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
        if( Mat4::mapWinToObj(winx, winy, winz, getPMvi(), viewport, objPos) ) {
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
                      const Recti& viewport, const float near, const float far, Vec4& objPos) noexcept {
        if( Mat4::mapWinToObj4(winx, winy, winz, clipw, getPMvi(), viewport, near, far, objPos) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a {@link Ray}. The resulting {@link Ray} maybe used for <i>picking</i>
     * using a {@link AABBox#getRayIntersection(Vec3, Ray, float, bool) bounding box}.
     * <p>
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * <ul>
     *   <li>see {@link FloatUtil#getZBufferEpsilon(int, float, float)}</li>
     *   <li>see {@link FloatUtil#getZBufferValue(int, float, float, float)}</li>
     *   <li>see {@link FloatUtil#getOrthoWinZ(float, float, float)}</li>
     * </ul>
     * </p>
     * @param winx
     * @param winy
     * @param winz0
     * @param winz1
     * @param viewport
     * @param ray storage for the resulting {@link Ray}
     * @return true if successful, otherwise false (failed to invert matrix, or becomes z is infinity)
     */
    bool mapWinToRay(const float winx, const float winy, const float winz0, const float winz1,
                     const Recti& viewport, Ray3f& ray) noexcept {
        return Mat4::mapWinToRay(winx, winy, winz0, winz1, getPMvi(), viewport, ray);
    }

    std::string& toString(std::string& sb, const std::string& f) const noexcept {
        const bool pmvDirty  = 0 != (PREMUL_PMV & m_dirtyBits);
        const bool pmvUsed = true; // null != matPMv;

        const bool pmviDirty  = 0 != (PREMUL_PMVI & m_dirtyBits);
        const bool pmviUsed = true; // null != matPMvi;

        const bool frustumDirty = 0 != (FRUSTUM & m_dirtyBits);
        const bool frustumUsed = true; // null != frustum;

        const bool mviDirty  = 0 != (INVERSE_MODELVIEW & m_dirtyBits);
        const bool mviReq = 0 != (INVERSE_MODELVIEW & m_requestBits);

        const bool mvitDirty = 0 != (INVERSE_TRANSPOSED_MODELVIEW & m_dirtyBits);
        const bool mvitReq = 0 != (INVERSE_TRANSPOSED_MODELVIEW & m_requestBits);

        const bool modP = 0 != ( MODIFIED_PROJECTION & m_modifiedBits );
        const bool modMv = 0 != ( MODIFIED_MODELVIEW & m_modifiedBits );
        const bool modT = 0 != ( MODIFIED_TEXTURE & m_modifiedBits );
        int count = 3; // P, Mv, T

        sb.append("PMVMatrix4[modified[P ").append(std::to_string(modP)).append(", Mv ").append(std::to_string(modMv)).append(", T ").append(std::to_string(modT));
        sb.append("], dirty/used[PMv ").append(std::to_string(pmvDirty)).append("/").append(std::to_string(pmvUsed))
          .append(", Pmvi ").append(std::to_string(pmviDirty)).append("/").append(std::to_string(pmviUsed))
          .append(", Frustum ").append(std::to_string(frustumDirty)).append("/").append(std::to_string(frustumUsed));
        sb.append("], dirty/req[Mvi ").append(std::to_string(mviDirty)).append("/").append(std::to_string(mviReq))
          .append(", Mvit ").append(std::to_string(mvitDirty)).append("/").append(std::to_string(mvitReq)).append("]\n");
        sb.append(", Projection\n");
        m_matP.toString(sb, f);
        sb.append(", Modelview\n");
        m_matMv.toString(sb, f);
        sb.append(", Texture\n");
        m_matTex.toString(sb, f);
        if( pmvUsed ) {
            sb.append(", P * Mv\n");
            m_matPMv.toString(sb, f);
            ++count;
        }
        if( pmviUsed ) {
            sb.append(", P * Mv\n");
            m_matPMvi.toString(sb, f);
            ++count;
        }
        if( mviReq ) {
            sb.append(", Inverse Modelview\n");
            m_matMvi.toString(sb, f);
            ++count;
        }
        if( mvitReq ) {
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
     * since last {@link #update()} or {@link #getModifiedBits(bool) getModifiedBits(true)} call.
     * </p>
     * @param clear if true, clears the modified bits, otherwise leaves them untouched.
     *
     * @see #MODIFIED_PROJECTION
     * @see #MODIFIED_MODELVIEW
     * @see #MODIFIED_TEXTURE
     * @see #getDirtyBits()
     * @see #isReqDirty()
     */
    constexpr uint32_t getModifiedBits(const bool clear) noexcept {
        const uint32_t r = m_modifiedBits;
        if(clear) {
            m_modifiedBits = 0;
        }
        return r;
    }

    /**
     * Returns the dirty bits due to mutable operations,
     * i.e.
     * - {@link #INVERSE_MODELVIEW} (if requested)
     * - {@link #INVERSE_TRANSPOSED_MODELVIEW} (if requested)
     * - {@link #FRUSTUM} (always, cleared via {@link #getFrustum()}
     * <p>
     * A dirty bit is set, if the corresponding matrix had been modified by a mutable operation
     * since last {@link #update()} call and requested in the constructor {@link #PMVMatrix4(int)}.
     * </p>
     * <p>
     * {@link #update()} clears the dirty state for the matrices and {@link #getFrustum()} for {@link #FRUSTUM}.
     * </p>
     *
     * @see #isReqDirty()
     * @see #INVERSE_MODELVIEW
     * @see #INVERSE_TRANSPOSED_MODELVIEW
     * @see #FRUSTUM
     * @see #PMVMatrix4(int)
     * @see #getMvi()
     * @see #getMvit()
     * @see #getSyncPMvMvi()
     * @see #getSyncPMvMviMvit()
     * @see #getFrustum()
     */
    constexpr uint32_t getDirtyBits() noexcept {
        return m_dirtyBits;
    }

    /**
     * Returns true if the one of the {@link #getReqBits() requested bits} are are set dirty due to mutable operations,
     * i.e. at least one of
     * - {@link #INVERSE_MODELVIEW}
     * - {@link #INVERSE_TRANSPOSED_MODELVIEW}
     * <p>
     * A dirty bit is set, if the corresponding matrix had been modified by a mutable operation
     * since last {@link #update()} call and requested in the constructor {@link #PMVMatrix4(int)}.
     * </p>
     * <p>
     * {@link #update()} clears the dirty state for the matrices and {@link #getFrustum()} for {@link #FRUSTUM}.
     * </p>
     *
     * @see #INVERSE_MODELVIEW
     * @see #INVERSE_TRANSPOSED_MODELVIEW
     * @see #PMVMatrix4(int)
     * @see #getMvi()
     * @see #getMvit()
     * @see #getSyncPMvMvi()
     * @see #getSyncPMvMviMvit()
     */
    constexpr bool isReqDirty() noexcept {
        return 0 != ( m_requestBits & m_dirtyBits );
    }

    /**
     * Sets the {@link #getMv() Modelview (Mv)} matrix dirty and modified,
     * i.e. adds {@link #getReqBits() requested bits} and {@link #MANUAL_BITS} to {@link #getDirtyBits() dirty bits}.
     * @see #isReqDirty()
     */
    constexpr void setModelviewDirty() noexcept {
        m_dirtyBits |= m_requestBits | MANUAL_BITS ;
        m_modifiedBits |= MODIFIED_MODELVIEW;
    }

    /**
     * Sets the {@link #getP() Projection (P)} matrix dirty and modified,
     * i.e. adds {@link #MANUAL_BITS} to {@link #getDirtyBits() dirty bits}.
     */
    constexpr void setProjectionDirty() noexcept {
        m_dirtyBits |= MANUAL_BITS ;
        m_modifiedBits |= MODIFIED_PROJECTION;
    }

    /**
     * Sets the {@link #getT() Texture (T)} matrix modified.
     */
    constexpr void setTextureDirty() noexcept {
        m_modifiedBits |= MODIFIED_TEXTURE;
    }

    /**
     * Returns the request bit mask, which uses bit values equal to the dirty mask
     * and may contain
     * - {@link #INVERSE_MODELVIEW}
     * - {@link #INVERSE_TRANSPOSED_MODELVIEW}
     * <p>
     * The request bit mask is set by in the constructor {@link #PMVMatrix4(int)}.
     * </p>
     *
     * @see #INVERSE_MODELVIEW
     * @see #INVERSE_TRANSPOSED_MODELVIEW
     * @see #PMVMatrix4(int)
     * @see #getMvi()
     * @see #getMvit()
     * @see #getSyncPMvMvi()
     * @see #getSyncPMvMviMvit()
     * @see #getFrustum()
     */
    constexpr uint32_t getReqBits() noexcept {
        return m_requestBits;
    }

    /**
     * Returns the pre-multiplied projection x modelview, P x Mv.
     * <p>
     * This {@link Mat4} instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * {@link #update()} has no effect on this {@link Mat4}.
     * </p>
     * <p>
     * This pre-multipled P x Mv is considered dirty, if its corresponding
     * {@link #getP() P matrix} or {@link #getMv() Mv matrix} has been modified since its last update.
     * </p>
     * @see #update()
     */
    constexpr Mat4& getPMv() noexcept {
        if( 0 != ( m_dirtyBits & PREMUL_PMV ) ) {
            m_matPMv.mul(m_matP, m_matMv);
            m_dirtyBits &= ~PREMUL_PMV;
        }
        return m_matPMv;
    }

    /**
     * Returns the pre-multiplied inverse projection x modelview,
     * if {@link Mat4#invert(Mat4)} succeeded, otherwise `null`.
     * <p>
     * This {@link Mat4} instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * {@link #update()} has no effect on this {@link Mat4}.
     * </p>
     * <p>
     * This pre-multipled invert(P x Mv) is considered dirty, if its corresponding
     * {@link #getP() P matrix} or {@link #getMv() Mv matrix} has been modified since its last update.
     * </p>
     * @see #update()
     */
    constexpr Mat4& getPMvi() noexcept {
        if( 0 != ( m_dirtyBits & PREMUL_PMVI ) ) {
            Mat4& mPMv = getPMv();
            m_matPMviOK = m_matPMvi.invert(mPMv);
            m_dirtyBits &= ~PREMUL_PMVI;
        }
        return m_matPMvi; // matPMviOK ? matPMvi : null; // FIXME
    }

    /**
     * Returns the frustum, derived from projection x modelview.
     * <p>
     * This {@link Frustum} instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * {@link #update()} has no effect on this {@link Frustum}.
     * </p>
     * <p>
     * The {@link Frustum} is considered dirty, if its corresponding
     * {@link #getP() P matrix} or {@link #getMv() Mv matrix} has been modified since its last update.
     * </p>
     * @see #update()
     */
    jau::math::geom::Frustum getFrustum() noexcept {
        if( 0 != ( m_dirtyBits & FRUSTUM ) ) {
            m_frustum.setFromMat(getPMv());
            m_dirtyBits &= ~FRUSTUM;
        }
        return m_frustum;
    }

    /**
     * Update the derived {@link #getMvi() inverse modelview (Mvi)},
     * {@link #getMvit() inverse transposed modelview (Mvit)} matrices
     * <b>if</b> they {@link #isReqDirty() are dirty} <b>and</b>
     * requested via the constructor {@link #PMVMatrix4(int)}.<br/>
     * Hence updates the following dirty bits.
     * - {@link #INVERSE_MODELVIEW}
     * - {@link #INVERSE_TRANSPOSED_MODELVIEW}
     * <p>
     * The {@link Frustum} is updated only via {@link #getFrustum()} separately.
     * </p>
     * <p>
     * The Mvi and Mvit matrices are considered dirty, if their corresponding
     * {@link #getMv() Mv matrix} has been modified since their last update.
     * </p>
     * <p>
     * Method is automatically called by {@link SyncMat4} and {@link SyncMatrices4f}
     * instances {@link SyncAction} as retrieved by e.g. {@link #getSyncMvit()}.
     * This ensures an automatic update cycle if used with {@link com.jogamp.opengl.GLUniformData}.
     * </p>
     * <p>
     * Method may be called manually in case mutable operations has been called
     * and caller operates on already fetched references, i.e. not calling
     * {@link #getMvi()}, {@link #getMvit()} anymore.
     * </p>
     * <p>
     * Method clears the modified bits like {@link #getModifiedBits(bool) getModifiedBits(true)},
     * which are set by any mutable operation. The modified bits have no impact
     * on this method, but the return value.
     * </p>
     *
     * @return true if any matrix has been modified since last update call or
     *         if the derived matrices Mvi and Mvit were updated, otherwise false.
     *         In other words, method returns true if any matrix used by the caller must be updated,
     *         e.g. uniforms in a shader program.
     *
     * @see #getModifiedBits(bool)
     * @see #isReqDirty()
     * @see #INVERSE_MODELVIEW
     * @see #INVERSE_TRANSPOSED_MODELVIEW
     * @see #PMVMatrix4(int)
     * @see #getMvi()
     * @see #getMvit()
     * @see #getSyncPMvMvi()
     * @see #getSyncPMvMviMvit()
     */
    bool update() {
        return updateImpl(true);
    }

    //
    // private
    //

  private:
    void updateImpl0() { updateImpl(false); }
    bool updateImpl(bool clearModBits) {
        bool mod = 0 != m_modifiedBits;
        if( clearModBits ) {
            m_modifiedBits = 0;
        }
        if( 0 != ( m_requestBits & ( ( m_dirtyBits & ( INVERSE_MODELVIEW | INVERSE_TRANSPOSED_MODELVIEW ) ) ) ) ) { // only if dirt requested & dirty
            if( !m_matMvi.invert(m_matMv) ) {
                throw jau::math::MathDomainError("Invalid source Mv matrix, can't compute inverse", E_FILE_LINE);
            }
            m_dirtyBits &= ~INVERSE_MODELVIEW;
            mod = true;
        }
        if( 0 != ( m_requestBits & ( m_dirtyBits & INVERSE_TRANSPOSED_MODELVIEW ) ) ) { // only if requested & dirty
            m_matMvit.transpose(m_matMvi);
            m_dirtyBits &= ~INVERSE_TRANSPOSED_MODELVIEW;
            mod = true;
        }
        return mod;
    }
};

template<typename Value_type,
         std::enable_if_t<std::is_floating_point_v<Value_type>, bool> = true>
inline std::ostream& operator<<(std::ostream& out, const PMVMatrix4<Value_type>& v) noexcept {
    return out << v.toString();
}

typedef PMVMatrix4<float> PMVMat4f;

 /**@}*/

 } // namespace jau::math::util

 #endif // JAU_MATH_UTIL_PMVMAT4f_HPP_
