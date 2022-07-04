/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
package org.jau.fs;

import java.time.Instant;
import java.time.ZoneOffset;

/**
 * Platform agnostic representation of POSIX ::lstat() and ::stat()
 * for a given pathname.
 *
 * Implementation follows the symbolic link, i.e. first opens
 * the given pathname with ::lstat() and if identifying as a symbolic link
 * opens it via ::stat() to retrieve the actual properties like size, time and ownership.
 *
 * On `GNU/Linux` implementation uses ::statx().
 */
public class FileStats {
    public static class Field {
        public enum Type {
            /** No mode bit set */
            none           (                  0 ),
            /** File type mode bits */
            type           ( 0b0000000000000001 ),
            /** POSIX file protection mode bits */
            mode           ( 0b0000000000000010 ),
            nlink          ( 0b0000000000000100 ),
            uid            ( 0b0000000000001000 ),
            gid            ( 0b0000000000010000 ),
            atime          ( 0b0000000000100000 ),
            mtime          ( 0b0000000001000000 ),
            ctime          ( 0b0000000010000000 ),
            ino            ( 0b0000000100000000 ),
            size           ( 0b0000001000000000 ),
            blocks         ( 0b0000010000000000 ),
            btime          ( 0b0000100000000000 );

            Type(final int v) { value = v; }
            public final int value;
        }
        public int mask;

        public Field(final int v) {
            mask = v;
        }
        public Field() {
            mask = 0;
        }

        public boolean isSet(final Type bit) { return bit.value == ( mask & bit.value ); }
        public void set(final Type bit) { mask = (short) ( mask | bit.value ); }

        @Override
        public String toString() {
            int count = 0;
            final StringBuilder out = new StringBuilder();
            for (final Type dt : Type.values()) {
                if( isSet(dt) ) {
                    if( 0 < count ) { out.append(", "); }
                    out.append(dt.name()); count++;
                }
            }
            if( 1 < count ) {
                out.insert(0, "[");
                out.append("]");
            }
            return out.toString();
        }
        @Override
        public boolean equals(final Object other) {
            if (this == other) {
                return true;
            }
            return (other instanceof Field) &&
                   this.mask == ((Field)other).mask;
        }
    }

    private final DirItem item_;
    private final String link_target_path_; // stored link-target path this symbolic-link points to if is_link(), otherwise nullptr.

    private final Field has_fields_;
    private final FMode mode_;
    private final int uid_;
    private final int gid_;
    private final int errno_res_;

    private final long size_;
    // final ZonedDateTime utc = btime_.atZone(ZoneOffset.UTC); String s = utc.toString();
    private final Instant btime_; // Birth or creation time
    private final Instant atime_; // Last access
    private final Instant ctime_; // Last meta-status change
    private final Instant mtime_; // Last modification

    private FileStats link_target_; // link-target this symbolic-link points to if is_link(), otherwise nullptr.

    /** Instantiate an empty file_stats with fmode_t::not_existing set. */
    public FileStats() {
        item_ = new DirItem();
        link_target_path_ = null;

        has_fields_ = new Field();
        mode_ = new FMode();
        mode_.set(FMode.Bit.not_existing);
        uid_ = 0;
        gid_ = 0;
        errno_res_ = 0;

        size_ = 0;
        btime_ = Instant.ofEpochSecond(0, 0);
        atime_ = btime_;
        ctime_ = btime_;
        mtime_ = btime_;

        link_target_ = null;
    }

    @Override
    public boolean equals(final Object other) {
        if (this == other) {
            return true;
        }
        if( other instanceof FileStats ) {
            final FileStats o = (FileStats)other;
            return item_.equals( o.item_ ) &&
                   has_fields_.equals(o.has_fields_) &&
                   mode_.equals(o.mode_) &&
                   uid_ == o.uid_ && gid_ == o.gid_ &&
                   errno_res_ == o.errno_res_ &&
                   size_ == o.size_ &&
                   btime_.equals( o.btime_ ) &&
                   atime_.equals( o.atime_ ) &&
                   ctime_.equals( o.ctime_ ) &&
                   mtime_.equals( o.mtime_ ) &&
                   ( !is_link() ||
                     ( link_target_path_.equals(o.link_target_path_) &&
                       link_target_.equals(o.link_target_)
                     )
                   );
        } else {
            return false;
        }
    }

