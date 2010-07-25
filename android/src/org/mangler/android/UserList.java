/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-06-13 00:25:11 +0200 (Sun, 13 Jun 2010) $
 * $Revision: 918 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/UserList.java $
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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import android.util.Log;

public class UserList {
	
	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void delUser(short userid) {
		for (Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext();) {
			if (((Short) iterator.next().get("userid")).equals(userid)) {
				iterator.remove();
				return;
			}
		}
	}
	
	public static void clear() {
		data.clear();
	}
	
	public static void addUser(short userid, String username, short channelid) {
			HashMap<String, Object> user = new HashMap<String, Object>();
			user.put("userstatus", R.drawable.transmit_off);
			user.put("userid", userid);
			user.put("username", username);
			user.put("channelid", channelid);
			user.put("channelname", ChannelList.getChannel(channelid).get("name").toString().trim());
			data.add(user);
	}
	
	public static void updateStatus(short userid, int status) {
		for (Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext();) {
			HashMap<String, Object> data = iterator.next();
			if ((Short) data.get("userid") == userid) {
				data.put("userstatus", status);
				return;
			}
		}
	}
	
	public static short getChannel(short userid) {
		for (Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext();) {
			HashMap<String, Object> data = iterator.next();
			if ((Short) data.get("userid") == userid) {
				short channelid = (Short) data.get("channelid");
				return (short) channelid;
			}
		}
		return -1;
	}

}
