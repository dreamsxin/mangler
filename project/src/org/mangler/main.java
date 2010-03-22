package org.mangler;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import java.lang.Thread;
import java.lang.Runnable;

public class main extends Activity {
    /** Called when the activity is first created. */
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	System.loadLibrary("ventrilo_interface");
    	
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

    	if(VentriloInterface.login("vent.mangler.org:9047", "dandroid", "", "") != 0) {
	    	
	    	Runnable runnable = new Runnable() {
	    		public void run() {
	    			while(true) {
	    				if(!VentriloInterface.recv()) break;
	    			}
	    		}
	    	};
	    	Thread t = new Thread(runnable);
	    	t.start();
	       
	    	VentriloEventData data = new VentriloEventData();
	    	for(;;) {
	    		VentriloInterface.getevent(data);
		    	if(data.type == VentriloEvents.V3_EVENT_PLAY_AUDIO) {
		    		Log.i("Audio event", Integer.toString(data.pcm_length));
		    	}
	    	}
	    	
	    	/*
	    	try {
		    	Recorder rec = new Recorder(8000);
		    	rec.start();
		    	try { Thread.sleep(5000); } catch(InterruptedException e) {}
		    	rec.stop();
	    	}
	    	catch(RuntimeException e) {
	    		Log.e("RuntimeException", e.toString());
	    	}
	    	*/
	        
	    } 
    }
        
}