package org.mangler;

import java.util.ArrayList;
import java.util.HashMap;

public class ChannelList {

	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void addChannel(short channelid, String channelname) {
		HashMap<String, Object> channel = new HashMap<String, Object>();
		channel.put("channelid", channelid);
		channel.put("channelname", channelname);
		data.add(channel);
	}
	
	public static void clear() {
		data.clear();
	}

}