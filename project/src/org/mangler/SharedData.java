package org.mangler;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class SharedData {

	public static ArrayList<HashMap<String, Object>> channelData = new ArrayList<HashMap<String, Object>>();
	public static ArrayList<HashMap<String, Object>> userData	 = new ArrayList<HashMap<String, Object>>();
	
	public static void addData(ArrayList<HashMap<String, Object>> container, short id, String name) {
		HashMap<String, Object> data = new HashMap<String, Object>();
		data.put("id", id);
		data.put("name", name);
		container.add(data);
	}
	
	public static void delData(ArrayList<HashMap<String, Object>> container, short id) {
		for(Iterator<HashMap<String, Object>> iterator = container.iterator(); iterator.hasNext(); ) {
			if((Short)iterator.next().get("id") == id) {
				iterator.remove();
				return;
			}
		}
	}
	
	public static void clearAll() {
		channelData.clear();
		userData.clear();
	}
	
}
