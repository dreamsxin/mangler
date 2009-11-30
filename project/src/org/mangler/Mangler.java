package org.mangler;

import android.app.Activity;
import android.os.Bundle;

public class Mangler extends Activity {
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        System.loadLibrary("ventrilo3");
        
        VentriloInterface.login("vent.mangler.org:9047", "android", "", "");
    }
}