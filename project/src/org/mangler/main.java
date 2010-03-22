package org.mangler;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
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
	    
	    	/*
	    	// Play incoming audio.
	    	AudioTrack at = new AudioTrack(
	    		AudioManager.STREAM_VOICE_CALL, 
	    		8000, // set this to channel rate
	    		AudioFormat.CHANNEL_CONFIGURATION_MONO, 
	    		AudioFormat.ENCODING_PCM_16BIT, 
	    		32768, 
	    		AudioTrack.MODE_STREAM
	    	);
	    	at.play();
	    	
	    	VentriloEventData data = new VentriloEventData();
	    	for(;;) {
	    		VentriloInterface.getevent(data);
	    		if(data.type == VentriloEvents.V3_EVENT_PLAY_AUDIO) {
		    		at.write(data.data.sample, 0, data.pcm.length);
		    	}
	    	}
	    	*/
	    	
	    	/*
	    	// Send recorded audio.
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