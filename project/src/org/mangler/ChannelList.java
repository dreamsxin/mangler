package org.mangler;

import android.app.ListActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class ChannelList extends ListActivity {
	
	HashMap<Short, String> channelMap;
	
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.channel_list);
        
        channelMap = new HashMap<Short, String>();
        
    	Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		channelMap = (HashMap<Short, String>)extras.getSerializable("channels");
    	}
        
    	fillData();
        registerForContextMenu(getListView());
    }
    
    // Populate channel list
    private void fillData() {
    	ArrayList<HashMap<String, Object>> resources = new ArrayList<HashMap<String, Object>>();

    	HashMap<String, Object> data;
       
    	Iterator<Short> iterator = channelMap.keySet().iterator();

    	while (iterator.hasNext()) {
    		data = new HashMap<String, Object>();
    		short id = iterator.next();
    		data.put("name", channelMap.get(id));
    		data.put("id", id);
    		resources.add(data);
    	}
       
    	SimpleAdapter channelAdapter = new SimpleAdapter(this, resources, R.layout.channel_row, new String[] { "name", "id" }, new int[] { R.id.crowtext, R.id.crowid } );
       
    	setListAdapter(channelAdapter);
    }
    
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        HashMap<String, Object> temp = (HashMap<String, Object>)getListView().getItemAtPosition(position); 
        
        VentriloInterface.changechannel((Short)temp.get("id"), "");
    }
}