package org.mangler;

public class VentriloInterface {
	public static native int 		login(String server, String username, String password, String phonetic);
	public static native void 		joinchat();
	public static native void 		leavechat();
	public static native void 		sendchatmessage(String message);
	public static native void 		logout();
	public static native void 		changechannel(short channelid, String password);
	public static native void 		phantomadd(short channelid);
	public static native void 		phantomremove(short channelid);
	public static native int  		debuglevel(int level);
	public static native boolean	isloggedin();
	public static native short 		getuserid();
	public static native void 		settext(String comment, String url, String integrationtext, Boolean silent);
	public static native int 		messagewaiting(int block);
	public static native int 		getsoundqlength();
	public static native int 		getmaxclients();
	public static native void 		clearevents();
	public static native int 		getcodecrate(short codec, short format);
	public static native short 		getuserchannel(short id);
	public static native short 		channelrequirespassword(short channelid);
	public static native void 		startaudio(short type);
	public static native void 		stopaudio();
	public static native int 		usercount();
	public static native int 		channelcount();
	public static native void		startprocessing();
	public static native boolean	recv();
	public static native void		sendaudio(byte[] pcm, int size, int rate);
	public static native int		pcmlengthforrate(int rate);
}
