/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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
package org.jau.lang;

import java.lang.reflect.InaccessibleObjectException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import org.jau.sys.Debug;

/**
 * Utility methods to simplify reflection access
 */
public class ReflectionUtil {
    static final boolean DEBUG = Debug.debug("Reflection");

    private static final String asString(final Class<?>[] argTypes) {
        final StringBuilder args = new StringBuilder();
        if(null != argTypes) {
            for (int i = 0; i < argTypes.length; i++) {
                if(i > 0) {
                     args.append(", ");
                }
                args.append(argTypes[i].getName());
            }
        }
        return args.toString();
    }

    private ReflectionUtil() {}

    /**
     * Returns true only if the class could be loaded but w/o class initialization.
     */
    public static boolean isClassAvailable(final String clazzName, final ClassLoader cl) {
        try {
            return null != Class.forName(clazzName, false /* initializeClazz */, cl);
        /**
        } catch( final ClassNotFoundException cnfe ) {
            return false;
        } catch( final ExceptionInInitializerError eiie ) {
            return false;
        } catch( final LinkageError le ) {
            return false;
        */
        } catch( final Throwable t ) {
            return false;
        }
    }

    /**
     * Loads and returns the class or null.
     * @see Class#forName(java.lang.String, boolean, java.lang.ClassLoader)
     */
    public static final Class<?> getClass(final String clazzName, final boolean initializeClazz, final ClassLoader cl)
        throws RuntimeException {
        try {
            return Class.forName(clazzName, initializeClazz, cl);
        } catch( final Throwable t ) {
            throw new RuntimeException(clazzName + " not available", t);
        }
    }

  /**
   * @throws RuntimeException if the Method can not be found.
   */
  public static final Method getMethod(final Class<?> clazz, final String methodName, final Class<?> ... argTypes)
      throws RuntimeException
  {
    try {
        return clazz.getDeclaredMethod(methodName, argTypes);
    } catch (final Throwable t) {
        throw new RuntimeException("Method: '" + clazz + "." + methodName + "(" + asString(argTypes) + ")' not found", t);
    }
  }

  /**
   * @throws RuntimeException if the Method can not be found.
   */
  public static final Method getMethod(final String clazzName, final boolean initializeClazz, final String methodName, final Class<?>[] argTypes, final ClassLoader cl)
      throws RuntimeException
  {
    try {
        return getMethod(Class.forName(clazzName, initializeClazz, cl), methodName, argTypes);
    } catch (final Throwable t) {
        throw new RuntimeException("Method: '" + clazzName + "." + methodName + "(" + asString(argTypes) + ")' not found", t);
    }
  }

  /**
   * @param instance may be null in case of a static method
   * @param method the method to be called
   * @param args the method arguments
   * @return the methods result, maybe null if void
   * @throws RuntimeException if call fails
   */
  @SuppressWarnings("unchecked")
  public static final <R> R callMethod(final Object instance, final Method method, final Object ... args)
      throws RuntimeException
  {
    try {
        return (R)method.invoke(instance, args);
    } catch (final Exception e) {
      Throwable t = e;
      if (t instanceof InvocationTargetException) {
        t = ((InvocationTargetException) t).getTargetException();
      }
      if (t instanceof Error) {
        throw (Error) t;
      }
      if (t instanceof RuntimeException) {
        throw (RuntimeException) t;
      }
      throw new RuntimeException("calling "+method+" failed", t);
    }
  }

  /**
   * @param method the method to be called
   * @param args the method arguments
   * @return the methods result, maybe null if void
   * @throws RuntimeException if call fails
   */
  public static final <R> R callStaticMethod(final Method method, final Object ... args)
      throws RuntimeException
  {
      return callMethod(null, method, args);
  }

  /**
   * @throws RuntimeException if the instance can not be created.
   */
  public static final <R> R callStaticMethod(final String clazzName, final String methodName, final Class<?>[] argTypes, final Object[] args, final ClassLoader cl)
      throws RuntimeException
  {
    return callStaticMethod(getMethod(clazzName, true /* initializeClazz */, methodName, argTypes, cl), args);
  }

  /** Convenient Method access class */
  public static class MethodAccessor {
    Method m = null;

    /** Check {@link #available()} before using instance. */
    public MethodAccessor(final Class<?> clazz, final String methodName, final Class<?> ... argTypes) {
        try {
            m = ReflectionUtil.getMethod(clazz, methodName, argTypes);
        } catch (final RuntimeException jre) { /* method n/a */ }
    }

    /** Returns true if method is available, otherwise false. */
    public boolean available() {
        return null != m;
    }

    /**
     * See {@link Method#setAccessible(boolean)}.
     * @param flag new accessible flag value
     * @throws InaccessibleObjectException
     * @throws SecurityException
     */
    public void setAccessible(final boolean flag)
            throws InaccessibleObjectException, SecurityException
    {
        m.setAccessible(flag);
    }

    /**
     * See {@link #setAccessible(boolean)}.
     * @param flag new accessible flag value
     * @return true if successful, otherwise false in case {@link Method#setAccessible(boolean)} threw an exception.
     */
    public boolean setAccessibleSafe(final boolean flag) {
        try {
            m.setAccessible(flag);
            return true;
        } catch( final Throwable t ) {
            return false;
        }
    }

    /**
     * Check {@link #available()} before calling to avoid throwing a RuntimeException.
     * @param instance may be null in case of a static method
     * @param args the method arguments
     * @return the methods result, maybe null if void
     * @throws RuntimeException if call fails or method not {@link #available()}.
     */
    public <R> R callMethod(final Object instance, final Object ... args) {
        if(null == m) {
            throw new RuntimeException("Method not available. Instance: "+instance);
        }
        return ReflectionUtil.callMethod(instance, m, args);
    }

    /**
     * Check {@link #available()} before calling to avoid throwing a RuntimeException.
     * @param args the method arguments
     * @return the methods result, maybe null if void
     * @throws RuntimeException if call fails or method not {@link #available()}.
     */
    public <R> R callStaticMethod(final Object ... args) {
        if(null == m) {
            throw new RuntimeException("Method not available.");
        }
        return ReflectionUtil.callStaticMethod(m, args);
    }
  }

}
