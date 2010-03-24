package org.mangler;

import android.app.ListActivity;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.AdapterView.AdapterContextMenuInfo;

public class UserList extends ListActivity {

	private static final int EDIT_ID = Menu.FIRST;
    private static final int DELETE_ID = Menu.FIRST + 1;
    
	private ManglerDBAdapter dbHelper;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.user_list);
        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();
        fillData();
        registerForContextMenu(getListView());
    }
    
    // Populate listview with entries from database
    private void fillData() {
        Cursor userCursor = dbHelper.fetchAllUsers();
        startManagingCursor(userCursor);

        // Fields we want to display in the list
        String[] from = new String[]{ManglerDBAdapter.KEY_USERNAME};
        
        // Fields we want to bind the display fields to
        int[] to = new int[]{R.id.urowtext};
        
        // Display simple cursor adapter
        SimpleCursorAdapter users = 
        	    new SimpleCursorAdapter(this, R.layout.user_row, userCursor, from, to);
        setListAdapter(users);
    }
    
    @Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		menu.add(0, EDIT_ID, 0, "Edit User");
		menu.add(0, DELETE_ID, 1, "Delete User");
	}
    
    @Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		switch(item.getItemId()) {
		case EDIT_ID:
	        dbHelper.deleteUser(info.id);
	        fillData();
	        return true;
		case DELETE_ID:
	        dbHelper.deleteUser(info.id);
	        fillData();
	        return true;
		}
		return super.onContextItemSelected(item);
	}
    
    // Starts ServerList activity and passes the userID of selected user
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        Intent i = new Intent(this, ServerList.class);
        i.putExtra(ManglerDBAdapter.KEY_USERID, id);
        startActivity(i);
    }
}
