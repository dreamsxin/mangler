/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-06-20 07:49:37 +0200 (Sun, 20 Jun 2010) $
 * $Revision: 950 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/EventService.java $
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

package org.mangler.android;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

public class EventService extends Service {

	private final IBinder binder = new EventBinder();
	private boolean running = false;

	public class EventBinder extends Binder {
		EventService getService() {
			return EventService.this;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return binder;
	}

    @Override
    public void onCreate() {
    	running = true;
    	Thread t = new Thread(eventRunnable);
        t.setPriority(10);
        t.start();
    }

    @Override
    public void onDestroy() {
    	running = false;
    }

    public static String StringFromBytes(byte[] bytes) {
    	return new String(bytes, 0, (new String(bytes).indexOf(0)));
    }
    
    private String getPhonetic(short userid) {
    	String username = "unknown";
		VentriloEventData userRet = new VentriloEventData();
		VentriloInterface.getuser(userRet, userid);
		String phonetic = StringFromBytes(userRet.text.phonetic);
		if (phonetic.length() > 0) {
			username = phonetic;
		} else {
			username = StringFromBytes(userRet.text.name);
		}
    	return username;
    }

    private Runnable eventRunnable = new Runnable() {

    	private Intent broadcastIntent;
    	private VentriloEventData data = new VentriloEventData();

		public void run() {
	    	while (running) {
	    		VentriloInterface.getevent(data);
	    		switch (data.type) {
	    			case VentriloEvents.V3_EVENT_CHAT_MESSAGE:
	    				VentriloInterface.getuser(data, data.user.id);
	    				broadcastIntent = new Intent(ServerView.CHATVIEW_ACTION);
	    				broadcastIntent.putExtra("event", ServerView.EVENT_CHAT_MSG);
	    				broadcastIntent.putExtra("username", StringFromBytes(data.text.name));
	    				broadcastIntent.putExtra("message", StringFromBytes(data.data.chatmessage));
	    			    sendBroadcast(broadcastIntent);
	    				break;

	    			case VentriloEvents.V3_EVENT_CHAT_JOIN:
	    				VentriloInterface.getuser(data, data.user.id);
	    				broadcastIntent = new Intent(ServerView.CHATVIEW_ACTION);
	    				broadcastIntent.putExtra("event", ServerView.EVENT_CHAT_JOIN);
	    				broadcastIntent.putExtra("username", StringFromBytes(data.text.name));
	    			    sendBroadcast(broadcastIntent);
	    				break;

	    			case VentriloEvents.V3_EVENT_CHAT_LEAVE:
	    				VentriloInterface.getuser(data, data.user.id);
	    				broadcastIntent = new Intent(ServerView.CHATVIEW_ACTION);
	    				broadcastIntent.putExtra("event", ServerView.EVENT_CHAT_LEAVE);
	    				broadcastIntent.putExtra("username", StringFromBytes(data.text.name));
	    			    sendBroadcast(broadcastIntent);
	    				break;

	    			case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
    					if (data.user.id == VentriloInterface.getuserid()) {
   							Player.clear();
    						Recorder.rate(VentriloInterface.getchannelrate(data.channel.id));
    						String message;
    						if (data.channel.id == 0) {
    							message = "Changed to Lobby";
    						} else {
    							VentriloEventData channelData = new VentriloEventData();
        						VentriloInterface.getchannel(channelData, data.channel.id);
        						message = "Changed to " + StringFromBytes(channelData.text.name);
    						}
    	    				broadcastIntent = new Intent(ServerView.NOTIFY_ACTION);
    	    				broadcastIntent.putExtra("message", message);
    	    			    sendBroadcast(broadcastIntent);
    					} else {
    	    				if (data.channel.id == VentriloInterface.getuserchannel(VentriloInterface.getuserid())) {
    	    					broadcastIntent = new Intent(ServerView.TTS_NOTIFY_ACTION);
    	    					String phonetic = getPhonetic(data.user.id);
    	    					broadcastIntent.putExtra("message", phonetic + " has joined the channel.");
    	    					sendBroadcast(broadcastIntent);
    	    				} else if (UserList.getChannel(data.user.id) == VentriloInterface.getuserchannel(VentriloInterface.getuserid())) {
    	    					broadcastIntent = new Intent(ServerView.TTS_NOTIFY_ACTION);
    	    					String phonetic = getPhonetic(data.user.id);
    	    					broadcastIntent.putExtra("message", phonetic+ " has left the channel.");
    	    					sendBroadcast(broadcastIntent);
    	    				}
    						Player.close(data.user.id);
    					}
    					UserList.changeChannel(data.user.id, data.channel.id);
    					sendBroadcast(new Intent(ServerView.USERLIST_ACTION));
	    				break;

	    			case VentriloEvents.V3_EVENT_USER_LOGIN:
	    				if (data.user.id != 0) {
		    				VentriloInterface.getuser(data, data.user.id);
		    				String username = StringFromBytes(data.text.name);
		    				UserList.addUser(data.user.id, username, data.channel.id);
		    			    sendBroadcast(new Intent(ServerView.USERLIST_ACTION));
	    					broadcastIntent = new Intent(ServerView.TTS_NOTIFY_ACTION);
	    					String phonetic = getPhonetic(data.user.id);
	    					broadcastIntent.putExtra("message", phonetic + " has logged in.");
	    					sendBroadcast(broadcastIntent);
	    				}
	    				break;

	    			case VentriloEvents.V3_EVENT_USER_LOGOUT:
	    				Player.close(data.user.id);
	    				UserList.delUser(data.user.id);
	    			    sendBroadcast(new Intent(ServerView.USERLIST_ACTION));
	    			    // the phonetic isn't available after the user logged out :/
    					String phonetic = StringFromBytes(data.text.name);
    					broadcastIntent = new Intent(ServerView.TTS_NOTIFY_ACTION);
    					broadcastIntent.putExtra("message", phonetic + " has logged out.");
    					sendBroadcast(broadcastIntent);
	    				break;

	    			case VentriloEvents.V3_EVENT_LOGIN_COMPLETE:
	    				Recorder.rate(VentriloInterface.getchannelrate(VentriloInterface.getuserchannel(VentriloInterface.getuserid())));
	    				break;

	    			case VentriloEvents.V3_EVENT_PLAY_AUDIO:
	    				Player.write(data.user.id, data.pcm.rate, data.pcm.channels, data.data.sample, data.pcm.length);
	    				UserList.updateStatus(data.user.id, R.drawable.transmit_on);
	    				sendBroadcast(new Intent(ServerView.USERLIST_ACTION));
	    				break;

	    			case VentriloEvents.V3_EVENT_USER_TALK_START:
	    				UserList.updateStatus(data.user.id, R.drawable.transmit_init);
	    				sendBroadcast(new Intent(ServerView.USERLIST_ACTION));
	    				break;
	    				
	    			case VentriloEvents.V3_EVENT_USER_TALK_END:
	    			case VentriloEvents.V3_EVENT_USER_TALK_MUTE:
	    			case VentriloEvents.V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
	    			case VentriloEvents.V3_EVENT_USER_CHANNEL_MUTE_CHANGED:
	    				Player.close(data.user.id);
	    				UserList.updateStatus(data.user.id, R.drawable.transmit_off);
	    				sendBroadcast(new Intent(ServerView.USERLIST_ACTION));
	    				break;

	    			case VentriloEvents.V3_EVENT_CHAN_ADD:
	    				VentriloInterface.getchannel(data, data.channel.id);
	    				ChannelList.addChannel(data.channel.id, StringFromBytes(data.text.name), VentriloInterface.channelrequirespassword(data.channel.id));
	    			    sendBroadcast(new Intent(ServerView.CHANNELLIST_ACTION));
	    				break;

	    			default:
	    				Log.w("mangler", "Unhandled event type: " + Integer.toString(data.type));
	    		}
	    	}
	    	Player.clear();
	    	Recorder.stop();
		}
    };

}
