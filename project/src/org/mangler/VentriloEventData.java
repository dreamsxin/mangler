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

public class VentriloEventData {

	public short type;
	public int ping;
	
	public class _status {
		public byte percent;
		public byte[] message = new byte[256];
	}
	_status status = new _status();
	
	public class _error {
		public short code;
		public boolean disconnected;
		public byte[] message = new byte[512];
	}
	_error error = new _error();
	
	public class _pcm {
		public int length;
		public int rate;
		public short send_type;
		public byte channels;
	};
	_pcm pcm = new _pcm();
	
	public class _user {
		public short id;
		public short privchat_user1;
		public short privchat_user2;
	}
	_user user = new _user();
	
	public class _channel {
		public short id;
	}
	_channel channel = new _channel();
	
	public class _account {
		public short id;
		public short id2;
	}
	_account account = new _account();
	
	public class _text {
        public byte[] name 				= new byte[32];
        public byte[] password 			= new byte[32];
        public byte[] phonetic 			= new byte[32];
        public byte[] comment 			= new byte[128];
        public byte[] url 				= new byte[128];
        public byte[] integration_text 	= new byte[128];
	}
	_text text = new _text();
	
	public class _serverproperty {
		public short property;
		public byte value;
	}
	_serverproperty serverproperty = new _serverproperty();
	
	public class _data {
		public class _account {
			public byte[] username 		= new byte[32];
			public byte[] owner 		= new byte[32];
			public byte[] notes 		= new byte[256];
			public byte[] lock_reason 	= new byte[128];
			public short[] chan_auth 	= new short[32];
			public short[] chan_admin 	= new short[32];
			public int chan_admin_count;
			public int chan_auth_count;
		}
		_account account = new _account();
		
		public class _channel {
			public short id;
			public short parent;
			public short channel_codec;
			public short channel_format;
			public boolean password_protected;
	        /*
			// Probably won't be needing these.
	        short voice_mode;
	        short transmit_time_limit;
	        short max_clients;
	        short protect_mode;
	        short transmit_rank_level;
	        boolean allow_recording;
	        boolean allow_cross_channel_transmit;
	        boolean allow_paging;
	        boolean allow_wave_file_binds;
	        boolean allow_tts_binds;
	        boolean allow_u2u_transmit;
	        boolean allow_voice_target;
	        boolean allow_command_target;
	        boolean allow_guests;
	        boolean allow_phantoms;
	        boolean disable_guest_transmit;
	        boolean disable_sound_events;
	        boolean inactive_exempt;
	        */
		}
		_channel channel = new _channel();
		
		public class _rank {
			public short id;
			public short level;
		};
		_rank rank = new _rank();
		
		public byte[] sample		= new byte[32768];
		public byte[] motd			= new byte[32768];
		public byte[] chatmessage	= new byte[256];
		public byte[] reason		= new byte[128];
	}
	_data data = new _data();
	
}
