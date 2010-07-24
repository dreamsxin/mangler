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
import java.util.Iterator;
import java.util.ListIterator;

import android.util.Log;

public class ChannelList {
	
	static int USER = 1;
	static int CHANNEL = 2;

	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void add(short id, String name, int passworded, int type, short parentid) {
		HashMap<String, Object> entity = new HashMap<String, Object>();
		String indent = "";
		if (id == 0) {
			parentid = -1;
		}
		entity.put("id", id);
		for (int ctr = 0; ctr < getDepth(parentid)+1 && id != 0; ctr++) {
			indent = "    " + indent;
		}
		if (type == USER) {
			entity.put("xmitstatus", R.drawable.xmit_off);
		} else {
			entity.put("xmitstatus", R.drawable.xmit_clear);
		}
		entity.put("name", name);
		entity.put("indent", indent);
		entity.put("type", type);
		entity.put("passworded", passworded);
		entity.put("parentid", parentid);
		Log.d("mangler", "adding entity id " + id + " (" + name + ") at location " + getLocation(parentid));
		if (id == 0) {
			data.add(entity);
		} else {
			if (type == USER) {
				data.add(getLocation(parentid)+1, entity);
			} else {
				data.add(getLocation(parentid)+getChildCount(parentid)+1, entity);
			}
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
	
	private static int getLocation(short id) {
		int ctr = 0;
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			short listid =  ((Short)iterator.next().get("id"));
			if(listid == id) {
				return ctr;
			}
			ctr++;
		}
		return ctr;
	}
	
	private static short getParentId(short channelid) {
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("id")).equals(channelid)) {	
				return (short)Integer.parseInt(iterator.previous().get("parentid").toString());
			}
		}
		return -1;
	}
	
	public static HashMap<String, Object> getChannel(short channelid) {
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("id")).equals(channelid)) {
				return data.get(iterator.previousIndex());
			}
		}
		return null;
	}
	
	public static void remove(short id) {
		Log.d("mangler", "removing entity with id " + id + " at location " + getLocation(id));
		data.remove(getLocation(id));
	}
	
	public static void clear() {
		data.clear();
	}
	
	public static void updateStatus(short id, int status) {
		for(Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext(); ) {
			HashMap<String, Object> data = iterator.next();
			if((Short)data.get("id") == id) {
				data.put("xmitstatus", status);
				return;
			}
		}
	}

}
