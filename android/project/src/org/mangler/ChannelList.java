package org.mangler;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.ListIterator;

public class ChannelList {

	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void addChannel(short channelid, String channelname) {
		HashMap<String, Object> channel = new HashMap<String, Object>();
		channel.put("channelid", channelid);
		channel.put("channelname", channelname);
		data.add(channel);
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