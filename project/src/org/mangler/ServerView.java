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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import android.app.TabActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.SimpleAdapter;
import android.widget.TextView;

public class ServerView extends TabActivity {
	
	// Events.
	public static final int EVENT_CHAT_JOIN	  = 1;
	public static final int EVENT_CHAT_LEAVE  = 2;
	public static final int EVENT_CHAT_MSG	  = 3;
	public static final int EVENT_USER_ADD    = 4;
	public static final int EVENT_USER_DEL	  = 5;
	public static final int EVENT_CHANNEL_ADD = 6;
	
	// Menu options.
	private final int OPTION_JOIN_CHAT  = 1;
	private final int OPTION_LEAVE_CHAT = 2;
	
	// List adapters.
	private SimpleAdapter channelAdapter;
	private SimpleAdapter userAdapter;
	
	// List containers.
	private ArrayList<HashMap<String, Object>> channelData 	= new ArrayList<HashMap<String, Object>>();
	private ArrayList<HashMap<String, Object>> userData 	= new ArrayList<HashMap<String, Object>>();
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.server_view);
        
        // Create adapters.
	    channelAdapter 	= new SimpleAdapter(this, channelData, R.layout.channel_row, new String[] { "name", "id" }, new int[] { R.id.crowtext, R.id.crowid } );  
	    userAdapter 	= new SimpleAdapter(this, userData, R.layout.user_row, new String[] { "name", "id" }, new int[] { R.id.urowtext, R.id.urowid } );
        
        // Register receivers.
        registerReceiver(chatReceiver, new IntentFilter(ReceiverIntents.CHATVIEW_ACTION));
        registerReceiver(channelReceiver, new IntentFilter(ReceiverIntents.CHANNELLIST_ACTION));
        registerReceiver(userReceiver, new IntentFilter(ReceiverIntents.USERLIST_ACTION));
        
        // Control listeners.
	    ((EditText)findViewById(R.id.message)).setOnKeyListener(onChatMessageEnter);
	    ((Button)findViewById(R.id.talkButton)).setOnClickListener(onTalkPress);
        
        // Start receiving packets.
    	startRecvThread();
    }

    public boolean onCreateOptionsMenu(Menu menu) {
    	 // Create our menu buttons.
    	menu.add(0, OPTION_JOIN_CHAT, 0, "Join chat").setIcon(R.drawable.menu_join_chat);
        menu.add(0, OPTION_LEAVE_CHAT, 0, "Leave chat").setIcon(R.drawable.menu_leave_chat);
        return true;
    }
    
    public boolean onOptionsItemSelected(MenuItem item) {
    	// Handle menu buttons.
    	final EditText message = (EditText)findViewById(R.id.message);
        switch(item.getItemId()) {
        	case OPTION_JOIN_CHAT:
        		VentriloInterface.joinchat();
        		message.setEnabled(true);
        		break;
        	
        	case OPTION_LEAVE_CHAT:
        		VentriloInterface.leavechat();
        		message.setEnabled(false);
        		break;
        		
        	default:
        		return false;
        }
        return true;
    }
    
    private void startRecvThread() {
    	Runnable recvRunnable = new Runnable() {
    		public void run() {
    			while(true) {
    				if(!VentriloInterface.recv()) {
    					break;
    				}
    			}
    		}
    	};
    	(new Thread(recvRunnable)).start();
    }
    
	private BroadcastReceiver chatReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			final TextView messages = (TextView)findViewById(R.id.messages);
			switch(intent.getIntExtra("event", -1)) {
				case EVENT_CHAT_JOIN:
					messages.append("\n* " + intent.getStringExtra("username") + " has joined the chat.");
					break;
					
				case EVENT_CHAT_LEAVE:
					messages.append("\n* " + intent.getStringExtra("username") + " has left the chat.");
					break;
			 		
			 	case EVENT_CHAT_MSG:
			 		messages.append("\n" + intent.getStringExtra("username") + ": " + intent.getStringExtra("message"));
			 		break;
			 }
			
			// Scroll to bottom.
			final ScrollView chatscroll = (ScrollView)findViewById(R.id.chatScroll);
			chatscroll.post(new Runnable() {
				public void run() {
					chatscroll.fullScroll(ScrollView.FOCUS_DOWN); 
				}
			});
		 }
	 };
	 
	private BroadcastReceiver userReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			short id = intent.getShortExtra("userid", (short)0);
			switch(intent.getIntExtra("event", -1)) {
				case EVENT_USER_ADD:
					addUser(id, intent.getStringExtra("username"));
					break;
						
				case EVENT_USER_DEL:
					delUser(id);
					break;
			}
		}
	};
	 
	private BroadcastReceiver channelReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			switch(intent.getIntExtra("event", -1)) {
				case EVENT_CHANNEL_ADD:
					addChannel(intent.getShortExtra("channelid", (short)0), intent.getStringExtra("channelname"));
					break;
			}
		}
	};

	
	private OnClickListener onTalkPress = new OnClickListener() {
		public void onClick(View v) {
			/*if (!recorder.recording()) {
				recorder.start();
				talkButton.setText(R.string.stop_talk);
			} else {
				recorder.stop();
				talkButton.setText(R.string.start_talk);
			}*/
		}
	};
	 
	private OnKeyListener onChatMessageEnter = new OnKeyListener() {
		public boolean onKey(View v, int keyCode, KeyEvent event) {
			if ((event.getAction() == KeyEvent.ACTION_DOWN) && (keyCode == KeyEvent.KEYCODE_ENTER)) {
				// Send chat message.
				final EditText message = (EditText)findViewById(R.id.message);
				VentriloInterface.sendchatmessage(message.getText().toString());
				
				// Clear message field.
				message.setText("");
				
				// Hide keyboard.
	           ((InputMethodManager)getSystemService(INPUT_METHOD_SERVICE)).hideSoftInputFromWindow(message.getWindowToken(), 0);
				return true;
			}
			return false;
		}
	};
		
	private void addUser(short id, String username) {
		// Add data.
		HashMap<String, Object> data = new HashMap<String, Object>();
		data.put("id", id);
		data.put("name", username);
		userData.add(data);
		
		// Update list.
		userAdapter.notifyDataSetChanged();
	}
    
	private void delUser(short id) {
		for(Iterator<HashMap<String, Object>> iterator = userData.iterator(); iterator.hasNext(); ) {
			if((Short)iterator.next().get("id") == id) {
				// Remove data.
				iterator.remove();
				
				// Update list and return.
				userAdapter.notifyDataSetChanged();
				return;
			}
		}
	}
	
	private void addChannel(short id, String channelname) {
		// Add data.
		HashMap<String, Object> data = new HashMap<String, Object>();
		data.put("id", id);
		data.put("name", channelname);
		channelData.add(data);
		
		// Update list.
		channelAdapter.notifyDataSetChanged();
	}
	
}
