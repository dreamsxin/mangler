package org.mangler;

import android.app.Activity;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;

import java.lang.Thread;
import java.lang.Runnable;

public class main extends Activity {
    
	private ManglerDBAdapter dbHelper;
	
	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	System.loadLibrary("ventrilo_interface");
    	
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        // Temporary initialization, uncomment for first run
        
        /*dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();
        dbHelper.createUser("droid_test", "durr");
        dbHelper.createServer(1, "mangler_test", "vent.mangler.org", 9047);
        */
        
        Intent intent = new Intent(this, UserList.class);
        startActivity(intent);

        // Connection code currently at bottom of ServerList class
        
    	/*if(VentriloInterface.login("vent.mangler.org:9047", "droid_test", "", "") != 0) {
	    	
	    	Runnable runnable = new Runnable() {
	    		public void run() {
	    			while(true) {
	    				if(!VentriloInterface.recv()) break;
	    			}
	    		}
	    	};
	    	Thread t = new Thread(runnable);
	    	t.start();
	    */
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
	        
	    //}
    }   
}