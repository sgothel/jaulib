#include <jni.h>

#include <cassert>

#include "jau/jni/helper_jni.hpp"
#include "org_jau_pkg_JarUtil.h"

#if defined(__APPLE__)
    #include <sys/xattr.h>
    static const char kQuarantineAttrName[] = "com.apple.quarantine";
#endif

/*
 * Class:     com_org_jau_pkg_JarUtil
 * Method:    fixNativeLibAttribs
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_org_jau_pkg_JarUtil_fixNativeLibAttribs(JNIEnv* env, jclass _unused, jstring fname) {
    (void)_unused;

    const char* _UTF8fname = nullptr;
    int status = 0;
    if (fname != nullptr) {
        _UTF8fname = env->GetStringUTFChars(fname, (jboolean*)nullptr);
        if (_UTF8fname == nullptr) {
            env->ThrowNew(env->FindClass("java/lang/OutOfMemoryError"),
                          R"(Failed to get UTF-8 chars for argument "fname" in native dispatcher for "removexattr")");
            return 0;
        }
    }
#if defined(__APPLE__)
    status = removexattr(_UTF8fname, kQuarantineAttrName, 0);
#endif
    if (fname != nullptr) {
        env->ReleaseStringUTFChars(fname, _UTF8fname);
    }
    return 0 == status ? JNI_TRUE : JNI_FALSE;
}
