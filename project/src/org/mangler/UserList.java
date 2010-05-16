package org.mangler;

import android.app.ListActivity;
import android.os.Bundle;
import android.widget.ArrayAdapter;

public class UserList extends ListActivity {
	
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.user_list);
        
        int numUsers = -1;
        
    	Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		numUsers = extras.getInt("numusers");
    	}
        
    	fillData(numUsers);
        registerForContextMenu(getListView());
    }

    // Populate user list
    private void fillData(int numUsers) {
    	String[] usernames = new String[numUsers];
    	
    	for (int i = 1; i <= numUsers; i++) {
    		usernames[i - 1] = "User " + i;
    	}
    	
    	ArrayAdapter<String> users = new ArrayAdapter<String>(this, R.layout.user_row, R.id.urowtext, usernames);

        setListAdapter(users);
    }
}