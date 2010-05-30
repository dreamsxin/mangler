/*
 * $LastChangedDate: 2010-05-28 20:39:54 +0200 (Fri, 28 May 2010) $
 * $Revision: 846 $
 * $LastChangedBy: clearscreen $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/libventrilo3.c $
 *
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
    		short id = iterator.next();
    		if(id > 0) {
	    		data = new HashMap<String, Object>();
	    		data.put("name", userMap.get(id));
	    		data.put("id", id);
	    		resources.add(data);
    		}
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