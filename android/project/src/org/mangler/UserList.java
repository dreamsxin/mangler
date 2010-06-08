package org.mangler;

import java.util.HashMap;
import java.util.Iterator;

public class UserList extends DataList<UserList> {
	
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
