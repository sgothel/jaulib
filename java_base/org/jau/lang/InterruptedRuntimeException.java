/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2015 Gothel Software e.K.
 * Copyright (c) 2015 JogAmp Community.
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

/**
 * <i>Unchecked exception</i> propagating an {@link InterruptedException}
 * where handling of the latter is not desired.
 * <p>
 * {@link InterruptedRuntimeException} may be thrown either by waiting for any {@link Runnable}
 * to be completed, or during its execution.
 * </p>
 * <p>
 * The propagated {@link InterruptedException} may be of type {@link SourcedInterruptedException}.
 * </p>
 * <p>
 * </p>
 */
@SuppressWarnings("serial")
public class InterruptedRuntimeException extends RuntimeException {

  /**
   * Constructor attempts to {@link SourcedInterruptedException#wrap(InterruptedException) wrap}
   * the given {@link InterruptedException} {@code cause} into a {@link SourcedInterruptedException}.
   *
   * @param message the message of this exception
   * @param cause the propagated {@link InterruptedException}
   */
  public InterruptedRuntimeException(final String message, final InterruptedException cause) {
    super(message, SourcedInterruptedException.wrap(cause));
  }

  /**
   * Constructor attempts to {@link SourcedInterruptedException#wrap(InterruptedException) wrap}
   * the given {@link InterruptedException} {@code cause} into a {@link SourcedInterruptedException}.
   *
   * @param cause the propagated {@link InterruptedException}
   */
  public InterruptedRuntimeException(final InterruptedException cause) {
    super(SourcedInterruptedException.wrap(cause));
  }

  /**
   * Returns the propagated {@link InterruptedException}, i.e. the cause of this exception.
   * <p>
   * {@inheritDoc}
   * </p>
   */
  @Override
  public InterruptedException getCause() {
      return (InterruptedException)super.getCause();
  }
}
