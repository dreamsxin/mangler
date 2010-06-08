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