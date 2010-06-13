/*
 * This file is part of Mangler.
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
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

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class Player {

	private static Map<Short, AudioTrack> audiotracks = new HashMap<Short, AudioTrack>();

	private static AudioTrack open(final short id, final int rate, final byte channels, final int buffer) {
		AudioTrack audiotrack;
		close(id);
		try {
			audiotrack = new AudioTrack(
				AudioManager.STREAM_MUSIC,
				rate,
				(channels == 2)
					? AudioFormat.CHANNEL_CONFIGURATION_STEREO
					: AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT,
				buffer,
				AudioTrack.MODE_STREAM
			);
			audiotracks.put(id, audiotrack);
			return audiotrack;
		}
		catch (IllegalArgumentException e) {
			throw e;
		}
	}

	public static void close(final short id) {
		AudioTrack audiotrack;
		if ((audiotrack = audiotracks.get(id)) != null) {
			audiotrack.release();
			audiotracks.remove(id);
		}
	}

	public static void clear() {
		Set<Entry<Short, AudioTrack>> set = audiotracks.entrySet();
		for (Iterator<Entry<Short, AudioTrack>> iter = set.iterator(); iter.hasNext();) {
			Entry<Short, AudioTrack> entry = iter.next();
			entry.getValue().flush();
			entry.getValue().release();
		}
		audiotracks.clear();
	}

	public static void rate(final short id, final int rate) {
		AudioTrack audiotrack;
		if ((audiotrack = audiotracks.get(id)) != null) {
			audiotrack.setPlaybackRate(rate);
		}
	}

	public static int rate(final short id) {
		AudioTrack audiotrack;
		if ((audiotrack = audiotracks.get(id)) != null) {
			return audiotrack.getPlaybackRate();
		}
		return 0;
	}

	public static void write(final short id, final int rate, final byte channels, final byte[] sample, final int length) {
		AudioTrack audiotrack;
		if ((audiotrack = audiotracks.get(id)) == null) {
			audiotrack = open(id, rate, channels, VentriloInterface.pcmlengthforrate(rate) * channels * 2);
			audiotrack.play();
		}
		audiotrack.write(sample, 0, length);
	}

}
