package org.mangler;

import android.app.TabActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.TabHost;

public class ServerView extends TabActivity {
	
	boolean stopEventThread;
	
	String hostname;
	int port;
	String password;
	String username;
	String phonetic;
	
	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.server_view);

        TabHost tabHost = getTabHost();
        TabHost.TabSpec tabSpec;
        Intent intent;
        
    	Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		hostname = extras.getString("hostname");
    		port = extras.getInt("port");
    		password = extras.getString("password");
    		username = extras.getString("username");
    		phonetic = extras.getString("phonetic");
    		
    		connect(hostname, port, password, username, phonetic);
    	}
        
        intent = new Intent().setClass(this, ChannelList.class);
        intent.putExtra("numchannels", VentriloInterface.channelcount());

        tabSpec = tabHost.newTabSpec("channels").setIndicator("Channels").setContent(intent);
        tabHost.addTab(tabSpec);

        intent = new Intent().setClass(this, UserList.class);
        intent.putExtra("numusers", VentriloInterface.usercount());
        tabSpec = tabHost.newTabSpec("users").setIndicator("Users").setContent(intent);
        tabHost.addTab(tabSpec);
        
        tabHost.setCurrentTab(0);
    }
    
    @Override
    protected void onStop() {
    	super.onStop();
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
    	// We need to start this before calling v3_login
    	startEventThread();
    	
    	VentriloInterface.debuglevel(VentriloDebugLevels.V3_DEBUG_INTERNAL);
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
