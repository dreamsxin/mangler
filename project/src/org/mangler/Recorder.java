package org.mangler;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

public class Recorder {
	
	private byte[] buffer;
	boolean is_recording = false;
	private int rate = 0;
	private int	pcmlength = 0;
	private AudioRecord audiorecord;
	
	public Recorder(int rate) {
		if(!VentriloInterface.isloggedin()) {
			throw new RuntimeException("Login before instantiating recorder instance.");
		}
		
		if(rate <= 0) {
			throw new RuntimeException("Incorrect rate specified.");
		}
		this.rate = rate;
		
		this.pcmlength = VentriloInterface.pcmlengthforrate(this.rate);
		if(this.pcmlength == 0) {
			throw new RuntimeException("Libventrilo could not determine pcm length.");
		}
		
		try {
			this.audiorecord = new AudioRecord(
				MediaRecorder.AudioSource.MIC,
				this.rate, 
				AudioFormat.CHANNEL_CONFIGURATION_MONO, 
				AudioFormat.ENCODING_PCM_16BIT, 
		        AudioRecord.getMinBufferSize
		        (
					this.rate, 
					AudioFormat.CHANNEL_CONFIGURATION_MONO, 
					AudioFormat.ENCODING_PCM_16BIT
				)
			);
		}
		catch(IllegalArgumentException ex) {
			throw ex;
		}
		
		this.buffer = new byte[this.pcmlength];
	}
	
	private class RecordThread implements Runnable {
		public void run() {
			while(recording()) {
		        int offset = 0;
		        do {
		        	int pcm_read = audiorecord.read(buffer, offset, pcmlength - offset);
		        	if(pcm_read == -3 || pcm_read == -2) {
		        		throw new RuntimeException("Audiorecord read failed: " + Integer.toString(pcm_read));
		        	}
		        	offset += pcm_read;
		        }
		        while(offset < pcmlength);
		        VentriloInterface.sendaudio(buffer, pcmlength, rate);
			}
		}
	}
	
	private synchronized boolean recording() {
		return this.is_recording;
	}
	
	private synchronized void recording(boolean is_recording) {
		this.is_recording = is_recording;
	}
	
	public void start() {
		if(recording()) {
			return;
		}
		this.recording(true);
		this.audiorecord.startRecording();
		VentriloInterface.startaudio((short) 3);
		(new Thread(new RecordThread())).start();
	}
	
	public void stop() {
		VentriloInterface.stopaudio();
		this.recording(false);
		this.audiorecord.stop();
	}
	
}
