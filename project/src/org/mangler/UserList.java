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
	
	HashMap<Short, String> userMap;
	
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.user_list);
        
        userMap = new HashMap<Short, String>();
        
    	Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		userMap = (HashMap<Short, String>)extras.getSerializable("users");
    	}
        
    	fillData();
        registerForContextMenu(getListView());
    }

    // Populate user list
    private void fillData() {
    	ArrayList<HashMap<String, Object>> resources = new ArrayList<HashMap<String, Object>>();

    	HashMap<String, Object> data;
       
    	Iterator<Short> iterator = userMap.keySet().iterator();

    	while (iterator.hasNext()) {
    		data = new HashMap<String, Object>();
    		short id = iterator.next();
    		data.put("name", userMap.get(id));
    		data.put("id", id);
    		resources.add(data);
    	}
       
    	SimpleAdapter userAdapter = new SimpleAdapter(this, resources, R.layout.user_row, new String[] { "name", "id" }, new int[] { R.id.urowtext, R.id.urowid } );
       
    	setListAdapter(userAdapter);
    }
    
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        HashMap<String, Object> temp = (HashMap<String, Object>)getListView().getItemAtPosition(position); 
        
        VentriloInterface.changechannel(VentriloInterface.getuserchannel((Short)temp.get("id")), "");
    }
}