package org.mangler;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class DataList<T> {

	protected static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void clear() {
		data.clear();
	}
	
	protected static void deleteByMatch(String index, Object value) {
		for(Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get(index)).equals(value)) {
				iterator.remove();
				return;
			}
		}
	}
	
}
