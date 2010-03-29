package org.mangler;

import android.app.ListActivity;
import android.database.Cursor;
import android.os.Bundle;
import android.util.Log;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.AdapterView.AdapterContextMenuInfo;

public class ServerList extends ListActivity {
	
	private static final int EDIT_ID = Menu.FIRST;
    private static final int DELETE_ID = Menu.FIRST + 1;
    
	private ManglerDBAdapter dbHelper;
	
	private Long userID;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.server_list);
        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();
        
        // Fetch userID that was passed in from UserList activity
        if (userID == null)
        {
        	Bundle extras = getIntent().getExtras();   
        	userID = extras.getLong(ManglerDBAdapter.KEY_USERID);
        }
        
        fillData();
        registerForContextMenu(getListView());
    }
    
    // Populate listview with entries from database
    private void fillData() {
        Cursor serverCursor = dbHelper.fetchServersForUser(userID);
        startManagingCursor(serverCursor);
        
        // Fields we want to display in the list
        String[] from = new String[]{ManglerDBAdapter.KEY_SERVERNAME};
        
        // Fields we want to bind the display fields to
        int[] to = new int[]{R.id.srowtext};
        
        // Display simple cursor adapter
        SimpleCursorAdapter servers = 
        	    new SimpleCursorAdapter(this, R.layout.server_row, serverCursor, from, to);
        setListAdapter(servers);
    }
    
    @Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		menu.add(0, EDIT_ID, 0, "Edit Server");
		menu.add(0, DELETE_ID, 1, "Delete Server");
	}
    
    @Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		switch(item.getItemId()) {
		case EDIT_ID:
	        dbHelper.deleteServer(info.id);
	        fillData();
	        return true;
		case DELETE_ID:
	        dbHelper.deleteServer(info.id);
	        fillData();
	        return true;
		}
		return super.onContextItemSelected(item);
	}
    
    private String StringFromBytes(byte[] bytes) {
    	return new String(bytes, 0, (new String(bytes).indexOf(0)));
    }
    
    // Currently connects to server immediately
    // TODO: Move into seperate activity later
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        Cursor servers = dbHelper.fetchServer(id);
        Cursor users = dbHelper.fetchUser(servers.getLong(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_USERID)));
        startManagingCursor(servers);
        String username = users.getString(users.getColumnIndexOrThrow(ManglerDBAdapter.KEY_USERNAME));
        String phonetic = users.getString(users.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PHONETIC));
        String hostname = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_HOSTNAME));
        int port = servers.getInt(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PORTNUMBER));
        
        // We need to start this -before- calling v3_login. 
    	Runnable event_runnable = new Runnable() {
    		public void run() {
		    	VentriloEventData data = new VentriloEventData();
		    	while(true) {
		    		VentriloInterface.getevent(data);
		    		switch(data.type) {
		    			case VentriloEvents.V3_EVENT_CHAT_MESSAGE:
		    				Log.i("mangler", 
		    						"User sent message: " + Integer.toString(data.user.id) +
		    						" -> " + StringFromBytes(data.data.chatmessage));
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_CHAT_JOIN:
		    				Log.i("mangler", 
		    						"User joined chat: " + Integer.toString(data.user.id));
		    				break;
		    			
		    			case VentriloEvents.V3_EVENT_PING:
		    				Log.i("mangler", 
		    						"Ping: " + Integer.toString(data.ping));
		    			break;
		    			
		    			case VentriloEvents.V3_EVENT_USER_TALK_START:
		    				Log.i("mangler", 
		    						"User started talking: " + Integer.toString(data.user.id) + 
		    						" - Rate: " + Integer.toString(data.pcm.rate));
		    			break;
		    			
		    			case VentriloEvents.V3_EVENT_USER_TALK_END:
		    				Log.i("mangler", 
		    						"User stopped talking: " + Integer.toString(data.user.id));
			    			break;
			    			
		    			case VentriloEvents.V3_EVENT_STATUS:
		    				Log.i("mangler", 
		    						"Status: " + Integer.toString(data.status.percent) + "%" +
		    						" -> " + StringFromBytes(data.status.message));
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_CHAN_ADD:
		    			case VentriloEvents.V3_EVENT_CHAN_MODIFY:
		    				VentriloInterface.getchannel(data, data.channel.id);
		    				Log.i("mangler", 
		    						"Channel added / modified: " + Short.toString(data.channel.id) +
		    						" -> " + StringFromBytes(data.text.name));
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_USER_LOGIN:
		    				VentriloInterface.getuser(data, data.user.id);
		    				Log.i("mangler", 
		    						"User logged in: " + Short.toString(data.user.id) +
		    						" -> " + StringFromBytes(data.text.name));
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_LOGIN_COMPLETE:
		    				Log.i("mangler",
		    						"Login complete!");
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_DISPLAY_MOTD:
		    				Log.i("mangler",
		    						"MOTD: " + StringFromBytes(data.data.motd));
		    					
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
		    				Log.i("mangler",
		    						"User joined channel: " + Integer.toString(data.user.id) +
		    						" - channel: " + Integer.toString(data.channel.id));
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_USER_LOGOUT:
		    				Log.i("mangler",
		    						"User left server: " + Integer.toString(data.user.id));
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_CHAT_LEAVE:
		    				Log.i("mangler",
		    						"User left chat: " + Integer.toString(data.user.id));
		    				break;
		    			
		    			default:
		    				Log.w("mangler", 
		    						"Unknown event of type: " + Integer.toString(data.type));
		    		}
		    	}
    		}
    	};
    	
    	Runnable recv_runnable = new Runnable() {
    		public void run() {
    			while(true) {
    				if(!VentriloInterface.recv()) break;
    			}
    		}
    	};
        
    	(new Thread(event_runnable)).start();
    	VentriloInterface.debuglevel(VentriloDebugLevels.V3_DEBUG_ALL);
        if(VentriloInterface.login(hostname + ":" + port, username, "", phonetic)) {
	    	(new Thread(recv_runnable)).start();
        }
    }
}
