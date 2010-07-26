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

import android.util.Log;
import android.widget.Toast;

public class EventHandler {
	private ServerView sv;
	
	public EventHandler(ServerView sv) {
		this.sv = sv;
	}
	
	public void process() {
		
		VentriloEventData data;
		
		while ((data = EventService.getNext()) != null) {
			ChannelListEntity entity;
			Log.d("mangler", "EventHandler: processing event type " + data.type);
			switch (data.type) {
				case VentriloEvents.V3_EVENT_CHAT_MESSAGE:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					sv.addChatMessage(entity.name, ChannelListEntity.stringFromBytes(data.data.chatmessage));
					break;
					
				case VentriloEvents.V3_EVENT_CHAT_JOIN:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					sv.addChatUser(entity.name);
					break;
					
				case VentriloEvents.V3_EVENT_CHAT_LEAVE:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					sv.removeChatUser(entity.name);
					break;
					
				case VentriloEvents.V3_EVENT_CHAN_ADD:
					entity = new ChannelListEntity(ChannelListEntity.CHANNEL, data.channel.id);
					ChannelList.add(entity);
					break;
					
				case VentriloEvents.V3_EVENT_USER_LOGIN:
					if (data.user.id != 0) {
						int flags = data.flags;
						entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
						ChannelList.add(entity);
						if (entity.parentid == VentriloInterface.getuserchannel(VentriloInterface.getuserid())) {
							UserList.addUser(entity.id, entity.name, entity.parentid);
						}
						sv.notifyAdaptersDataSetChanged();
						// user was added from userlist sent at login (existing user)
						// from lv3: #define V3_LOGIN_FLAGS_EXISTING (1 << 0)
						if ((flags & (1 << 0)) == 0) {
							sv.ttsWrapper.speak((entity.phonetic != "" ? entity.phonetic : entity.name) + " has logged in");
						}
					}
					break;
					
				case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					if (entity.id == VentriloInterface.getuserid()) {
						Player.clear();
						Recorder.rate(VentriloInterface.getchannelrate(data.channel.id));
						if (data.channel.id == 0) {
							UserList.addUser(data.user.id, entity.name, data.channel.id);
							Toast.makeText(sv, "Changed to Lobby", Toast.LENGTH_SHORT).show();
						} else {
							entity = new ChannelListEntity(ChannelListEntity.CHANNEL, data.channel.id);
							Toast.makeText(sv, "Changed to " + entity.name, Toast.LENGTH_SHORT).show();
						}
					} else {
						if (data.channel.id == VentriloInterface.getuserchannel(VentriloInterface.getuserid())) {
							sv.ttsWrapper.speak((entity.phonetic != "" ? entity.phonetic : entity.name) + " has joined the channel");
							UserList.addUser(data.user.id, entity.name, data.channel.id);
						} else if (UserList.getChannel(data.user.id) == VentriloInterface.getuserchannel(VentriloInterface.getuserid())) {
							sv.ttsWrapper.speak((entity.phonetic != "" ? entity.phonetic : entity.name) + " has left the channel");
							UserList.delUser(data.user.id);
						}
						Player.close(data.user.id);
					}
					entity = new ChannelListEntity(ChannelList.get(data.user.id));
					entity.parentid = data.channel.id;
					ChannelList.remove(entity.id);
					ChannelList.add(entity);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_USER_LOGOUT:
					entity = new ChannelListEntity(ChannelList.get(data.user.id));
					Player.close(data.user.id);
					UserList.delUser(data.user.id);
					ChannelList.remove(data.user.id);
					sv.ttsWrapper.speak((entity.phonetic != "" ? entity.phonetic : entity.name) + " has logged out");
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_PLAY_AUDIO:
					UserList.updateStatus(data.user.id, R.drawable.transmit_on);
					ChannelList.updateStatus(data.user.id, R.drawable.xmit_on);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_USER_TALK_START:
					UserList.updateStatus(data.user.id, R.drawable.transmit_init);
					ChannelList.updateStatus(data.user.id, R.drawable.xmit_init);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_USER_TALK_END:
				case VentriloEvents.V3_EVENT_USER_TALK_MUTE:
				case VentriloEvents.V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
				case VentriloEvents.V3_EVENT_USER_CHANNEL_MUTE_CHANGED:
					UserList.updateStatus(data.user.id, R.drawable.transmit_off);
					ChannelList.updateStatus(data.user.id, R.drawable.xmit_off);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_PING:
					if (data.ping < 65535) {
						sv.setTitle(sv.servername + " - Ping: " + data.ping + "ms");
					}
					break;
					
				default:
					Log.d("mangler", "Unhandled event type: " + Integer.toString(data.type));
					break;
			}
		}
	}
}
