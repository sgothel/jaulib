/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2014-2024 Gothel Software e.K.
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
#ifndef JAU_PMVMAT4f_HPP_
#define JAU_PMVMAT4f_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cassert>
#include <limits>
#include <string>
#include <vector>
#include <initializer_list>
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
 * PMVMat4f implements the basic computer graphics {@link Mat4f} pack using
 * projection (P), modelview (Mv) and texture (T) {@link Mat4f} operations.
 * <p>
 * Unlike {@link com.jogamp.opengl.util.PMVMatrix PMVMatrix}, this class doesn't implement
 * {@link com.jogamp.opengl.fixedfunc.GLMatrixFunc GLMatrixFunc} and is OpenGL agnostic.
 * </p>
 * <p>
 * This is the second implementation of `PMVMat4f` using
 * direct {@link Mat4f}, {@link Vec4f} and {@link Vec3f} math operations instead of `float[]`
 * via {@link com.jogamp.math.FloatUtil FloatUtil}.
 * </p>
 * <p>
 * PMVMat4f provides the {@link #getMvi() inverse modelview matrix (Mvi)} and
 * {@link #getMvit() inverse transposed modelview matrix (Mvit)}.
 * {@link Frustum} is also provided by {@link #getFrustum()}.
 *
 * To keep these derived values synchronized after mutable Mv operations like {@link #rotateMv(Quaternion)}
 * users have to call {@link #update()} before using Mvi and Mvit.
 * </p>
 * <p>
 * All matrices are provided in column-major order,
 * as specified in the OpenGL fixed function pipeline, i.e. compatibility profile.
 * See {@link Mat4f}.
 * </p>
 * <p>
 * PMVMat4f can supplement {@link com.jogamp.opengl.GL2ES2 GL2ES2} applications w/ the
 * lack of the described matrix functionality.
 * </p>
 * <a name="storageDetails"><h5>Matrix storage details</h5></a>
 * <p>
 * The {@link SyncBuffer} abstraction is provided, e.g. {@link #getSyncPMvMvi()},
 * to synchronize the respective {@link Mat4f matrices} with the `float[]` backing store.
 * The latter is represents the data to {@link com.jogamp.opengl.GLUniformData} via its {@link FloatBuffer}s, see {@link SyncBuffer#getBuffer()},
 * and is pushed to the GPU eventually.
 * </p>
 * <p>
 * {@link SyncBuffer}'s {@link SyncAction} is called by {@link com.jogamp.opengl.GLUniformData#getBuffer()},
 * i.e. before the data is pushed to the GPU.
 * </p>
 * <p>
 * The provided {@link SyncAction} ensures that the {@link Mat4f matrices data}
 * gets copied into the `float[]` backing store.
 * </p>
 * <p>
 * PMVMat4f provides two specializations of {@link SyncBuffer}, {@link SyncMat4f} for single {@link Mat4f} mappings
 * and {@link SyncMatrices4f} for multiple {@link Mat4f} mappings.
 * </p>
 * <p>
 * They can be feed directly to instantiate a {@link com.jogamp.opengl.GLUniformData} object via e.g. {@link com.jogamp.opengl.GLUniformData#GLUniformData(String, int, int, SyncBuffer)}.
 * </p>
 * <p>
 * All {@link Mat4f matrix} {@link SyncBuffer}'s backing store are backed up by a common primitive float-array for performance considerations
 * and are a {@link Buffers#slice2Float(float[], int, int) sliced} representation of it.
 * </p>
 * <p>
 * <b>{@link Mat4f} {@link SyncBuffer}'s Backing-Store Notes:</b>
 * <ul>
 *   <li>The {@link Mat4f matrix} {@link SyncBuffer}'s backing store is a {@link Buffers#slice2Float(float[], int, int) sliced part } of a host matrix and it's start position has been {@link FloatBuffer#mark() marked}.</li>
 *   <li>Use {@link FloatBuffer#reset() reset()} to rewind it to it's start position after relative operations, like {@link FloatBuffer#get() get()}.</li>
 *   <li>If using absolute operations like {@link FloatBuffer#get(int) get(int)}, use it's {@link FloatBuffer#reset() reset} {@link FloatBuffer#position() position} as it's offset.</li>
 * </ul>
 * </p>
 */
class PMVMat4f {
    private:
        class PMVSync1 : public SyncMat4f {
          private:
            Mat4f& m_mat;
            sync_action_t m_sync;

          public:
            PMVSync1(Mat4f& m, sync_action_t s) noexcept
            : m_mat(m), m_sync( std::move(s) )
            { }

            PMVSync1(Mat4f& m) noexcept
            : m_mat(m), m_sync( jau::bind_free(sync_action_fptr(nullptr)) )
            { }

            virtual sync_action_t& action() noexcept override { return m_sync; }
            virtual const Mat4f& matrix() const noexcept override { return m_mat; }
        };

        class PMVSyncN : public SyncMats4f {
          private:
            Mat4f* m_mat;
            size_t m_count;
            sync_action_t m_sync;

          public:
            PMVSyncN(Mat4f* m, size_t count, sync_action_t s) noexcept
            : m_mat(m), m_count(count), m_sync( std::move(s) )
            { }
            PMVSyncN(Mat4f* m, size_t count) noexcept
            : m_mat(m), m_count(count), m_sync( jau::bind_free(sync_action_fptr(nullptr)) )
            { }

            virtual sync_action_t& action() noexcept override { return m_sync; }
            virtual const Mat4f* matrices() const noexcept override { return m_mat; }
            virtual size_t matrixCount() const noexcept override { return m_count; }
        };

        Mat4f matP;
        Mat4f matMv;
        Mat4f matMvi;
        Mat4f matMvit;

        Mat4f matTex;

        Mat4fStack stackMv, stackP, stackTex;

        uint32_t requestBits; // may contain the requested bits: INVERSE_MODELVIEW | INVERSE_TRANSPOSED_MODELVIEW

        PMVSync1 syncP = PMVSync1(matP);
        PMVSync1 syncMv = PMVSync1(matMv);
        PMVSync1 syncTex = PMVSync1(matTex);

        PMVSync1 syncMvi = PMVSync1(matMvi, jau::bind_member(this, &PMVMat4f::updateImpl0));
        PMVSync1 syncMvit = PMVSync1(matMvit, jau::bind_member(this, &PMVMat4f::updateImpl0));

        PMVSyncN syncP_Mv = PMVSyncN(&matP, 2);
        PMVSyncN syncP_Mv_Mvi = PMVSyncN(&matP, 3, jau::bind_member(this, &PMVMat4f::updateImpl0));
        PMVSyncN syncP_Mv_Mvi_Mvit = PMVSyncN(&matP, 4, jau::bind_member(this, &PMVMat4f::updateImpl0));

        Mat4f mat4Tmp1;
        Mat4f mat4Tmp2;

        uint32_t modifiedBits = MODIFIED_ALL;
        uint32_t dirtyBits = 0; // contains the dirty bits, i.e. hinting for update operation
        Mat4f matPMv;
        Mat4f matPMvi;
        bool matPMviOK;
        geom::Frustum frustum;

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
     * Creates an instance of PMVMat4f.
     * <p>
     * This constructor only sets up an instance w/o additional {@link #INVERSE_MODELVIEW} or {@link #INVERSE_TRANSPOSED_MODELVIEW}.
     * </p>
     * <p>
     * Implementation uses non-direct non-NIO Buffers with guaranteed backing array,
     * which are synchronized to the actual Mat4f instances.
     * This allows faster access in Java computation.
     * </p>
     * @see #PMVMat4f(int)
     */
    PMVMat4f() noexcept
    : PMVMat4f(0) { }

    /**
     * Creates an instance of PMVMat4f.
     * <p>
     * Additional derived matrices can be requested via `derivedMatrices`, i.e.
     * - {@link #INVERSE_MODELVIEW}
     * - {@link #INVERSE_TRANSPOSED_MODELVIEW}
     * </p>
     * <p>
     * Implementation uses non-direct non-NIO Buffers with guaranteed backing array,
     * which are synchronized to the actual Mat4f instances.
     * This allows faster access in Java computation.
     * </p>
     * @param derivedMatrices additional matrices can be requested by passing bits {@link #INVERSE_MODELVIEW} and {@link #INVERSE_TRANSPOSED_MODELVIEW}.
     * @see #getReqBits()
     * @see #isReqDirty()
     * @see #getDirtyBits()
     * @see #update()
     */
    PMVMat4f(int derivedMatrices) noexcept
    : requestBits( matToReq(derivedMatrices) )
    {
        matPMviOK = false;
        reset();
    }

    /**
     * Issues {@link Mat4f#loadIdentity()} on all matrices and resets all internal states.
     */
    constexpr void reset() noexcept {
        matP.loadIdentity();
        matMv.loadIdentity();
        matTex.loadIdentity();

        modifiedBits = MODIFIED_ALL;
        dirtyBits = requestBits | MANUAL_BITS;
    }

    //
    // Regular Mat4f access as well as their SyncedBuffer counterpart SyncedMatrix and SyncedMatrices
    //

    /**
     * Returns the {@link GLMatrixFunc#GL_TEXTURE_MATRIX texture matrix} (T).
     * <p>
     * Consider using {@link #setTextureDirty()} if modifying the returned {@link Mat4f}.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4f& getT() noexcept { return matTex; }

    /**
     * Returns the {@link SyncMatrix} of {@link GLMatrixFunc#GL_TEXTURE_MATRIX texture matrix} (T).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMat4f& getSyncT() noexcept { return syncTex; }

    /**
     * Returns the {@link GLMatrixFunc#GL_PROJECTION_MATRIX projection matrix} (P).
     * <p>
     * Consider using {@link #setProjectionDirty()} if modifying the returned {@link Mat4f}.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4f& getP() noexcept { return matP; }

    /**
     * Returns the {@link SyncMatrix} of {@link GLMatrixFunc#GL_PROJECTION_MATRIX projection matrix} (P).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMat4f& getSyncP() noexcept { return syncP; }

    /**
     * Returns the {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mv).
     * <p>
     * Consider using {@link #setModelviewDirty()} if modifying the returned {@link Mat4f}.
     * </p>
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr Mat4f& getMv() noexcept { return matMv; }

    /**
     * Returns the {@link SyncMatrix} of {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mv).
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMat4f& getSyncMv() noexcept { return syncMv; }

    /**
     * Returns {@link SyncMatrices4f} of 2 matrices within one FloatBuffer: {@link #getP() P} and {@link #getMv() Mv}.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     */
    constexpr SyncMats4f& getSyncPMv() noexcept { return syncP_Mv; }

    /**
     * Returns the inverse {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvi) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_MODELVIEW} has not been requested in ctor {@link #PMVMat4f(int)}.
     */
    Mat4f& getMvi() {
        if( 0 == ( INVERSE_MODELVIEW & requestBits ) ) { // FIXME
            throw jau::IllegalArgumentException("Not requested in ctor", E_FILE_LINE);
        }
        updateImpl(false);
        return matMvi;
    }

    /**
     * Returns the {@link SyncMatrix} of inverse {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvi) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_MODELVIEW} has not been requested in ctor {@link #PMVMat4f(int)}.
     */
    SyncMat4f& getSyncMvi() {
        if( 0 == ( INVERSE_MODELVIEW & requestBits ) ) { // FIXME
            throw jau::IllegalArgumentException("Not requested in ctor", E_FILE_LINE);
        }
        return syncMvi;
    }

    /**
     * Returns the inverse transposed {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvit) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_TRANSPOSED_MODELVIEW} has not been requested in ctor {@link #PMVMat4f(int)}.
     */
    Mat4f& getMvit() {
        if( 0 == ( INVERSE_TRANSPOSED_MODELVIEW & requestBits ) ) { // FIXME
            throw jau::IllegalArgumentException("Not requested in ctor", E_FILE_LINE);
        }
        updateImpl(false);
        return matMvit;
    }

    /**
     * Returns the {@link SyncMatrix} of inverse transposed {@link GLMatrixFunc#GL_MODELVIEW_MATRIX modelview matrix} (Mvit) if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_TRANSPOSED_MODELVIEW} has not been requested in ctor {@link #PMVMat4f(int)}.
     */
    SyncMat4f& getSyncMvit() {
        if( 0 == ( INVERSE_TRANSPOSED_MODELVIEW & requestBits ) ) { // FIXME
            throw jau::IllegalArgumentException("Not requested in ctor", E_FILE_LINE);
        }
        return syncMvit;
    }

    /**
     * Returns {@link SyncMatrices4f} of 3 matrices within one FloatBuffer: {@link #getP() P}, {@link #getMv() Mv} and {@link #getMvi() Mvi} if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_MODELVIEW} has not been requested in ctor {@link #PMVMat4f(int)}.
     */
    SyncMats4f& getSyncPMvMvi() {
        if( 0 == ( INVERSE_MODELVIEW & requestBits ) ) { // FIXME
            throw jau::IllegalArgumentException("Not requested in ctor", E_FILE_LINE);
        }
        return syncP_Mv_Mvi;
    }

    /**
     * Returns {@link SyncMatrices4f} of 4 matrices within one FloatBuffer: {@link #getP() P}, {@link #getMv() Mv}, {@link #getMvi() Mvi} and {@link #getMvit() Mvit} if requested.
     * <p>
     * See <a href="#storageDetails"> matrix storage details</a>.
     * </p>
     * @throws IllegalArgumentException if {@link #INVERSE_TRANSPOSED_MODELVIEW} has not been requested in ctor {@link #PMVMat4f(int)}.
     */
    SyncMats4f& getSyncPMvMviMvit() {
        if( 0 == ( INVERSE_TRANSPOSED_MODELVIEW & requestBits ) ) { // FIXME
            throw jau::IllegalArgumentException("Not requested in ctor", E_FILE_LINE);
        }
        return syncP_Mv_Mvi_Mvit;
    }

    //
    // Basic Mat4f, Vec3f and Vec4f operations similar to GLMatrixFunc
    //

    /**
     * Returns multiplication result of {@link #getP() P} and {@link #getMv() Mv} matrix, i.e.
     * <pre>
     *    result = P x Mv
     * </pre>
     * @param result 4x4 matrix storage for result
     * @return given result matrix for chaining
     */
    constexpr Mat4f& getMulPMv(Mat4f& result) noexcept {
        return result.mul(matP, matMv);
    }

    /**
     * Returns multiplication result of {@link #getMv() Mv} and {@link #getP() P} matrix, i.e.
     * <pre>
     *    result = Mv x P
     * </pre>
     * @param result 4x4 matrix storage for result
     * @return given result matrix for chaining
     */
    constexpr Mat4f& getMulMvP(Mat4f& result) noexcept {
        return result.mul(matMv, matP);
    }

    /**
     * v_out = Mv * v_in
     * @param v_in input vector, can be v_out for in-place transformation
     * @param v_out output vector
     * @returns v_out for chaining
     */
    constexpr Vec4f& mulWithMv(const Vec4f& v_in, Vec4f& v_out) noexcept {
        return matMv.mulVec4(v_in, v_out);
    }

    /**
     * v_inout = Mv * v_inout
     * @param v_inout input and output vector, i.e. in-place transformation
     * @returns v_inout for chaining
     */
    constexpr Vec4f& mulWithMv(Vec4f& v_inout) noexcept {
        return matMv.mulVec4(v_inout);
    }

    /**
     * v_out = Mv * v_in
     *
     * Affine 3f-vector transformation by 4x4 matrix, see {@link Mat4f#mulVec3f(Vec3f, Vec3f)}.
     *
     * @param v_in input vector, can be v_out for in-place transformation
     * @param v_out output vector
     * @returns v_out for chaining
     */
    constexpr Vec3f& mulWithMv(const Vec3f& v_in, Vec3f& v_out) noexcept {
        return matMv.mulVec3(v_in, v_out);
    }

    //
    // GLMatrixFunc alike functionality
    //

    /**
     * Load the {@link #getMv() modelview matrix} with the provided values.
     */
    constexpr PMVMat4f& loadMv(float values[]) noexcept {
        matMv.load(values);
        setModelviewDirty();
        return *this;
    }
    /**
     * Load the {@link #getMv() modelview matrix} with the values of the given {@link Mat4f}.
     */
    constexpr PMVMat4f& loadMv(const Mat4f& m) noexcept {
        matMv.load(m);
        setModelviewDirty();
        return *this;
    }
    /**
     * Load the {@link #getMv() modelview matrix} with the values of the given {@link Quaternion}'s rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMat4f& loadMv(const Quat4f& quat) noexcept {
        quat.toMatrix(matMv);
        setModelviewDirty();
        return *this;
    }

    /**
     * Load the {@link #getP() projection matrix} with the provided values.
     */
    constexpr PMVMat4f& loadP(float values[]) noexcept {
        matP.load(values);
        setProjectionDirty();
        return *this;
    }
    /**
     * Load the {@link #getP() projection matrix} with the values of the given {@link Mat4f}.
     */
    constexpr PMVMat4f& loadP(const Mat4f& m) {
        matP.load(m);
        setProjectionDirty();
        return *this;
    }
    /**
     * Load the {@link #getP() projection matrix} with the values of the given {@link Quaternion}'s rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMat4f& loadP(const Quat4f& quat) noexcept {
        quat.toMatrix(matP);
        setProjectionDirty();
        return *this;
    }

    /**
     * Load the {@link #getT() texture matrix} with the provided values.
     */
    constexpr PMVMat4f& loadT(float values[]) noexcept {
        matTex.load(values);
        setTextureDirty();
        return *this;
    }
    /**
     * Load the {@link #getT() texture matrix} with the values of the given {@link Mat4f}.
     */
    constexpr PMVMat4f& loadT(Mat4f& m) noexcept {
        matTex.load(m);
        setTextureDirty();
        return *this;
    }
    /**
     * Load the {@link #getT() texture matrix} with the values of the given {@link Quaternion}'s rotation Quaternion::toMatrix() representation.
     */
    constexpr PMVMat4f& loadT(const Quat4f& quat) noexcept {
        quat.toMatrix(matTex);
        setTextureDirty();
        return *this;
    }

    /**
     * Load the {@link #getMv() modelview matrix} with the values of the given {@link Mat4f}.
     */
    constexpr PMVMat4f& loadMvIdentity() noexcept {
        matMv.loadIdentity();
        setModelviewDirty();
        return *this;
    }

    /**
     * Load the {@link #getP() projection matrix} with the values of the given {@link Mat4f}.
     */
    constexpr PMVMat4f& loadPIdentity() noexcept {
        matP.loadIdentity();
        setProjectionDirty();
        return *this;
    }

    /**
     * Load the {@link #getT() texture matrix} with the values of the given {@link Mat4f}.
     */
    constexpr PMVMat4f& loadTIdentity() noexcept {
        matTex.loadIdentity();
        setTextureDirty();
        return *this;
    }

    /**
     * Multiply the {@link #getMv() modelview matrix}: [c] = [c] x [m]
     * @param m the right hand Mat4f
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& mulMv(const Mat4f& m) noexcept {
        matMv.mul( m );
        setModelviewDirty();
        return *this;
    }

    /**
     * Multiply the {@link #getP() projection matrix}: [c] = [c] x [m]
     * @param m the right hand Mat4f
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& mulP(const Mat4f& m) noexcept {
        matP.mul( m );
        setProjectionDirty();
        return *this;
    }

    /**
     * Multiply the {@link #getT() texture matrix}: [c] = [c] x [m]
     * @param m the right hand Mat4f
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& mulT(const Mat4f& m) noexcept {
        matTex.mul( m );
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
    constexpr PMVMat4f& translateMv(float x, float y, float z) noexcept {
        return mulMv( mat4Tmp1.setToTranslation(x, y, z) );
    }
    /**
     * Translate the {@link #getMv() modelview matrix}.
     * @param t translation vec3
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& translateMv(const Vec3f& t) noexcept {
        return mulMv( mat4Tmp1.setToTranslation(t) );
    }

    /**
     * Translate the {@link #getP() projection matrix}.
     * @param x
     * @param y
     * @param z
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& translateP(float x, float y, float z) noexcept {
        return mulP( mat4Tmp1.setToTranslation(x, y, z) );
    }
    /**
     * Translate the {@link #getP() projection matrix}.
     * @param t translation vec3
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& translateP(const Vec3f& t) noexcept {
        return mulP( mat4Tmp1.setToTranslation(t) );
    }

    /**
     * Scale the {@link #getMv() modelview matrix}.
     * @param x
     * @param y
     * @param z
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& scaleMv(float x, float y, float z) noexcept {
        return mulMv( mat4Tmp1.setToScale(x, y, z) );
    }
    /**
     * Scale the {@link #getMv() modelview matrix}.
     * @param s scale vec4f
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& scaleMv(const Vec3f& s) noexcept {
        return mulMv( mat4Tmp1.setToScale(s) );
    }

    /**
     * Scale the {@link #getP() projection matrix}.
     * @param x
     * @param y
     * @param z
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& scaleP(float x, float y, float z) noexcept {
        return mulP( mat4Tmp1.setToScale(x, y, z) );
    }
    /**
     * Scale the {@link #getP() projection matrix}.
     * @param s scale vec4f
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& scaleP(const Vec3f& s) noexcept {
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
    constexpr_cxx26 PMVMat4f& rotateMv(const float ang_rad, const float x, const float y, const float z) noexcept {
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
    constexpr_cxx26 PMVMat4f& rotateMv(const float ang_rad, const Vec3f& axis) noexcept {
        return mulMv( mat4Tmp1.setToRotationAxis(ang_rad, axis) );
    }
    /**
     * Rotate the {@link #getMv() modelview matrix} with the given {@link Quaternion}'s rotation {@link Mat4f#setToRotation(Quaternion) matrix representation}.
     * @param quat the {@link Quaternion}
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& rotateMv(const Quat4f& quat) noexcept {
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
    constexpr_cxx26 PMVMat4f& rotateP(const float ang_rad, const float x, const float y, const float z) noexcept {
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
    constexpr_cxx26 PMVMat4f& rotateP(const float ang_rad, const Vec3f& axis) noexcept {
        return mulP( mat4Tmp1.setToRotationAxis(ang_rad, axis) );
    }
    /**
     * Rotate the {@link #getP() projection matrix} with the given {@link Quaternion}'s rotation {@link Mat4f#setToRotation(Quaternion) matrix representation}.
     * @param quat the {@link Quaternion}
     * @return *this instance of chaining
     */
    constexpr PMVMat4f& rotateP(const Quat4f& quat) noexcept {
        return mulP( quat.toMatrix(mat4Tmp1) );
    }

    /** Pop the {@link #getMv() modelview matrix} from its stack. */
    constexpr_cxx20 PMVMat4f& popMv() noexcept {
        stackMv.pop(matMv);
        setModelviewDirty();
        return *this;
    }
    /** Pop the {@link #getP() projection matrix} from its stack. */
    constexpr_cxx20 PMVMat4f& popP() noexcept {
        stackP.pop(matP);
        setProjectionDirty();
        return *this;
    }
    /** Pop the {@link #getT() texture matrix} from its stack. */
    constexpr_cxx20 PMVMat4f& popT() noexcept {
        stackTex.pop(matTex);
        setTextureDirty();
        return *this;
    }
    /** Push the {@link #getMv() modelview matrix} to its stack, while preserving its values. */
    constexpr_cxx20 PMVMat4f& pushMv() noexcept {
        stackMv.push(matMv);
        return *this;
    }
    /** Push the {@link #getP() projection matrix} to its stack, while preserving its values. */
    constexpr_cxx20 PMVMat4f& pushP() noexcept {
        stackP.push(matP);
        return *this;
    }
    /** Push the {@link #getT() texture matrix} to its stack, while preserving its values. */
    constexpr_cxx20 PMVMat4f& pushT() noexcept {
        stackTex.push(matTex);
        return *this;
    }

    /**
     * {@link #mulP(Mat4f) Multiply} the {@link #getP() projection matrix} with the orthogonal matrix.
     * @param left
     * @param right
     * @param bottom
     * @param top
     * @param zNear
     * @param zFar
     * @see Mat4f#setToOrtho(float, float, float, float, float, float)
     */
    constexpr void orthoP(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) noexcept {
        mulP( mat4Tmp1.setToOrtho(left, right, bottom, top, zNear, zFar) );
    }

    /**
     * {@link #mulP(Mat4f) Multiply} the {@link #getP() projection matrix} with the frustum matrix.
     *
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     *                          or {@code left == right}, or {@code bottom == top}.
     * @see Mat4f#setToFrustum(float, float, float, float, float, float)
     */
    void frustumP(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
        mulP( mat4Tmp1.setToFrustum(left, right, bottom, top, zNear, zFar) );
    }

    //
    // Extra functionality
    //

    /**
     * {@link #mulP(Mat4f) Multiply} the {@link #getP() projection matrix} with the perspective/frustum matrix.
     *
     * @param fovy_rad fov angle in radians
     * @param aspect aspect ratio width / height
     * @param zNear
     * @param zFar
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see Mat4f#setToPerspective(float, float, float, float)
     */
    PMVMat4f& perspectiveP(const float fovy_rad, const float aspect, const float zNear, const float zFar) {
        mulP( mat4Tmp1.setToPerspective(fovy_rad, aspect, zNear, zFar) );
        return *this;
    }

    /**
     * {@link #mulP(Mat4f) Multiply} the {@link #getP() projection matrix}
     * with the eye, object and orientation, i.e. {@link Mat4f#setToLookAt(Vec3f, Vec3f, Vec3f, Mat4f)}.
     */
    constexpr PMVMat4f& lookAtP(const Vec3f& eye, const Vec3f& center, const Vec3f& up) noexcept {
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
    bool mapObjToWin(const Vec3f& objPos, const Recti& viewport, Vec3f& winPos) noexcept {
        return Mat4f::mapObjToWin(objPos, matMv, matP, viewport, winPos);
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
                     const Recti& viewport, Vec3f& objPos) noexcept {
        if( Mat4f::mapWinToObj(winx, winy, winz, getPMvi(), viewport, objPos) ) {
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
                      const Recti& viewport, const float near, const float far, Vec4f& objPos) noexcept {
        if( Mat4f::mapWinToObj4(winx, winy, winz, clipw, getPMvi(), viewport, near, far, objPos) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a {@link Ray}. The resulting {@link Ray} maybe used for <i>picking</i>
     * using a {@link AABBox#getRayIntersection(Vec3f, Ray, float, bool) bounding box}.
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
        return Mat4f::mapWinToRay(winx, winy, winz0, winz1, getPMvi(), viewport, ray);
    }

    std::string& toString(std::string& sb, const std::string& f) const noexcept {
        const bool pmvDirty  = 0 != (PREMUL_PMV & dirtyBits);
        const bool pmvUsed = true; // null != matPMv;

        const bool pmviDirty  = 0 != (PREMUL_PMVI & dirtyBits);
        const bool pmviUsed = true; // null != matPMvi;

        const bool frustumDirty = 0 != (FRUSTUM & dirtyBits);
        const bool frustumUsed = true; // null != frustum;

        const bool mviDirty  = 0 != (INVERSE_MODELVIEW & dirtyBits);
        const bool mviReq = 0 != (INVERSE_MODELVIEW & requestBits);

        const bool mvitDirty = 0 != (INVERSE_TRANSPOSED_MODELVIEW & dirtyBits);
        const bool mvitReq = 0 != (INVERSE_TRANSPOSED_MODELVIEW & requestBits);

        const bool modP = 0 != ( MODIFIED_PROJECTION & modifiedBits );
        const bool modMv = 0 != ( MODIFIED_MODELVIEW & modifiedBits );
        const bool modT = 0 != ( MODIFIED_TEXTURE & modifiedBits );
        int count = 3; // P, Mv, T

        sb.append("PMVMat4f[modified[P ").append(std::to_string(modP)).append(", Mv ").append(std::to_string(modMv)).append(", T ").append(std::to_string(modT));
        sb.append("], dirty/used[PMv ").append(std::to_string(pmvDirty)).append("/").append(std::to_string(pmvUsed))
          .append(", Pmvi ").append(std::to_string(pmviDirty)).append("/").append(std::to_string(pmviUsed))
          .append(", Frustum ").append(std::to_string(frustumDirty)).append("/").append(std::to_string(frustumUsed));
        sb.append("], dirty/req[Mvi ").append(std::to_string(mviDirty)).append("/").append(std::to_string(mviReq))
          .append(", Mvit ").append(std::to_string(mvitDirty)).append("/").append(std::to_string(mvitReq)).append("]\n");
        sb.append(", Projection\n");
        matP.toString(sb, f);
        sb.append(", Modelview\n");
        matMv.toString(sb, f);
        sb.append(", Texture\n");
        matTex.toString(sb, f);
        if( pmvUsed ) {
            sb.append(", P * Mv\n");
            matPMv.toString(sb, f);
            ++count;
        }
        if( pmviUsed ) {
            sb.append(", P * Mv\n");
            matPMvi.toString(sb, f);
            ++count;
        }
        if( mviReq ) {
            sb.append(", Inverse Modelview\n");
            matMvi.toString(sb, f);
            ++count;
        }
        if( mvitReq ) {
            sb.append(", Inverse Transposed Modelview\n");
            matMvit.toString(sb, f);
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
        const uint32_t r = modifiedBits;
        if(clear) {
            modifiedBits = 0;
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
     * since last {@link #update()} call and requested in the constructor {@link #PMVMat4f(int)}.
     * </p>
     * <p>
     * {@link #update()} clears the dirty state for the matrices and {@link #getFrustum()} for {@link #FRUSTUM}.
     * </p>
     *
     * @see #isReqDirty()
     * @see #INVERSE_MODELVIEW
     * @see #INVERSE_TRANSPOSED_MODELVIEW
     * @see #FRUSTUM
     * @see #PMVMat4f(int)
     * @see #getMvi()
     * @see #getMvit()
     * @see #getSyncPMvMvi()
     * @see #getSyncPMvMviMvit()
     * @see #getFrustum()
     */
    constexpr uint32_t getDirtyBits() noexcept {
        return dirtyBits;
    }

    /**
     * Returns true if the one of the {@link #getReqBits() requested bits} are are set dirty due to mutable operations,
     * i.e. at least one of
     * - {@link #INVERSE_MODELVIEW}
     * - {@link #INVERSE_TRANSPOSED_MODELVIEW}
     * <p>
     * A dirty bit is set, if the corresponding matrix had been modified by a mutable operation
     * since last {@link #update()} call and requested in the constructor {@link #PMVMat4f(int)}.
     * </p>
     * <p>
     * {@link #update()} clears the dirty state for the matrices and {@link #getFrustum()} for {@link #FRUSTUM}.
     * </p>
     *
     * @see #INVERSE_MODELVIEW
     * @see #INVERSE_TRANSPOSED_MODELVIEW
     * @see #PMVMat4f(int)
     * @see #getMvi()
     * @see #getMvit()
     * @see #getSyncPMvMvi()
     * @see #getSyncPMvMviMvit()
     */
    constexpr bool isReqDirty() noexcept {
        return 0 != ( requestBits & dirtyBits );
    }

    /**
     * Sets the {@link #getMv() Modelview (Mv)} matrix dirty and modified,
     * i.e. adds {@link #getReqBits() requested bits} and {@link #MANUAL_BITS} to {@link #getDirtyBits() dirty bits}.
     * @see #isReqDirty()
     */
    constexpr void setModelviewDirty() noexcept {
        dirtyBits |= requestBits | MANUAL_BITS ;
        modifiedBits |= MODIFIED_MODELVIEW;
    }

    /**
     * Sets the {@link #getP() Projection (P)} matrix dirty and modified,
     * i.e. adds {@link #MANUAL_BITS} to {@link #getDirtyBits() dirty bits}.
     */
    constexpr void setProjectionDirty() noexcept {
        dirtyBits |= MANUAL_BITS ;
        modifiedBits |= MODIFIED_PROJECTION;
    }

    /**
     * Sets the {@link #getT() Texture (T)} matrix modified.
     */
    constexpr void setTextureDirty() noexcept {
        modifiedBits |= MODIFIED_TEXTURE;
    }

    /**
     * Returns the request bit mask, which uses bit values equal to the dirty mask
     * and may contain
     * - {@link #INVERSE_MODELVIEW}
     * - {@link #INVERSE_TRANSPOSED_MODELVIEW}
     * <p>
     * The request bit mask is set by in the constructor {@link #PMVMat4f(int)}.
     * </p>
     *
     * @see #INVERSE_MODELVIEW
     * @see #INVERSE_TRANSPOSED_MODELVIEW
     * @see #PMVMat4f(int)
     * @see #getMvi()
     * @see #getMvit()
     * @see #getSyncPMvMvi()
     * @see #getSyncPMvMviMvit()
     * @see #getFrustum()
     */
    constexpr uint32_t getReqBits() noexcept {
        return requestBits;
    }

    /**
     * Returns the pre-multiplied projection x modelview, P x Mv.
     * <p>
     * This {@link Mat4f} instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * {@link #update()} has no effect on this {@link Mat4f}.
     * </p>
     * <p>
     * This pre-multipled P x Mv is considered dirty, if its corresponding
     * {@link #getP() P matrix} or {@link #getMv() Mv matrix} has been modified since its last update.
     * </p>
     * @see #update()
     */
    constexpr Mat4f& getPMv() noexcept {
        if( 0 != ( dirtyBits & PREMUL_PMV ) ) {
            /* if( null == matPMv ) { // FIXME
                matPMv = new Mat4f();
            } */
            matPMv.mul(matP, matMv);
            dirtyBits &= ~PREMUL_PMV;
        }
        return matPMv;
    }

    /**
     * Returns the pre-multiplied inverse projection x modelview,
     * if {@link Mat4f#invert(Mat4f)} succeeded, otherwise `null`.
     * <p>
     * This {@link Mat4f} instance should be re-fetched via this method and not locally stored
     * to have it updated from a potential modification of underlying projection and/or modelview matrix.
     * {@link #update()} has no effect on this {@link Mat4f}.
     * </p>
     * <p>
     * This pre-multipled invert(P x Mv) is considered dirty, if its corresponding
     * {@link #getP() P matrix} or {@link #getMv() Mv matrix} has been modified since its last update.
     * </p>
     * @see #update()
     */
    constexpr Mat4f& getPMvi() noexcept {
        if( 0 != ( dirtyBits & PREMUL_PMVI ) ) {
            /* if( null == matPMvi ) { // FIXME
                matPMvi = new Mat4f();
            } */
            Mat4f& mPMv = getPMv();
            matPMviOK = matPMvi.invert(mPMv);
            dirtyBits &= ~PREMUL_PMVI;
        }
        return matPMvi; // matPMviOK ? matPMvi : null; // FIXME
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
        if( 0 != ( dirtyBits & FRUSTUM ) ) {
            /* if( null == frustum ) { // FIXME
                frustum = new Frustum();
            } */
            frustum.setFromMat(getPMv());
            dirtyBits &= ~FRUSTUM;
        }
        return frustum;
    }

    /**
     * Update the derived {@link #getMvi() inverse modelview (Mvi)},
     * {@link #getMvit() inverse transposed modelview (Mvit)} matrices
     * <b>if</b> they {@link #isReqDirty() are dirty} <b>and</b>
     * requested via the constructor {@link #PMVMat4f(int)}.<br/>
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
     * Method is automatically called by {@link SyncMat4f} and {@link SyncMatrices4f}
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
     * @see #PMVMat4f(int)
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
        bool mod = 0 != modifiedBits;
        if( clearModBits ) {
            modifiedBits = 0;
        }
        if( 0 != ( requestBits & ( ( dirtyBits & ( INVERSE_MODELVIEW | INVERSE_TRANSPOSED_MODELVIEW ) ) ) ) ) { // only if dirt requested & dirty
            if( !matMvi.invert(matMv) ) {
                throw jau::RuntimeException("Invalid source Mv matrix, can't compute inverse", E_FILE_LINE);
            }
            dirtyBits &= ~INVERSE_MODELVIEW;
            mod = true;
        }
        if( 0 != ( requestBits & ( dirtyBits & INVERSE_TRANSPOSED_MODELVIEW ) ) ) { // only if requested & dirty
            matMvit.transpose(matMvi);
            dirtyBits &= ~INVERSE_TRANSPOSED_MODELVIEW;
            mod = true;
        }
        return mod;
    }
};

inline std::ostream& operator<<(std::ostream& out, const PMVMat4f& v) noexcept {
    return out << v.toString();
}

 /**@}*/

 } // namespace jau::math::util

 #endif // JAU_PMVMAT4f_HPP_
