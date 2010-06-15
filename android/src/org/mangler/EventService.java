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
    	(new Thread(eventRunnable)).start();
    }

    @Override
    public void onDestroy() {
    	running = false;
    }

    private Runnable eventRunnable = new Runnable() {

    	private Intent broadcastIntent;
    	private VentriloEventData data = new VentriloEventData();

        private String StringFromBytes(byte[] bytes) {
        	return new String(bytes, 0, (new String(bytes).indexOf(0)));
        }

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

	    			case VentriloEvents.V3_EVENT_CHAN_MOVE:
	    			case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
    					if (data.user.id == VentriloInterface.getuserid()) {
    						int channel_rate = VentriloInterface.getchannelrate(data.channel.id);
    						Player.clear();
    						Recorder.recorder.stop();
    						Recorder.recorder = new Recorder(channel_rate);
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
	    				}
	    				break;

	    			case VentriloEvents.V3_EVENT_USER_LOGOUT:
	    				Player.close(data.user.id);
	    				UserList.delUser(data.user.id);
	    			    sendBroadcast(new Intent(ServerView.USERLIST_ACTION));
	    				break;

	    			case VentriloEvents.V3_EVENT_LOGIN_COMPLETE:
	    				int lobby_rate = VentriloInterface.getchannelrate((short)0);
	    				Recorder.recorder = new Recorder(lobby_rate);
	    				break;

	    			case VentriloEvents.V3_EVENT_PLAY_AUDIO:
	    				Player.write(data.user.id, data.pcm.rate, data.pcm.channels, data.data.sample, data.pcm.length);
	    				break;

	    			case VentriloEvents.V3_EVENT_USER_TALK_END:
	    			case VentriloEvents.V3_EVENT_USER_TALK_MUTE:
	    				Player.close(data.user.id);
	    				break;

	    			case VentriloEvents.V3_EVENT_CHAN_ADD:
	    				VentriloInterface.getchannel(data, data.channel.id);
	    				ChannelList.addChannel(data.channel.id, StringFromBytes(data.text.name));
	    			    sendBroadcast(new Intent(ServerView.CHANNELLIST_ACTION));
	    				break;

	    			default:
	    				Log.w("mangler", "Unhandled event type: " + Integer.toString(data.type));
	    		}
	    	}
	    	Player.clear();
		}
    };

}
