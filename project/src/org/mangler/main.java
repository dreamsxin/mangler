package org.mangler;

import android.app.Activity;
import android.os.Bundle;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.AudioFormat;
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
		
		    	VentriloInterface.changechannel((short) 2, "");
		    	try { Thread.sleep(5000); } catch(InterruptedException e) {}
		    	VentriloInterface.startaudio((short) 3);

		        int rate = 8000;
		        int bufsz = 
		        AudioRecord.getMinBufferSize
		        (
					rate, 
					AudioFormat.CHANNEL_CONFIGURATION_MONO, 
					AudioFormat.ENCODING_PCM_16BIT
				);
		        
		        AudioRecord record = 
		        new AudioRecord
		        (
					MediaRecorder.AudioSource.MIC,
					rate, 
					AudioFormat.CHANNEL_CONFIGURATION_MONO, 
					AudioFormat.ENCODING_PCM_16BIT, 
					bufsz
		        );
		        record.startRecording();
		        
		        short buffer[] = new short[bufsz];
		        while(true) {
			    	int bufrd = record.read(buffer, 0, bufsz);
			    	if(bufrd != AudioRecord.ERROR_BAD_VALUE && bufrd != AudioRecord.ERROR_INVALID_OPERATION) {
			    		VentriloInterface.sendaudio(buffer, bufrd, rate);
			    	}
			    	else {
			    		Log.e("read_fail", "bufrd returned: " + Integer.toString(bufrd));
			    	}
		        }
		    } 
    	
    	}
}