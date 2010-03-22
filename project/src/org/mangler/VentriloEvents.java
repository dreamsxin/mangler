package org.mangler;

/* Cannot use enum due to the way the Java implements it compared to our native implementation. */

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
	public static final int V3_EVENT_PLAY_AUDIO 				= 15;
	public static final int V3_EVENT_DISPLAY_MOTD 				= 16;
	public static final int V3_EVENT_DISCONNECT 				= 17;
	public static final int V3_EVENT_USER_MODIFY 				= 18;
	public static final int V3_EVENT_CHAT_JOIN 					= 19;
	public static final int V3_EVENT_CHAT_LEAVE 				= 20;
	public static final int V3_EVENT_CHAT_MESSAGE 				= 21;
	public static final int V3_EVENT_ADMIN_AUTH 				= 22;
	public static final int V3_EVENT_CHAN_ADMIN_UPDATED 		= 23;
	public static final int V3_EVENT_PRIVATE_CHAT_MESSAGE 		= 24;
	public static final int V3_EVENT_PRIVATE_CHAT_START 		= 25;
	public static final int V3_EVENT_PRIVATE_CHAT_END 			= 26;
	public static final int V3_EVENT_PRIVATE_CHAT_AWAY 			= 27;
	public static final int V3_EVENT_PRIVATE_CHAT_BACK 			= 28;
	public static final int V3_EVENT_USERLIST_ADD 				= 29;
	public static final int V3_EVENT_USERLIST_MODIFY 			= 30;
	public static final int V3_EVENT_USERLIST_REMOVE 			= 31;
	public static final int V3_EVENT_USERLIST_CHANGE_OWNER 		= 32;
	public static final int V3_EVENT_USER_GLOBAL_MUTE_CHANGED 	= 33;
	public static final int V3_EVENT_USER_CHANNEL_MUTE_CHANGED 	= 34;
	public static final int V3_EVENT_PERMS_UPDATED 				= 35;
	public static final int V3_EVENT_USER_RANK_CHANGE 			= 36;
	
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