    /**
     * Instantiates a file_stats for the given `path`.
     *
     * The dir_item will be constructed without parent_dir
     * @param path the path to produce stats for
     */
    public FileStats(final String path) {
        this( ctorImpl(path) );
    }
    private FileStats(final long h) {
        {
            final String[/*3*/] s3 = getString3DirItemLinkTargetPath(h);
            item_ = new DirItem(s3[0], s3[1]);
            link_target_path_ = s3[2];
        }
        {
            final int[/*3*/] i5 = getInt5FieldsFModeUidGidErrno(h);
            has_fields_ = new Field(i5[0]);
            mode_ = new FMode(i5[1]);
            uid_ = i5[2];
            gid_ = i5[3];
            errno_res_ = i5[4];
        }
        {
            final long[/*1+ 2*4*/] l3 = getLong9SizeTimes(h);
            size_ = l3[0];
            btime_ = Instant.ofEpochSecond(l3[1+0*2+0], l3[1+0*2+1]);
            atime_ = Instant.ofEpochSecond(l3[1+1*2+0], l3[1+1*2+1]);
            ctime_ = Instant.ofEpochSecond(l3[1+2*2+0], l3[1+2*2+1]);
            mtime_ = Instant.ofEpochSecond(l3[1+3*2+0], l3[1+3*2+1]);
        }
        {
            final long lth = ctorLinkTargetImpl(h);
            if( lth == 0 ) {
                link_target_ = null;
            } else {
                link_target_ = new FileStats(lth);
            }
        }
        dtorImpl(h);
    }
    private static native long ctorImpl(final String path);
    private static native void dtorImpl(final long h);
    private static native int[/*4*/] getInt5FieldsFModeUidGidErrno(final long h);
    private static native String[/*3*/] getString3DirItemLinkTargetPath(final long h);
    private static native long[/*1+ 2*4*/] getLong9SizeTimes(final long h);
    private static native long ctorLinkTargetImpl(final long h);

    /**
     * Returns the dir_item.
     *
     * In case this instance is created by following a symbolic link instance,
     * it represents the resolved path relative to the used symbolic link's dirname.
     *
     * @see is_link()
     * @see path()
     */
    public DirItem item() { return item_; }

    /**
     * Returns the unix path representation.
     *
     * In case this instance is created by following a symbolic link instance,
     * it represents the resolved path relative to the used symbolic link's dirname.
     *
     * @see is_link()
     * @see item()
     */
    public String path() { return item_.path(); }

    /**
     * Returns the stored link-target path this symbolic-link points to if instance is a symbolic-link, otherwise nullptr.
     *
     * @see is_link()
     * @see link_target()
     * @see final_target()
     */
    public String link_target_path() { return link_target_path_; }

    /**
     * Returns the link-target this symbolic-link points to if instance is a symbolic-link, otherwise nullptr.
     *
     * nullptr is also returned for an erroneous symbolic-links, i.e. non-existing link-targets or recursive loop-errors.
     *
     * @see is_link()
     * @see link_target_path()
     * @see final_target()
     */
    public FileStats link_target() { return link_target_; }

    /**
     * Returns the final target element, either a pointer to this instance if not a symbolic-link
     * or the final link-target a symbolic-link (chain) points to.
     *
     * @param link_count optional size_t pointer to store the number of symbolic links leading to the final target, excluding the final instance. 0 indicates no symbolic-link;
     *
     * @see is_link()
     * @see link_target_path()
     * @see link_target()
     */
    public FileStats final_target(final long link_count[]) {
        long count = 0;
        FileStats fs0 = this;
        FileStats fs1 = fs0.link_target();
        while( null != fs1 ) {
            ++count;
            fs0 = fs1;
            fs1 = fs0.link_target();
        }
        if( null != link_count && link_count.length > 0 ) {
            link_count[0] = count;
        }
        return fs0;
    }

