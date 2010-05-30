package org.mangler;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

public class Player {
	
	private AudioTrack audiotrack;
	
	public Player(int rate) {
		// Attempt to initialize AudioTrack instance.
		try {
			audiotrack = new AudioTrack(
				AudioManager.STREAM_VOICE_CALL, 
				rate,
				AudioFormat.CHANNEL_CONFIGURATION_MONO, 
				AudioFormat.ENCODING_PCM_16BIT, 
				32768, 
				AudioTrack.MODE_STREAM
			);
		}
		catch(IllegalArgumentException e) {
			throw e;
		}
		audiotrack.play();
	}
	
	public void rate(int rate) {
		audiotrack.setPlaybackRate(rate);
	}
	
	public int rate() {
		return audiotrack.getPlaybackRate();
	}
	
	public void write(byte[] sample, int length) {
		audiotrack.write(sample, 0, length);
	}
	
}
