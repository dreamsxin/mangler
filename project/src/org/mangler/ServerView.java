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

import android.app.TabActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TabHost;

import java.util.HashMap;

public class ServerView extends TabActivity {
	
	// Actions.
	public static final String CHANNELLIST_ACTION = "org.mangler.ChannelListUpdateEvent";
	public static final String USERLIST_ACTION 	  = "org.mangler.UserListUpdateEvent";
	public static final String CHAT_ACTION		  = "org.mangler.ChatUpdateEvent";
	
	// Local variables.
	private boolean  stopEventThread;
	private String 	 hostname;
	private int 	 port;
	private String 	 password;
	private String 	 username;
	private String 	 phonetic;
	private Player 	 player;
	private Recorder recorder;
	private Intent	 broadcastIntent;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.server_view);

        // Talk button.
    	final Button ttalk = (Button)findViewById(R.id.ToggleTalkButton);
    	ttalk.setText(R.string.start_talk);
        ttalk.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				if (!recorder.recording()) {
					recorder.start();
					ttalk.setText(R.string.stop_talk);
				} else {
					recorder.stop();
					ttalk.setText(R.string.start_talk);
				}
			}
        });
        
        // Tabs.
        TabHost tabHost = getTabHost();
        TabHost.TabSpec tabSpec;
        Intent intent;
        
        // Talk tab.
    	tabSpec = tabHost.newTabSpec("talk").setIndicator("Talk").setContent(R.id.talkView);
        tabHost.addTab(tabSpec);
    	
        // Channel tab.
        intent = new Intent().setClass(this, ChannelList.class);
        tabSpec = tabHost.newTabSpec("channels").setIndicator("Channels").setContent(intent);
        tabHost.addTab(tabSpec);

        // User tab.
        intent = new Intent().setClass(this, UserList.class);
        tabSpec = tabHost.newTabSpec("users").setIndicator("Users").setContent(intent);
        tabHost.addTab(tabSpec);
        
        // Chat tab.
        intent = new Intent().setClass(this, ChatView.class);
        tabSpec = tabHost.newTabSpec("chat").setIndicator("Chat").setContent(intent);
        tabHost.addTab(tabSpec);
        
        // Load activities that need to register broadcastreceivers.
        // I'm sure we can do this in a better way.
        tabHost.setCurrentTab(3);
        tabHost.setCurrentTab(2);
        tabHost.setCurrentTab(1);
        tabHost.setCurrentTab(0);
        
        // Connect to server.
    	Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		hostname = extras.getString("hostname");
    		port = extras.getInt("port");
    		password = extras.getString("password");
    		username = extras.getString("username");
    		phonetic = extras.getString("phonetic");
    		connect(hostname, port, password, username, phonetic);
    	}
    }
    
    @Override
    protected void onStop() {
    	super.onStop();
    	recorder.stop();
    	stopEventThread = true;
    	VentriloInterface.logout();
    }
    
    @Override
    protected void onRestart() {
    	super.onRestart();
    	stopEventThread = false;
    	connect(hostname, port, password, username, phonetic);
    }
    
    private String StringFromBytes(byte[] bytes) {
    	return new String(bytes, 0, (new String(bytes).indexOf(0)));
    }
    
    protected void connect(String hostname, int port, String password, String username, String phonetic) {
    	startEventThread();
    	
    	VentriloInterface.debuglevel(VentriloDebugLevels.V3_DEBUG_INFO);
        if(VentriloInterface.login(hostname + ":" + port, username, password, phonetic)) {
	    	startRecvThread();
        }
    }
    
    private void startEventThread() {
    	Runnable event_runnable = new Runnable() {
    		public void run() {
		    	VentriloEventData data = new VentriloEventData();
		    	while(true) {
		    		VentriloInterface.getevent(data);
		    		switch(data.type) {
		    			case VentriloEvents.V3_EVENT_CHAT_MESSAGE:
		    				VentriloInterface.getuser(data, data.user.id);
		    				broadcastIntent = new Intent(CHAT_ACTION);
		    				broadcastIntent.putExtra("event", ChatView.EVENT_MSG);
		    				broadcastIntent.putExtra("username", StringFromBytes(data.text.name));
		    				broadcastIntent.putExtra("message", StringFromBytes(data.data.chatmessage));
		    			    sendBroadcast(broadcastIntent);
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_CHAT_JOIN:
		    				VentriloInterface.getuser(data, data.user.id);
		    				broadcastIntent = new Intent(CHAT_ACTION);
		    				broadcastIntent.putExtra("event", ChatView.EVENT_JOIN);
		    				broadcastIntent.putExtra("username", StringFromBytes(data.text.name));
		    			    sendBroadcast(broadcastIntent);
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_CHAT_LEAVE:
		    				VentriloInterface.getuser(data, data.user.id);
		    				broadcastIntent = new Intent(CHAT_ACTION);
		    				broadcastIntent.putExtra("event", ChatView.EVENT_LEAVE);
		    				broadcastIntent.putExtra("username", StringFromBytes(data.text.name));
		    			    sendBroadcast(broadcastIntent);
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
	    					if(data.user.id == VentriloInterface.getuserid()) {
	    						int channel_rate = VentriloInterface.getchannelrate(data.channel.id);
	    						player.rate(channel_rate);
	    						recorder.stop();
	    						recorder = new Recorder(channel_rate);
	    					}
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_USER_LOGIN:
		    				if(data.user.id != 0) {
			    				VentriloInterface.getuser(data, data.user.id);
			    				broadcastIntent = new Intent(USERLIST_ACTION);
			    				broadcastIntent.putExtra("event", UserList.EVENT_ADD);
			    				broadcastIntent.putExtra("userid", data.user.id);
			    				broadcastIntent.putExtra("username", StringFromBytes(data.text.name));
			    			    sendBroadcast(broadcastIntent);
		    				}
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_USER_LOGOUT:
		    				broadcastIntent = new Intent(USERLIST_ACTION);
		    				broadcastIntent.putExtra("event", UserList.EVENT_DEL);
		    				broadcastIntent.putExtra("userid", data.user.id);
		    			    sendBroadcast(broadcastIntent);
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_LOGIN_COMPLETE:
		    				int lobby_rate = VentriloInterface.getchannelrate((short)0);
		    				player = new Player(lobby_rate);
		    				recorder = new Recorder(lobby_rate);
		    				break;
		    				
		    			case VentriloEvents.V3_EVENT_PLAY_AUDIO:
		    				player.write(data.data.sample, data.pcm.length);
		    				break;
		
		    			case VentriloEvents.V3_EVENT_CHAN_ADD:
		    				VentriloInterface.getchannel(data, data.channel.id);
		    				broadcastIntent = new Intent(CHANNELLIST_ACTION);
		    				broadcastIntent.putExtra("event", ChannelList.EVENT_ADD);
		    				broadcastIntent.putExtra("channelid", data.channel.id);
		    				broadcastIntent.putExtra("channelname", StringFromBytes(data.text.name));
		    			    sendBroadcast(broadcastIntent);
		    				break;
		    				
		    			default:
		    				Log.w("mangler", "Unhandled event of type: " + Integer.toString(data.type));
		    		}
		    		
		    		if (stopEventThread) break;
		    	}
    		}
    	};
    	(new Thread(event_runnable)).start();
    }
    
    private void startRecvThread() {
    	Runnable recv_runnable = new Runnable() {
    		public void run() {
    			while(true) {
    				if(!VentriloInterface.recv()) break;
    			}
    		}
    	};
    	
    	(new Thread(recv_runnable)).start();
    }
}
