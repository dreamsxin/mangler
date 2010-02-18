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
		        
		        Log.e("8000", Integer.toString(VentriloInterface.pcmlengthforrate(8000)));
		        Log.e("16000", Integer.toString(VentriloInterface.pcmlengthforrate(16000)));
		        Log.e("32000", Integer.toString(VentriloInterface.pcmlengthforrate(32000)));
		        Log.e("44000", Integer.toString(VentriloInterface.pcmlengthforrate(44000)));
		        
		        /*
		        int pcm_length = 5000;
		        int offset = 0;
		        byte buffer[] = new byte[pcm_length];
		        do {
		        	Log.e("ddebug", "---> Requested bytes: " + Integer.toString(pcm_length - offset) + ", offset: " + Integer.toString(offset));
		        	int pcm_read = record.read(buffer, offset, pcm_length - offset);
		        	Log.e("ddebug", "---> Bytes read: " + Integer.toString(pcm_read));
		        	offset += pcm_read;
		        	Log.e("ddebug", "---> Total read: " + Integer.toString(offset) + ", " + Integer.toString(pcm_length - offset) + " bytes left.");
		        }
		        while(offset < pcm_length);
		        Log.e("ddebug", "Total bytes read: " + Integer.toString(offset));
		        */
		        
		        /*
		        do {
		        	int pcm_read = record.read(store, pcm_length);
		        	if(pcm_read == -3 || pcm_read == -2) {
		        		break;
		        	}
		        	Log.e("recordread", "Read " + Integer.toString(pcm_read) + " of " + Integer.toString(pcm_length) + " bytes.");
		        }
		        while(store.arrayOffset() < pcm_length);
		        */
		        
		        /*
		        short buffer[] = new short[bufsz];
		        while(true) {
			    	int bufrd = record.read(buffer, 0, bufsz);
			    	if(bufrd != AudioRecord.ERROR_BAD_VALUE && bufrd != AudioRecord.ERROR_INVALID_OPERATION) {
			    		VentriloInterface.sendaudio(buffer, bufrd, rate);
			    	}
			    	else {
			    		Log.e("read_fail", "bufrd returned: " + Integer.toString(bufrd));
			    	}
		        }*/
		    } 
    	
    	}
}