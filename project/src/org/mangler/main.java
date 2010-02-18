package org.mangler;

import android.app.Activity;
import android.os.Bundle;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.AudioFormat;
import android.util.Log;
import java.lang.Thread;
import java.lang.Runnable;
import java.nio.ByteBuffer;

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
		        
		        // We require an even number because recording in 16 bit. (8 bit does not work!)
		        int pcm_length = VentriloInterface.pcmlengthforrate(8000);
		        byte buffer[] = new byte[pcm_length];
		        
		        VentriloInterface.startaudio((short) 3);
		        
		        while(VentriloInterface.isloggedin()) {
			        int offset = 0;
			        do {
			        	//Log.e("ddebug", "---> Requested bytes: " + Integer.toString(pcm_length - offset) + ", offset: " + Integer.toString(offset));
			        	int pcm_read = record.read(buffer, offset, pcm_length - offset);
			        	//Log.e("ddebug", "---> Bytes read: " + Integer.toString(pcm_read));
			        	offset += pcm_read;
			        	//Log.e("ddebug", "---> Total read: " + Integer.toString(offset) + ", " + Integer.toString(pcm_length - offset) + " bytes left.");
			        }
			        while(offset < pcm_length);
			        //Log.e("ddebug", "Total bytes read: " + Integer.toString(offset));
			        
			        VentriloInterface.sendaudio(buffer, pcm_length, 8000);
		        }
		    } 
    	
    	}
}