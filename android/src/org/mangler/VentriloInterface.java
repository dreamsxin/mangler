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

public class VentriloInterface {
	public static native void 		changechannel(short channelid, String password);
	public static native int 		channelcount();
	public static native boolean	channelrequirespassword(short channelid);
	public static native void 		clearevents();
	public static native int  		debuglevel(int level);
	public static native void		getchannel(VentriloEventData data, short channelid);
	public static native int		getchannelrate(short channelid);
	public static native int 		getcodecrate(short codec, short format);
	public static native void		getevent(VentriloEventData data);
	public static native int 		getmaxclients();
	public static native short 		getuserchannel(short id);
	public static native void		getuser(VentriloEventData data, short userid);
	public static native short 		getuserid();
	public static native boolean	isloggedin();
	public static native void 		joinchat();
	public static native void 		leavechat();
	public static native boolean	login(String server, String username, String password, String phonetic);
	public static native void 		logout();
	public static native boolean 	messagewaiting(boolean block);
	public static native int		pcmlengthforrate(int rate);
	public static native void 		phantomadd(short channelid);
	public static native void 		phantomremove(short channelid);
	public static native boolean	recv();
	public static native void		sendaudio(byte[] pcm, int size, int rate);
	public static native void 		sendchatmessage(String message);
	public static native void 		settext(String comment, String url, String integrationtext, Boolean silent);
	public static native void 		startaudio(short type);
	public static native void 		stopaudio();
	public static native int 		usercount();
}
