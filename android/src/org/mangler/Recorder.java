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

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Build;

public class Recorder {

	private static Thread thread = null;
	private static boolean stop = false;
	private static int rate = 0;

	private static class RecordThread implements Runnable {
		public void run() {
			AudioRecord audiorecord = null;
			byte[] buf = null;
			int rate = 0;
			int pcmlen = 0;
			VentriloInterface.startaudio((short)0);
			for (;;) {
				if (audiorecord == null || rate != rate()) {
					if (audiorecord != null) {
						audiorecord.stop();
						audiorecord.release();
					}
					if (stop || (rate = rate()) <= 0 || (pcmlen = VentriloInterface.pcmlengthforrate(rate)) <= 0 || buffer() <= 0) {
						VentriloInterface.stopaudio();
						thread = null;
						return;
					}
					try {
						audiorecord = new AudioRecord(
							MediaRecorder.AudioSource.MIC,
							rate,
							AudioFormat.CHANNEL_CONFIGURATION_MONO,
							AudioFormat.ENCODING_PCM_16BIT,
							buffer()
						);
					}
					catch (IllegalArgumentException e) {
						throw e;
					}
					if (audiorecord.getState() == AudioRecord.STATE_UNINITIALIZED && rate() != 8000) {
						rate(8000);
						audiorecord.release();
						audiorecord = null;
						continue;
					}
					try {
						audiorecord.startRecording();
					}
					catch (IllegalStateException e) {
						VentriloInterface.stopaudio();
						audiorecord.release();
						thread = null;
						return;
					}
					buf = new byte[pcmlen];
				}
		        for (int offset = 0, read = 0; offset < pcmlen; offset += read) {
		        	if (stop) {
		        		VentriloInterface.stopaudio();
		        		audiorecord.stop();
		        		audiorecord.release();
		        		thread = null;
		        		return;
		        	}
		        	if (!stop && (read = audiorecord.read(buf, offset, pcmlen - offset)) < 0) {
		        		throw new RuntimeException("AudioRecord read failed: " + Integer.toString(read));
		        	}
		        }
		        if (!stop && rate == rate()) {
		        	VentriloInterface.sendaudio(buf, pcmlen, rate);
		        }
			}
		}
	}

	private static int buffer() {
		return AudioRecord.getMinBufferSize(rate, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
	}

	public static void rate(final int _rate) {
		rate = Build.PRODUCT.equals("sdk") ? 8000 : _rate;
	}

	public static int rate() {
		return rate;
	}

	public static boolean recording() {
		return thread != null;
	}

	public static boolean start() {
		if (recording() || rate <= 0) {
			return true;
		}
		if (buffer() <= 0) {
			return false;
		}
		stop = false;
		(thread = new Thread(new RecordThread())).start();
		return true;
	}

	public static void stop() {
		stop = true;
	}

}
