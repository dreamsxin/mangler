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

public class VentriloEvents {
    public static final int V3_EVENT_STATUS 					= 1;
    public static final int V3_EVENT_PING 						= 2;
	public static final int V3_EVENT_USER_LOGIN 				= 3;
	public static final int V3_EVENT_USER_LOGOUT 				= 4;
	public static final int V3_EVENT_LOGIN_COMPLETE 			= 5;
	public static final int V3_EVENT_LOGIN_FAIL 				= 6;
	public static final int V3_EVENT_USER_CHAN_MOVE 			= 7;
	public static final int V3_EVENT_CHAN_ADD 					= 8;
	public static final int V3_EVENT_CHAN_MODIFY 				= 9;
	public static final int V3_EVENT_CHAN_REMOVE 				= 10;
	public static final int V3_EVENT_CHAN_BADPASS 				= 11;
	public static final int V3_EVENT_ERROR_MSG 					= 12;
	public static final int V3_EVENT_USER_TALK_START 			= 13;
	public static final int V3_EVENT_USER_TALK_END 				= 14;
	public static final int V3_EVENT_USER_TALK_MUTE				= 15;
	public static final int V3_EVENT_PLAY_AUDIO 				= 16;
	public static final int V3_EVENT_DISPLAY_MOTD 				= 17;
	public static final int V3_EVENT_DISCONNECT 				= 18;
	public static final int V3_EVENT_USER_MODIFY 				= 19;
	public static final int V3_EVENT_CHAT_JOIN 					= 20;
	public static final int V3_EVENT_CHAT_LEAVE 				= 21;
	public static final int V3_EVENT_CHAT_MESSAGE 				= 22;
	public static final int V3_EVENT_ADMIN_AUTH 				= 23;
	public static final int V3_EVENT_CHAN_ADMIN_UPDATED 		= 24;
	public static final int V3_EVENT_PRIVATE_CHAT_MESSAGE 		= 25;
	public static final int V3_EVENT_PRIVATE_CHAT_START 		= 26;
	public static final int V3_EVENT_PRIVATE_CHAT_END 			= 27;
	public static final int V3_EVENT_PRIVATE_CHAT_AWAY 			= 28;
	public static final int V3_EVENT_PRIVATE_CHAT_BACK 			= 29;
	public static final int V3_EVENT_USERLIST_ADD 				= 30;
	public static final int V3_EVENT_USERLIST_MODIFY 			= 31;
	public static final int V3_EVENT_USERLIST_REMOVE 			= 32;
	public static final int V3_EVENT_USERLIST_CHANGE_OWNER 		= 33;
	public static final int V3_EVENT_USER_GLOBAL_MUTE_CHANGED 	= 34;
	public static final int V3_EVENT_USER_CHANNEL_MUTE_CHANGED 	= 35;
	public static final int V3_EVENT_PERMS_UPDATED 				= 36;
	public static final int V3_EVENT_USER_RANK_CHANGE 			= 37;
	
	/*
	 * Probably won't need this because we call into native functions that handle this for us.
	 * 
	public static final int V3_EVENT_CHANGE_CHANNEL 			= 0;
	public static final int V3_EVENT_PHANTOM_ADD 				= 0;
	public static final int V3_EVENT_PHANTOM_REMOVE 			= 0;
	public static final int V3_EVENT_ADMIN_LOGIN 				= 0;
	public static final int V3_EVENT_ADMIN_LOGOUT 				= 0;
	public static final int V3_EVENT_ADMIN_KICK 				= 0;
	public static final int V3_EVENT_ADMIN_BAN 					= 0;
	public static final int V3_EVENT_ADMIN_CHANNEL_BAN 			= 0;
	public static final int V3_EVENT_FORCE_CHAN_MOVE 			= 0;
	public static final int V3_EVENT_USERLIST_OPEN 				= 0;
	public static final int V3_EVENT_USERLIST_CLOSE 			= 0;*/
}
