/*
 * This file is part of Mangler.
 *
 * Mangler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mangler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mangler.  If not, see <http://www.gnu.org/licenses/>.
 */

package org.mangler;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

public class Recorder {
	
	public static Recorder recorder;
	private boolean is_recording = false;
	private byte[] buffer;
	private int	pcmlength = 0;
	private int rate = 0;
	private AudioRecord audiorecord;
	
	public Recorder(final int rate) {
		if(!VentriloInterface.isloggedin()) {
			throw new RuntimeException("Login before instantiating recorder instance.");
		}	
		
		// Attempt to initialize AudioRecord instance.
		try {
			audiorecord = new AudioRecord(
				MediaRecorder.AudioSource.MIC,
				rate, 
				AudioFormat.CHANNEL_CONFIGURATION_MONO, 
				AudioFormat.ENCODING_PCM_16BIT, 
		        AudioRecord.getMinBufferSize
		        (
					rate,
					AudioFormat.CHANNEL_CONFIGURATION_MONO, 
					AudioFormat.ENCODING_PCM_16BIT
				)
			);
		}
		catch(IllegalArgumentException ex) {
			throw ex;
		}
		
		this.rate = rate;

		// Generate pcm length from input rate and create buffer.
		pcmlength = VentriloInterface.pcmlengthforrate(rate);
		if(pcmlength <= 0) {
			throw new RuntimeException("libventrilo could not determine pcm length.");
		}
		buffer = new byte[pcmlength];
	}
	
	private class RecordThread implements Runnable {
		public void run() {
			while(true) {
		        int offset = 0;
		        do {
		        	// Kill thread if we stopped recording.
		        	if(!recording()) {
		        		return;
		        	}
		        
		        	// Read number of bytes equal to pcmlength - offset.
		        	int pcm_read = audiorecord.read(buffer, offset, pcmlength - offset);
		        	if(pcm_read < 0) {
		        		throw new RuntimeException("Audiorecord read failed: " + Integer.toString(pcm_read));
		        	}
		        	
		        	// Increment offset based on bytes read.
		        	offset += pcm_read;
		        }
		        while(offset < pcmlength);
		        
		        // Transmit recorded audio.
		        VentriloInterface.sendaudio(buffer, pcmlength, rate);
			}
		}
	}
	
	public boolean recording() {
		return is_recording;
	}
	
	private void recording(final boolean is_recording) {
		this.is_recording = is_recording;
	}
	
	public void start() {
		if(recording()) {
			return;
		}
		recording(true);
		audiorecord.startRecording();
		VentriloInterface.startaudio((short) 3);
		(new Thread(new RecordThread())).start();
	}
	
	public void stop() {
		if(!recording()) {
			return;
		}
		recording(false);
		audiorecord.stop();
		VentriloInterface.stopaudio();
	}
	
}
