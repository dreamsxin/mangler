/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
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

package org.mangler.android;

import java.util.HashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

public class EventService extends Service {

	private final IBinder binder = new EventBinder();
	private boolean running = false;
	private static ConcurrentLinkedQueue<VentriloEventData> eventQueue;

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
    	eventQueue = new ConcurrentLinkedQueue<VentriloEventData>();
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
    

    private Runnable eventRunnable = new Runnable() {

    	

		public void run() {
			boolean forwardToUI = true;
    		final int INIT = 0;
    		final int ON = 1;
    		final int OFF = 2;
    		HashMap<Short, Integer> talkState = new HashMap<Short, Integer>();
	    	while (running) {
	    		forwardToUI = true;
	    		
	    		VentriloEventData data = new VentriloEventData();
	    		VentriloInterface.getevent(data);

	    		switch (data.type) {
    				case VentriloEvents.V3_EVENT_USER_LOGOUT:
    					Player.close(data.user.id);
    					talkState.put(data.user.id, OFF);
    					break;
    					
	    			case VentriloEvents.V3_EVENT_LOGIN_COMPLETE:
	    				Recorder.rate(VentriloInterface.getchannelrate(VentriloInterface.getuserchannel(VentriloInterface.getuserid())));
    					break;
    					
	    			case VentriloEvents.V3_EVENT_USER_TALK_START:
	    				talkState.put(data.user.id, INIT);
	    				break;
	    				
	    			case VentriloEvents.V3_EVENT_PLAY_AUDIO:
	    				Player.write(data.user.id, data.pcm.rate, data.pcm.channels, data.data.sample, data.pcm.length);
	    				// Only forward the first play audio event to the UI
	    				if (talkState.get(data.user.id) == OFF) {
		    				talkState.put(data.user.id, ON);
	    					forwardToUI = false;
	    				}
	    				break;
	    				
	    			case VentriloEvents.V3_EVENT_USER_TALK_END:
	    			case VentriloEvents.V3_EVENT_USER_TALK_MUTE:
	    			case VentriloEvents.V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
	    			case VentriloEvents.V3_EVENT_USER_CHANNEL_MUTE_CHANGED:
	    				Player.close(data.user.id);
    					talkState.put(data.user.id, OFF);
	    				break;
	    				
	    			case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
    					if (data.user.id == VentriloInterface.getuserid()) {
   							Player.clear();
    						Recorder.rate(VentriloInterface.getchannelrate(data.channel.id));
    						talkState.put(data.user.id, OFF);
    					} else {
    						Player.close(data.user.id);
    						talkState.put(data.user.id, OFF);
    					}
    					break;
						
	    		}
	    		if (forwardToUI) {
	    			// delete these massive elements before queuing.  The UI should not need them
	    			//data.data.sample = null;
	    			//data.data.motd = null;
	    			
	    			eventQueue.add(data);
    				sendBroadcast(new Intent(ServerView.EVENT_ACTION));
	    		}
	    	}
	    	Player.clear();
	    	Recorder.stop();
		}
    };
    
    public static VentriloEventData getNext() {
    	return eventQueue.poll();
    }

}
