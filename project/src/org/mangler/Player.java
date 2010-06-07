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
import android.media.AudioManager;
import android.media.AudioTrack;

public class Player {
	
	public static Player player;
	private AudioTrack audiotrack;

	public Player(int rate) {
		// Attempt to initialize AudioTrack instance.
		try {
			audiotrack = new AudioTrack(
				AudioManager.STREAM_MUSIC, 
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
	
	public void rate(final int rate) {
		audiotrack.setPlaybackRate(rate);
	}
	
	public int rate() {
		return audiotrack.getPlaybackRate();
	}
	
	public void write(final byte[] sample, final int length) {
		audiotrack.write(sample, 0, length);
	}
	
}
