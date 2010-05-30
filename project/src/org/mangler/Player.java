/*
 * $LastChangedDate: 2010-05-28 20:39:54 +0200 (Fri, 28 May 2010) $
 * $Revision: 846 $
 * $LastChangedBy: clearscreen $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/libventrilo3.c $
 *
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
