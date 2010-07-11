/*
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-06-18 23:17:29 +0200 (Fri, 18 Jun 2010) $
 * $Revision: 945 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/Recorder.java $
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

package org.mangler.android;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Build;

public class Recorder {

	private static Thread thread = null; // limited to only one recording thread
	private static boolean stop = false; // stop flag
	private static int rate = 0; // current or overridden rate for current channel
	private static int buflen;

	private static class RecordThread implements Runnable {
		public void run() {
			AudioRecord audiorecord = null;
			byte[] buf = null; // send buffer

			// argument not needed; send method is hardcoded
			VentriloInterface.startaudio((short)0);
			// Find out if the minimum buffer length is smaller
			// than the amount of data we need to send.  If so,
			// adjust buflen (set from buffer()) accordingly
			if (buflen < VentriloInterface.pcmlengthforrate(0)) {
				buflen = VentriloInterface.pcmlengthforrate(0);
			}
			audiorecord = new AudioRecord(
					MediaRecorder.AudioSource.MIC,
					rate,
					AudioFormat.CHANNEL_CONFIGURATION_MONO,
					AudioFormat.ENCODING_PCM_16BIT,
					buflen
			);
			try {
				audiorecord.startRecording();
			}
			catch (IllegalStateException e) {
				VentriloInterface.stopaudio();
				audiorecord.release();
				thread = null;
				return;
			}
			buf = new byte[buflen];
			for (;;) {
		        for (int offset = 0, read = 0; offset < buflen; offset += read) {
	        		// if stop flag is set, exit now
		        	if (stop) {
		        		VentriloInterface.stopaudio();
		        		audiorecord.stop();
		        		audiorecord.release();
		        		// a new recording thread can now be instantiated
		        		thread = null;
		        		return;
		        	}
		        	if (!stop && (read = audiorecord.read(buf, offset, buflen - offset)) < 0) {
		        		throw new RuntimeException("AudioRecord read failed: " + Integer.toString(read));
		        	}
		        }
		        if (!stop) {
		        	VentriloInterface.sendaudio(buf, buflen, rate);
		        }
			}
		}
	}

	private static int buffer() {
		// all rates used by the protocol
		final int[] rates = { 8000, 11025, 16000, 22050, 32000, 44100 };

		for (int cur = 0; cur < rates.length; cur++) {
			// find the current rate in the rates array
			if (rates[cur] == rate()) {
				int buffer = 0;
				// try current and higher rates
				for (int ctr = cur; ctr < rates.length; ctr++) {
					buffer = AudioRecord.getMinBufferSize(
							rates[ctr],
							AudioFormat.CHANNEL_CONFIGURATION_MONO,
							AudioFormat.ENCODING_PCM_16BIT);
					if (buffer > 0) {
						// found a supported rate
						// override if it is not the channel rate and use the resampler
						if (rates[ctr] != rate()) {
							rate(rates[ctr]);
						}
						return buffer;
					}
				}
				// else try lower rates than current
				for (int ctr = cur - 1; ctr >= 0; ctr--) {
					buffer = AudioRecord.getMinBufferSize(
							rates[ctr],
							AudioFormat.CHANNEL_CONFIGURATION_MONO,
							AudioFormat.ENCODING_PCM_16BIT);
					if (buffer > 0) {
						if (rates[ctr] != rate()) {
							rate(rates[ctr]);
						}
						return buffer;
					}
				}
				// else break and return 0
				break;
			}
		}
		return 0;
	}

	public static void rate(final int _rate) {
		rate = Build.PRODUCT.equals("sdk") ? 8000 : _rate;
	}

	public static int rate() {
		return rate;
	}
	
	public static int buflen() {
		return buflen;
	}

	public static boolean recording() {
		// if a recording thread is running, we can't instantiate another one
		return thread != null;
	}

	public static boolean start() {
		if (recording() || rate <= 0) {
			return true;
		}
		// find a supported rate
		if ((buflen = buffer()) <= 0) {
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
