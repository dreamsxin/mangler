package org.mangler;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

public class Player {
	
	private AudioTrack audiotrack;
	
	public Player() {
		this.audiotrack = new AudioTrack(
			AudioManager.STREAM_VOICE_CALL, 
			8000,
			AudioFormat.CHANNEL_CONFIGURATION_MONO, 
			AudioFormat.ENCODING_PCM_16BIT, 
			32768, 
			AudioTrack.MODE_STREAM
		);
		this.audiotrack.play();
	}
	
	public void set_rate(int rate) {
		this.audiotrack.setPlaybackRate(rate);
	}
	
	public void write(byte[] sample, int length) {
		this.audiotrack.write(sample, 0, length);
	}
	
}
