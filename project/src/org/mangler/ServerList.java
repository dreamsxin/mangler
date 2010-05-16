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

public class ServerList extends ListActivity {
	
	private static final int ACTIVITY_CREATE = 0;
	private static final int ACTIVITY_EDIT = 1;
	
	private static final int ADD_ID = Menu.FIRST;
	private static final int EDIT_ID = Menu.FIRST + 1;
	private static final int CLONE_ID = Menu.FIRST + 2;
    private static final int DELETE_ID = Menu.FIRST + 3;
    
    
	private ManglerDBAdapter dbHelper;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.server_list);
        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();
    	fillData();
        registerForContextMenu(getListView());
    }
    
    @Override
    protected void onDestroy() {
    	super.onDestroy();
    	dbHelper.close();
    }
    
    // Populate listview with entries from database
    private void fillData() {
        Cursor serverCursor = dbHelper.fetchServers();
        startManagingCursor(serverCursor);
        
        // Fields we want to display in the list
        String[] from = new String[]{ManglerDBAdapter.KEY_SERVERNAME};
        
        // TODO: Add field to display username as well
        
        // Fields we want to bind the display fields to
        int[] to = new int[]{R.id.srowtext};
        
        // Display simple cursor adapter
        SimpleCursorAdapter servers = 
        	    new SimpleCursorAdapter(this, R.layout.server_row, serverCursor, from, to);
        setListAdapter(servers);
    }
    
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, ADD_ID, 0, "Add Server")
        	.setIcon(R.drawable.menu_add);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case ADD_ID:
        	Intent i = new Intent(this, ServerEdit.class);
       	 	startActivityForResult(i, ACTIVITY_CREATE);
            return true;
        }
        return false;
    }
    
    @Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		menu.add(0, EDIT_ID, 0, "Edit Server");
		menu.add(0, CLONE_ID, 1, "Clone Server");
		menu.add(0, DELETE_ID, 2, "Delete Server");
	}
    
    @Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		Cursor servers = dbHelper.fetchServer(info.id);
        startManagingCursor(servers);
		switch(item.getItemId()) {
		case EDIT_ID:
	        Intent i = new Intent(this, ServerEdit.class);
	        i.putExtra(ManglerDBAdapter.KEY_ROWID, info.id);
	        startActivityForResult(i, ACTIVITY_EDIT);
	        fillData();
	        return true;
		case CLONE_ID:
	        startManagingCursor(servers);
	        String servername = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERNAME));
	        String hostname = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_HOSTNAME));
	        int port = servers.getInt(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PORTNUMBER));
	        String password = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PASSWORD));
	        String username = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_USERNAME));
	        String phonetic = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PHONETIC));
			dbHelper.createServer(servername, hostname, port, password, username, phonetic);
	        fillData();
	        return true;
		case DELETE_ID:
	        dbHelper.deleteServer(info.id);
	        fillData();
	        return true;
		}
		return super.onContextItemSelected(item);
	}
    
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        Cursor servers = dbHelper.fetchServer(id);
        startManagingCursor(servers);
        
        Intent intent = new Intent(this, ServerView.class);
        intent.putExtra("hostname", servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_HOSTNAME)));
        intent.putExtra("port", servers.getInt(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PORTNUMBER)));
        intent.putExtra("password", servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PASSWORD)));
        intent.putExtra("username", servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_USERNAME)));
        intent.putExtra("phonetic", servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PHONETIC)));
        
        startActivity(intent);
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);
        fillData();
    }
}
