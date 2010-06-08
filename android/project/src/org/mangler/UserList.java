package org.mangler;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class UserList {
	
	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	private static void deleteByMatch(String index, Object value) {
		for(Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get(index)).equals(value)) {
				iterator.remove();
				return;
			}
		}
	}
	
	public static void clear() {
		data.clear();
	}
	
	public static void addUser(short userid, String username, short channelid, String channelname) {
		HashMap<String, Object> user = new HashMap<String, Object>();
		user.put("userid", userid);
		user.put("username", username);
		user.put("channelid", channelid);
		user.put("channelname", channelname);
		data.add(user);
	}
	
	public static void changeChannel(short userid, short channelid, String channelname) {
		for(Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext(); ) {
			HashMap<String, Object> data = iterator.next();
			if((Short)data.get("userid") == userid) {
				data.put("channelid", channelid);
				data.put("channelname", channelname);
				return;
			}
		}
	}
	
	public static void delUser(short userid) {
		deleteByMatch("userid", userid);
	}

}