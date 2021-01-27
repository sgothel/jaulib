package jau.test;

import java.io.IOException;

import org.jau.base.JaulibVersion;
import org.jau.util.VersionUtil;

public class VersionInfo {
    public static void main(final String args[]) throws IOException {
        System.err.println(VersionUtil.getPlatformInfo());
        System.err.println("Version Info:");
        final JaulibVersion v = JaulibVersion.getInstance();
        System.err.println(v);
        System.err.println("");
        System.err.println("Full Manifest:");
        System.err.println(v.getFullManifestInfo(null));
    }

}
