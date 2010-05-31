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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class UserList extends ListActivity {
	
	// Local variables.
	private ArrayList<HashMap<String, Object>> resources = new ArrayList<HashMap<String, Object>>();
	private SimpleAdapter userAdapter;
	
	// User events.
	public static final int EVENT_ADD = 1;
	public static final int EVENT_DEL = 2;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.user_list);

	    // Start receiving userlist intents.
	    registerReceiver(receiver, new IntentFilter(ServerView.USERLIST_ACTION));
        
        userAdapter = new SimpleAdapter(this, resources, R.layout.user_row, new String[] { "name", "id" }, new int[] { R.id.urowtext, R.id.urowid } );
        setListAdapter(userAdapter);
        registerForContextMenu(getListView());
    }
	
	public BroadcastReceiver receiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			short id = intent.getShortExtra("userid", (short)0);
			switch(intent.getIntExtra("event", -1)) {
				case EVENT_ADD:
					addUser(id, intent.getStringExtra("username"));
					break;
					
				case EVENT_DEL:
					delUser(id);
					break;
			}
		}
	};
	
	private void addUser(short id, String username) {
		// Add data.
		HashMap<String, Object> data = new HashMap<String, Object>();
		data.put("id", id);
		data.put("name", username);
		resources.add(data);
		
		// Update list.
		userAdapter.notifyDataSetChanged();
	}
    
	private void delUser(short id) {
		for(Iterator<HashMap<String, Object>> iterator = resources.iterator(); iterator.hasNext(); ) {
			if((Short)iterator.next().get("id") == id) {
				// Remove data.
				iterator.remove();
				
				// Update list and return.
				userAdapter.notifyDataSetChanged();
				return;
			}
		}
	}
	
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        
        // Join channel of selected user.
        VentriloInterface.changechannel(VentriloInterface.getuserchannel((Short)((HashMap<String, Object>)getListView().getItemAtPosition(position)).get("id")), "");
    }
}