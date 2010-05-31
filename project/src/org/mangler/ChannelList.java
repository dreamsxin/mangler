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

public class ChannelList extends ListActivity {
	
	// Local variables.
	ArrayList<HashMap<String, Object>> resources = new ArrayList<HashMap<String, Object>>();
	SimpleAdapter channelAdapter;
	
	// Channel events.
	public static final int EVENT_ADD = 1;
	
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.channel_list);
        
	    // Start receiving channellist intents.
	    registerReceiver(receiver, new IntentFilter(ServerView.CHANNELLIST_ACTION));
	    
	    // Create adapter.
	    channelAdapter = new SimpleAdapter(this, resources, R.layout.channel_row, new String[] { "name", "id" }, new int[] { R.id.crowtext, R.id.crowid } );  
	    setListAdapter(channelAdapter);
	    
	    // Add lobby.
	    addChannel((short)0, "Lobby");

        registerForContextMenu(getListView());
    }
	
	private void addChannel(short id, String channelname) {
		// Add data.
		HashMap<String, Object> data = new HashMap<String, Object>();
		data.put("id", id);
		data.put("name", channelname);
		resources.add(data);
		
		// Update list.
		channelAdapter.notifyDataSetChanged();
	}
	
	public BroadcastReceiver receiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			switch(intent.getIntExtra("event", -1)) {
				case EVENT_ADD:
					addChannel(intent.getShortExtra("channelid", (short)0), intent.getStringExtra("channelname"));
					break;
			}
		}
	};
    
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        
        // Join selected channel.
        VentriloInterface.changechannel((Short)((HashMap<String, Object>)getListView().getItemAtPosition(position)).get("id"), "");
    }
}