    /** Returns true if the given field_t fields were retrieved, otherwise false. */
    public boolean has(final Field.Type bit) { return has_fields_.isSet(bit); }

    /** Returns the retrieved field_t fields. */
    public Field fields() { return has_fields_; }

    /** Returns the FMode, file type and mode. */
    public FMode mode() { return mode_; }

    /** Returns the POSIX protection bit portion of fmode_t, i.e. mode() & {@link FMode.Bit#protection_mask}. */
    public FMode prot_mode() { return mode_.mask(FMode.Bit.protection_mask.value); }

    /** Returns the user id, owning the element. */
    public int uid() { return uid_; }

    /** Returns the group id, owning the element. */
    public int gid() { return gid_; }

    /**
     * Returns the size in bytes of this element if is_file(), otherwise zero.
     *
     * If the element also is_link(), the linked target size is returned.
     */
    public long size() { return size_; }

    /** Returns the birth time of this element since Unix Epoch, i.e. its creation time. */
    public Instant btime() { return btime_; }
    /** Returns the last access time of this element since Unix Epoch. */
    public Instant atime() { return atime_; }
    /** Returns the last status change time of this element since Unix Epoch. */
    public Instant ctime() { return ctime_; }
    /** Returns the last modification time of this element since Unix Epoch. */
    public Instant mtime() { return mtime_; }

    /** Returns the `errno` value occurred to produce this instance, or zero for no error. */
    public int errno_res() { return errno_res_; }

    /** Returns true if no error occurred */
    public boolean ok() { return 0 == errno_res_; }

    /** Returns true if entity is a file, might be in combination with is_link().  */
    public boolean is_file() { return mode_.isSet( FMode.Bit.file ); }

    /** Returns true if entity is a directory, might be in combination with is_link().  */
    public boolean is_dir() { return mode_.isSet( FMode.Bit.dir ); }

    /** Returns true if entity is a symbolic link, might be in combination with is_file() or is_dir(). */
    public boolean is_link() { return mode_.isSet( FMode.Bit.link ); }

    /** Returns true if entity gives no access to user, exclusive bit. */
    public boolean has_access() { return !mode_.isSet( FMode.Bit.no_access); }

    /** Returns true if entity does not exist, exclusive bit. */
    public boolean exists() { return !mode_.isSet( FMode.Bit.not_existing ); }

    @Override
    public String toString() {
        final String stored_path, link_detail;
        {
            if( null != link_target_path_ ) {
                stored_path = " [-> "+link_target_path_+"]";
            } else {
                stored_path = new String();
            }
            final long link_count[] = { 0 };
            final FileStats final_target_ = final_target(link_count);
            if( 0 < link_count[0] ) {
                link_detail = " -(" + link_count[0] + ")-> '" + final_target_.path() + "'";
            } else {
                link_detail = new String();
            }
        }
        final StringBuilder res = new StringBuilder( "file_stats[");
        res.append(mode_)
           .append(", '"+item_.path()+"'"+stored_path+link_detail );
        if( 0 == errno_res_ ) {
            if( has( Field.Type.uid ) ) {
                res.append( ", uid " ).append( uid_ );
            }
            if( has( Field.Type.gid ) ) {
                res.append( ", gid " ).append( gid_ );
            }
            if( has( Field.Type.size ) ) {
                res.append( ", size " ).append( String.format("%,d",  size_ ) );
            }
            if( has( Field.Type.btime ) ) {
                res.append( ", btime " ).append( btime_.atZone(ZoneOffset.UTC) );
            }
            if( has( Field.Type.atime ) ) {
                res.append( ", atime " ).append( atime_.atZone(ZoneOffset.UTC) );
            }
            if( has( Field.Type.ctime ) ) {
                res.append( ", ctime " ).append( ctime_.atZone(ZoneOffset.UTC) );
            }
            if( has( Field.Type.mtime ) ) {
                res.append( ", mtime " ).append( mtime_.atZone(ZoneOffset.UTC) );
            }
            // res.append( ", fields ").append( jau::fs::to_string( has_fields_ ) );
        } else {
            res.append( ", errno " ).append( errno_res_ );
        }
        res.append("]");
        return res.toString();
    }
}
