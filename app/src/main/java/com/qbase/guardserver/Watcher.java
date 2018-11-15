package com.qbase.guardserver;

/**
 * Create by qay on 2018/11/15
 */
public class Watcher {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }


    public native void createWatcher(String userId);


    public native void connectMonitor();

}
