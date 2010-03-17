package org.mangler;

import android.app.Activity;
import android.os.Bundle;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.AudioFormat;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
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
		    	
		    	VentriloInterface.changechannel((short)7, "");
		    	try { Thread.sleep(5000); } catch(InterruptedException e) {}
		      
		        int pcm_length = VentriloInterface.pcmlengthforrate(rate);
		        byte buffer[] = new byte[pcm_length];
		        VentriloInterface.startaudio((short) 3);
		        
		        while(true) {
			        int offset = 0;
			        do {
			        	int pcm_read = record.read(buffer, offset, pcm_length - offset);
			        	offset += pcm_read;
			        }
			        while(offset < pcm_length);
			        VentriloInterface.sendaudio(buffer, pcm_length, rate);
		        }
		        
		        
		    } 
    	
    	}
}