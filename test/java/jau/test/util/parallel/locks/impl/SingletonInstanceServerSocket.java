/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2010 Gothel Software e.K.
 * Copyright (c) 2010 JogAmp Community.
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

package jau.test.util.parallel.locks.impl;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;

import org.jau.lang.ExceptionUtils;
import org.jau.lang.InterruptSource;
import org.jau.lang.InterruptedRuntimeException;
import org.jau.lang.SourcedInterruptedException;

import jau.test.util.parallel.locks.SingletonInstance;

public class SingletonInstanceServerSocket extends SingletonInstance {

    private static int serverInstanceCount = 0;
    private final Server singletonServer;
    private final String fullName;

    public SingletonInstanceServerSocket(final long poll_ms, final int portNumber) {
        super(poll_ms);

        // Gather the local InetAddress, loopback is prioritized
        InetAddress ilh = null;
        try {
            ilh = InetAddress.getByName(null); // loopback
        } catch (final UnknownHostException e1) { }
        if(null == ilh) {
            try {
                ilh = InetAddress.getByName("localhost");
                if(null!=ilh && !ilh.isLoopbackAddress()) { ilh = null; }
            } catch (final UnknownHostException e1) { }
        }
        if(null == ilh) {
            try {
                ilh = InetAddress.getByAddress(new byte[] { 127, 0, 0, 1 } );
                if(null!=ilh && !ilh.isLoopbackAddress()) { ilh = null; }
            } catch (final UnknownHostException e) { }
        }
        if(null == ilh) {
            try {
                ilh = InetAddress.getLocalHost();
            } catch (final UnknownHostException e) { }
        }
        if(null == ilh) {
            throw new RuntimeException(infoPrefix()+" EEE Could not determine local InetAddress");
        }

        fullName = ilh.toString()+":"+portNumber;
        singletonServer = new Server(ilh, portNumber);
        Runtime.getRuntime().addShutdownHook(new InterruptSource.Thread() {
            @Override
            public void run() {
                singletonServer.kill();
            }
        });
    }

    public final InetAddress getLocalInetAddress() {
        return singletonServer.getLocalInetAddress();
    }

    public final int getPortNumber() {
        return singletonServer.getPortNumber();
    }

    @Override
    public final String getName() { return fullName; }

    @Override
    protected boolean tryLockImpl() {
        if( singletonServer.isRunning() ) {
            return false; // same JVM .. server socket already installed !
        }

        // check if other JVM's locked the server socket ..
        final Socket clientSocket = singletonServer.connect();
        if(null != clientSocket) {
            try {
                clientSocket.close();
            } catch (final IOException e) { }
            return false;
        }

        if( !singletonServer.start() ) {
            return false;
        }

        return true;
    }

    @Override
    protected boolean unlockImpl() {
        return singletonServer.shutdown();
    }

    public class Server implements Runnable {
       private final InetAddress localInetAddress;
       private final int portNumber;

       private volatile boolean shallQuit = false;
       private volatile boolean alive = false;

       private final Object syncOnStartStop = new Object();
       private ServerSocket serverSocket = null;
       private Thread serverThread = null;  // allowing kill() to force-stop last server-thread

       public Server(final InetAddress localInetAddress, final int portNumber) {
           this.localInetAddress = localInetAddress;
           this.portNumber = portNumber;
       }

       public final InetAddress getLocalInetAddress() { return localInetAddress; }
       public final int getPortNumber() { return portNumber; }

       public final boolean start() {
           if(alive) return true;

           final String sname;
           synchronized (Server.class) {
               serverInstanceCount++;
               sname = "SingletonServerSocket"+serverInstanceCount+"-"+fullName;
           }
           synchronized (syncOnStartStop) {
               shallQuit = false;
               serverThread = new InterruptSource.Thread(null, this, sname);
               serverThread.setDaemon(true);  // be a daemon, don't keep the JVM running
               serverThread.start();
               try {
                   while( !alive && !shallQuit ) {
                       syncOnStartStop.wait();
                   }
               } catch (final InterruptedException ie) {
                   final InterruptedException ie2 = SourcedInterruptedException.wrap(ie);
                   shutdown(false);
                   throw new InterruptedRuntimeException(ie2);
               }
           }
           final boolean ok = isBound();
           if(!ok) {
               shutdown(true);
           }
           return ok;
       }

       public final boolean shutdown() {
           return shutdown(true);
       }
       private final boolean shutdown(final boolean wait) {
           if(!alive) return true;

           try {
               synchronized (syncOnStartStop) {
                   shallQuit = true;
                   connect();
                   if( wait ) {
                       try {
                           while( alive ) {
                               syncOnStartStop.wait();
                           }
                       } catch (final InterruptedException ie) {
                           throw new InterruptedRuntimeException(ie);
                       }
                   }
               }
           } finally {
               if(alive) {
                   System.err.println(infoPrefix()+" EEE "+getName()+" - Unable to remove lock: ServerThread still alive ?");
                   kill();
               }
           }
           return true;
       }

       /**
        * Brutally kill server thread and close socket regardless.
        * This is out last chance for JVM shutdown.
        */
       @SuppressWarnings("deprecation")
       public final void kill() {
           if(alive) {
                System.err.println(infoPrefix()+" XXX "+getName()+" - Kill @ JVM Shutdown");
           }
           alive = false;
           shallQuit = false;
           if(null != serverThread && serverThread.isAlive() ) {
               try {
                   serverThread.stop();
               } catch(final Throwable t) { }
           }
           if(null != serverSocket) {
               try {
                   final ServerSocket ss = serverSocket;
                   serverSocket = null;
                   ss.close();
               } catch (final Throwable t) { }
           }
       }

       public final boolean isRunning() { return alive; }

       public final boolean isBound() {
           return alive && null != serverSocket && serverSocket.isBound() ;
       }

       public final Socket connect() {
           try {
               return new Socket(localInetAddress, portNumber);
           } catch (final Exception e) { }
           return null;
       }

       @Override
       public void run() {
           if(DEBUG) {
               System.err.println(infoPrefix()+" III - Start");
           }
           try {
               synchronized (syncOnStartStop) {
                   try {
                       serverSocket = new ServerSocket(portNumber, 1, localInetAddress);
                       serverSocket.setReuseAddress(true); // reuse same port w/ subsequent instance, i.e. overcome TO state when JVM crashed
                       alive = true;
                   } catch (final IOException e) {
                       System.err.println(infoPrefix()+" III - Unable to install ServerSocket: "+e.getMessage());
                       shallQuit = true;
                   } finally {
                       syncOnStartStop.notifyAll();
                   }
               }

               while (!shallQuit) {
                   try {
                       final Socket clientSocket = serverSocket.accept();
                       clientSocket.close();
                   } catch (final IOException ioe) {
                       System.err.println(infoPrefix()+" EEE - Exception during accept: " + ioe.getMessage());
                   }
               }
           } catch(final ThreadDeath td) {
               if( DEBUG ) {
                   ExceptionUtils.dumpThrowable("", td);
               }
           } finally {
               synchronized (syncOnStartStop) {
                   if(DEBUG) {
                       System.err.println(infoPrefix()+" III - Stopping: alive "+alive+", shallQuit "+shallQuit+", hasSocket "+(null!=serverSocket));
                   }
                   if(null != serverSocket) {
                       try {
                           serverSocket.close();
                       } catch (final IOException e) {
                           System.err.println(infoPrefix()+" EEE - Exception during close: " + e.getMessage());
                       }
                   }
                   serverSocket = null;
                   alive = false;
                   syncOnStartStop.notifyAll();
               }
           }
       }
    }
}
