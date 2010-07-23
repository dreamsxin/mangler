/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-06-13 00:25:11 +0200 (Sun, 13 Jun 2010) $
 * $Revision: 918 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/ChannelList.java $
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
import java.util.ListIterator;

import android.util.Log;

public class ChannelList {

	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void addChannel(short channelid, String channelname, int passworded, short parentid) {
		HashMap<String, Object> channel = new HashMap<String, Object>();
		if (channelid == 0) {
			parentid = -1;
		}
		channel.put("channelid", channelid);
		for (int ctr = 0; ctr < getDepth(parentid)+1 && channelid != 0; ctr++) {
			channelname = "    " + channelname;
		}
		channel.put("channelname", channelname);
		channel.put("passworded", passworded);
		channel.put("parentid", parentid);
		Log.d("mangler", "adding channel at location " + getLocation(parentid));
		if (channelid == 0) {
			data.add(channel);
		} else {
			data.add(getLocation(parentid)+getChildCount(parentid)+1, channel);
		}
	}
	
	private static int getDepth(short channelid) {
		int depth = 0;
		short p;
		while ((p = getParentId(channelid)) >= 0) {
			channelid = p;
			depth++;
		}
		return depth;
	}
	
	private static int getChildCount(short channelid) {
		int childcount = 0;
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("parentid")).equals(channelid)) {	
				childcount++;
			}
		}
		return childcount;
	}
	
	private static int getLocation(short channelid) {
		int ctr = 0;
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("channelid")).equals(channelid)) {
				return ctr;
			}
			ctr++;
		}
		return ctr;
	}
	
	private static short getParentId(short channelid) {
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("channelid")).equals(channelid)) {	
				return (short)Integer.parseInt(iterator.previous().get("parentid").toString());
			}
		}
		return -1;
	}
	
	public static HashMap<String, Object> getChannel(short channelid) {
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("channelid")).equals(channelid)) {
				return data.get(iterator.previousIndex());
			}
		}
		return null;
	}
	
	public static void clear() {
		data.clear();
	}

}
