package com.forrest.jrtplib;

public class JrtplibUtil {
    static {
        System.loadLibrary("jrtplib");
    }

    public native void test();
}
