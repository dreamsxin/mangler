/*
 * This file is part of Mangler.
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

import android.app.ListActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class UserList extends ListActivity {
	
	ArrayList<HashMap<String, Object>> resources = new ArrayList<HashMap<String, Object>>();
	SimpleAdapter userAdapter;
	
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.user_list);

        // Create empty adapter.
        userAdapter = new SimpleAdapter(this, resources, R.layout.user_row, new String[] { "name", "id" }, new int[] { R.id.urowtext, R.id.urowid } );
        setListAdapter(userAdapter);
        
        // Grab users (if any) and add them to our resources.
    	Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		@SuppressWarnings("unchecked")
			HashMap<Short, String> userMap = (HashMap<Short, String>)extras.getSerializable("users");
			if(userMap != null) {
				Iterator<Short> iterator = userMap.keySet().iterator();
		    	while(iterator.hasNext()) {
		    		short id = iterator.next();
		    		if(id > 0) {
		    			addUser(id, userMap.get(id));
		    		}
		    	}
			}
    	}

        registerForContextMenu(getListView());
    }
	
	public void addUser(int id, String username) {
		// Add data.
		HashMap<String, Object> data = new HashMap<String, Object>();
		data.put("name", username);
		data.put("id", id);
		resources.add(data);
		
		// Update list.
		userAdapter.notifyDataSetChanged();
	}
    
	public void delUser(int id) {
		for(Iterator<HashMap<String, Object>> iterator = resources.iterator(); iterator.hasNext(); ) {
			if((Integer)iterator.next().get("id") == id) {
				// Remove data and update list.
				iterator.remove();
				userAdapter.notifyDataSetChanged();
				return;
			}
		}
	}
	
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        HashMap<String, Object> temp = (HashMap<String, Object>)getListView().getItemAtPosition(position); 
        
        VentriloInterface.changechannel(VentriloInterface.getuserchannel((Short)temp.get("id")), "");
    }
}