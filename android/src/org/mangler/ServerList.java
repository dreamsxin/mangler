/*
 * This file is part of Mangler.
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
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

import com.nullwire.trace.ExceptionHandler;

import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;

public class ServerList extends ListActivity {

	private static final int ACTIVITY_CONNECT = 0;
	private static final int ACTIVITY_CREATE = 1;
	private static final int ACTIVITY_EDIT = 2;

	private static final int ADD_ID = Menu.FIRST;
	private static final int EDIT_ID = Menu.FIRST + 1;
	private static final int CLONE_ID = Menu.FIRST + 2;
    private static final int DELETE_ID = Menu.FIRST + 3;

    private static final String SERVERLIST_NOTIFICATION = "org.mangler.ServerListNotification";

	private ManglerDBAdapter dbHelper;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.server_list);

        // Send crash reports to server
        ExceptionHandler.register(this, "http://www.mangler.org/errors/upload.php");

        // Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);

        // Notification broadcast receiver.
        registerReceiver(notificationReceiver, new IntentFilter(SERVERLIST_NOTIFICATION));

        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();
    	fillData();
        registerForContextMenu(getListView());
    }

    @Override
    protected void onDestroy() {
    	super.onDestroy();

    	// Unregister notification broadcase receiver.
        unregisterReceiver(notificationReceiver);

        dbHelper.close();
    }

    // Populate listview with entries from database
    private void fillData() {
        Cursor serverCursor = dbHelper.fetchServers();
        startManagingCursor(serverCursor);

        TextView temp = (TextView)findViewById(R.id.emptyServerList);
        if (serverCursor.getCount() > 0)
        {
            temp.setVisibility(TextView.GONE);
        }
        else
        {
        	temp.setVisibility(TextView.VISIBLE);
        }

        // Display simple cursor adapter
        SimpleCursorAdapter servers = new SimpleCursorAdapter(this, R.layout.server_row, serverCursor, new String[]{ManglerDBAdapter.KEY_SERVERNAME}, new int[]{R.id.srowtext});
        setListAdapter(servers);
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, ADD_ID, 0, "Add Server").setIcon(R.drawable.menu_add);
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
		switch(item.getItemId()) {
			case EDIT_ID:
		        Intent i = new Intent(this, ServerEdit.class);
		        i.putExtra(ManglerDBAdapter.KEY_ROWID, info.id);
		        startActivityForResult(i, ACTIVITY_EDIT);
		        fillData();
		        return true;
			case CLONE_ID:
		        dbHelper.cloneServer(info.id);
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

        final ProgressDialog dialog = ProgressDialog.show(this, "", "Connecting. Please wait...", true);

        final Cursor servers = dbHelper.fetchServer(id);
        startManagingCursor(servers);

        // Get rid of any data from previous connections.
        UserList.clear();
        ChannelList.clear();

        // Add lobby.
        ChannelList.addChannel((short)0, "Lobby");

        new Thread(new Runnable(){
        	public void run(){
		        if(VentriloInterface.login(
		        		servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_HOSTNAME)) + ":" + Integer.toString( servers.getInt(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PORTNUMBER))),
		        		servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_USERNAME)),
		        		servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PASSWORD)),
		        		servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PHONETIC)))) {
		        	dialog.dismiss();
		            // Start receiving packets.
		        	startRecvThread();
		        	startActivityForResult(new Intent(ServerList.this, ServerView.class), ACTIVITY_CONNECT);
		        }
		        else {
		        	Intent broadcastIntent = new Intent(ServerList.SERVERLIST_NOTIFICATION);
		        	broadcastIntent.putExtra("notification", "Connection to server failed.");
		        	sendBroadcast(broadcastIntent);
		        }
		        dialog.dismiss();
		    }
    	}).start();
    }

    private void startRecvThread() {
    	Runnable recvRunnable = new Runnable() {
    		public void run() {
    			while (VentriloInterface.recv());
    		}
    	};
    	(new Thread(recvRunnable)).start();
    }

	private BroadcastReceiver notificationReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			Toast.makeText(ServerList.this, intent.getStringExtra("notification"), Toast.LENGTH_SHORT).show();
		}
	};

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);

        if (requestCode == ACTIVITY_CONNECT) {
        	VentriloInterface.logout();
        }

        fillData();
    }
}
