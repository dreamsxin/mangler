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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class UserList {
	
	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void delUser(short userid) {
		for(Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("userid")).equals(userid)) {
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
		user.put("userid", userid);
		user.put("username", username);
		user.put("channelid", channelid);
		user.put("channelname", ChannelList.getChannel(channelid).get("channelname"));
		data.add(user);
	}
	
	public static void changeChannel(short userid, short channelid) {
		for(Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext(); ) {
			HashMap<String, Object> data = iterator.next();
			if((Short)data.get("userid") == userid) {
				data.put("channelid", channelid);
				data.put("channelname", ChannelList.getChannel(channelid).get("channelname"));
				return;
			}
		}
	}

}
