#include <jni.h>
#include <stdint.h>
#include "ventrilo3.h"
#include "debug.h"

JNIEXPORT jboolean JNICALL Java_org_mangler_VentriloInterface_recv() {
	_v3_net_message *msg = _v3_recv(V3_BLOCK);
	return msg && _v3_process_message(msg) == V3_OK;
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_pcmlengthforrate(JNIEnv* env, jobject obj, jint rate) {
	return v3_pcmlength_for_rate(rate);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_joinchat() {
	v3_join_chat();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_leavechat() {
	v3_leave_chat();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_logout() {
	v3_logout();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_phantomadd(jchar channelid) {
	v3_phantom_add(channelid);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_phantomremove(jchar channelid) {
	v3_phantom_remove(channelid);
}

JNIEXPORT jboolean JNICALL Java_org_mangler_VentriloInterface_isloggedin() {
	return v3_is_loggedin() != 0;
}

JNIEXPORT jchar JNICALL Java_org_mangler_VentriloInterface_getuserid() {
	return v3_get_user_id();
}

JNIEXPORT jboolean JNICALL Java_org_mangler_VentriloInterface_messagewaiting(jboolean block) {
	return v3_message_waiting(block) != 0;
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_getmaxclients() {
	return v3_get_max_clients();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_clearevents() {
	v3_clear_events();
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_getcodecrate(jchar codec, jchar format) {
	return v3_get_codec_rate(codec, format);
}

JNIEXPORT jchar JNICALL Java_org_mangler_VentriloInterface_getuserchannel(jchar id) {
	return v3_get_user_channel(id);
}

JNIEXPORT jchar JNICALL Java_org_mangler_VentriloInterface_channelrequirespassword(jchar channelid) {
	return v3_channel_requires_password(channelid);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_startaudio(jchar sendtype) {
	v3_start_audio(3);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_stopaudio() {
	v3_stop_audio();
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_usercount() {
	return v3_user_count();
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_channelcount() {
	return v3_channel_count();
}

JNIEXPORT int JNICALL Java_org_mangler_VentriloInterface_debuglevel(jint level) {
	return v3_debuglevel(level);
}

JNIEXPORT jboolean JNICALL Java_org_mangler_VentriloInterface_login(JNIEnv* env, jobject obj, jstring server, jstring username, jstring password, jstring phonetic) {
	v3_debuglevel(V3_DEBUG_ALL);
	char* _server	= (char*)(*env)->GetStringUTFChars(env, server, 0);
	char* _username = (char*)(*env)->GetStringUTFChars(env, username, 0);
	char* _password = (char*)(*env)->GetStringUTFChars(env, password, 0);
	char* _phonetic = (char*)(*env)->GetStringUTFChars(env, phonetic, 0);
	jint ret = v3_login(_server, _username, _password, _phonetic);
	(*env)->ReleaseStringUTFChars(env, server, _server);
	(*env)->ReleaseStringUTFChars(env, username, _username);
	(*env)->ReleaseStringUTFChars(env, password, _password);
	(*env)->ReleaseStringUTFChars(env, phonetic, _phonetic);
	return ret;
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_sendchatmessage(JNIEnv* env, jobject obj, jstring message) {
	char* _message = (char*)(*env)->GetStringUTFChars(env, message, 0);
	v3_send_chat_message(_message);
	(*env)->ReleaseStringUTFChars(env, message, _message);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_changechannel(JNIEnv* env, jobject obj, jchar channelid, jstring password) {
	char* _password = (char*)(*env)->GetStringUTFChars(env, password, 0);
	v3_change_channel(channelid, _password);
	(*env)->ReleaseStringUTFChars(env, password, _password);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_settext(JNIEnv* env, jobject obj, jstring comment, jstring url, jstring integrationtext, jboolean silent) {
	char* _comment = (char*)(*env)->GetStringUTFChars(env, comment, 0);
	char* _url = (char*)(*env)->GetStringUTFChars(env, url, 0);
	char* _integrationtext = (char*)(*env)->GetStringUTFChars(env, integrationtext, 0);
	v3_set_text(_comment, _url, _integrationtext, silent);
	(*env)->ReleaseStringUTFChars(env, comment, _comment);
	(*env)->ReleaseStringUTFChars(env, url, _url);
	(*env)->ReleaseStringUTFChars(env, integrationtext, _integrationtext);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_getevent(JNIEnv* env, jobject obj, jobject eventdata) {
	v3_event *ev = v3_get_event(V3_BLOCK);
	if(ev != NULL) {
		jclass event_class = (*env)->GetObjectClass(env, eventdata);

		// Event type.
		(*env)->SetShortField(
			env, 
			eventdata, 
			(*env)->GetFieldID(env, event_class, "type", "S"), 
			ev->type
		);
		
		switch(ev->type) {
			case V3_EVENT_PLAY_AUDIO:
				{
					// PCM data.
					jobject pcm = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "pcm", "Lorg/mangler/VentriloEventData$_pcm;"));
					jclass pcm_class = (*env)->GetObjectClass(env, pcm);
					(*env)->SetIntField(env, pcm, (*env)->GetFieldID(env, pcm_class, "length", "I"), ev->pcm.length);
					(*env)->SetIntField(env, pcm, (*env)->GetFieldID(env, pcm_class, "rate", "I"), ev->pcm.rate);
					(*env)->SetShortField(env, pcm, (*env)->GetFieldID(env, pcm_class, "send_type", "S"), ev->pcm.send_type);
					(*env)->SetByteField(env, pcm, (*env)->GetFieldID(env, pcm_class, "channels", "B"), ev->pcm.channels);
					
					// User ID.
					jobject user = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "user", "Lorg/mangler/VentriloEventData$_user;"));
					jclass user_class = (*env)->GetObjectClass(env, user);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->user.id);
					
					// Sample.
					jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
					jclass data_class = (*env)->GetObjectClass(env, data);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "sample", "[B")), 0, ev->pcm.length, ev->data->sample);
				}
				break;
				
			case V3_EVENT_PING:
				{
					// Ping.
					(*env)->SetIntField(env, eventdata, (*env)->GetFieldID(env, event_class, "ping", "I"), ev->ping);
				}
				break;
				
			case V3_EVENT_USER_TALK_END:
			case V3_EVENT_CHAT_JOIN:
			case V3_EVENT_CHAT_LEAVE:
			case V3_EVENT_PRIVATE_CHAT_START:
			case V3_EVENT_PRIVATE_CHAT_END:
			case V3_EVENT_PRIVATE_CHAT_AWAY:
			case V3_EVENT_PRIVATE_CHAT_BACK:
			case V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
			case V3_EVENT_USER_CHANNEL_MUTE_CHANGED:
			case V3_EVENT_USER_RANK_CHANGE:
				{
					// User ID.
					jobject user = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "user", "Lorg/mangler/VentriloEventData$_user;"));
					jclass user_class = (*env)->GetObjectClass(env, user);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->user.id);
				}
				break;
				
			case V3_EVENT_USER_CHAN_MOVE:
			case V3_EVENT_USER_LOGOUT:
			case V3_EVENT_CHAN_REMOVE:
				{
					// User ID.
					jobject user = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "user", "Lorg/mangler/VentriloEventData$_user;"));
					jclass user_class = (*env)->GetObjectClass(env, user);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->user.id);
					
					// Channel ID.
					jobject channel = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "channel", "Lorg/mangler/VentriloEventData$_channel;"));
					jclass channel_class = (*env)->GetObjectClass(env, channel);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->channel.id);
				}
				break;
			
			case V3_EVENT_USER_MODIFY:
			case V3_EVENT_USER_LOGIN:
				{
					// User ID.
					jobject user = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "user", "Lorg/mangler/VentriloEventData$_user;"));
					jclass user_class = (*env)->GetObjectClass(env, user);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->user.id);
					
					// Channel ID.
					jobject channel = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "channel", "Lorg/mangler/VentriloEventData$_channel;"));
					jclass channel_class = (*env)->GetObjectClass(env, channel);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->channel.id);
					
					v3_user *u = v3_get_user(ev->user.id);
					if(u) {
						jobject text = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "text", "Lorg/mangler/VentriloEventData$_text;"));
						jclass text_class = (*env)->GetObjectClass(env, text);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "name", "[B")), 0, sizeof(ev->text.name), u->name);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "comment", "[B")), 0, sizeof(ev->text.comment), u->comment);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "phonetic", "[B")), 0, sizeof(ev->text.phonetic), u->phonetic);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "url", "[B")), 0, sizeof(ev->text.url), u->url);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "integration_text", "[B")), 0, sizeof(ev->text.integration_text), u->integration_text);
						v3_free_user(u);
					}
				}
				break;
				
			case V3_EVENT_STATUS:
				{
					// Status message & percentage.
					jobject status = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "status", "Lorg/mangler/VentriloEventData$_status;"));
					jclass status_class = (*env)->GetObjectClass(env, status);
					(*env)->SetByteField(env, status, (*env)->GetFieldID(env, status_class, "percent", "B"), ev->status.percent);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, status, (*env)->GetFieldID(env, status_class, "message", "[B")), 0, sizeof(ev->status.message), ev->status.message);
				}
				break;
				
			case V3_EVENT_LOGIN_COMPLETE:
			case V3_EVENT_LOGIN_FAIL:
			case V3_EVENT_DISCONNECT:
			case V3_EVENT_ADMIN_AUTH:
			case V3_EVENT_CHAN_ADMIN_UPDATED:
			case V3_EVENT_PERMS_UPDATED:
				{
					// No event data for these types!
				}
				break;

			case V3_EVENT_PRIVATE_CHAT_MESSAGE:
				{
					// User IDs.
					jobject user = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "user", "Lorg/mangler/VentriloEventData$_user;"));
					jclass user_class = (*env)->GetObjectClass(env, user);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "privchat_user1", "S"), ev->user.privchat_user1);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "privchat_user2", "S"), ev->user.privchat_user2);
					
					// Chat message.
					jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
					jclass data_class = (*env)->GetObjectClass(env, data);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "chatmessage", "[B")), 0, sizeof(ev->data->chatmessage), ev->data->chatmessage);
				}
				break;
				
			case V3_EVENT_USERLIST_ADD:
			case V3_EVENT_USERLIST_MODIFY:
				{
					// Account fields.
					jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
					jclass data_class = (*env)->GetObjectClass(env, data);
					jobject account = (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "account", "Lorg/mangler/VentriloEventData$_data$_account;"));
					jclass account_class = (*env)->GetObjectClass(env, account);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, account, (*env)->GetFieldID(env, account_class, "username", "[B")), 0, sizeof(ev->data->account.username), ev->data->account.username);
					/*
					 * We wont need these for a while.
					ev->data->account.perms
					ev->data->account.owner
					ev->data->account.notes
					ev->data->account.lock_reason
					ev->data->account.chan_admin_count
					ev->data->account.chan_admin
					ev->data->account.chan_auth_count
					ev->data->account.chan_auth
					*/
				}
				break;
			
			case V3_EVENT_CHAN_ADD:
			case V3_EVENT_CHAN_MODIFY:
				{
					// Channel id.
					jobject channel = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "channel", "Lorg/mangler/VentriloEventData$_channel;"));
					jclass channel_class = (*env)->GetObjectClass(env, channel);
					(*env)->SetShortField(env, channel, (*env)->GetFieldID(env, channel_class, "id", "S"), ev->channel.id);
					
					v3_channel *c = v3_get_channel(ev->channel.id);
					if(c) {
						// Channel fields.
						jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
						jclass data_class = (*env)->GetObjectClass(env, data);
						jobject channel = (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "channel", "Lorg/mangler/VentriloEventData$_data$_channel;"));
						jclass channel_class = (*env)->GetObjectClass(env, channel);
						(*env)->SetShortField(env, channel, (*env)->GetFieldID(env, channel_class, "parent", "S"), c->parent);
						(*env)->SetShortField(env, channel, (*env)->GetFieldID(env, channel_class, "channel_codec", "S"), c->channel_codec);
						(*env)->SetShortField(env, channel, (*env)->GetFieldID(env, channel_class, "channel_format", "S"), c->channel_format);
						(*env)->SetBooleanField(env, channel, (*env)->GetFieldID(env, channel_class, "password_protected", "Z"), c->password_protected != 0);
						
						// Text fields.
						jobject text = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "text", "Lorg/mangler/VentriloEventData$_text;"));
						jclass text_class = (*env)->GetObjectClass(env, text);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "name", "[B")), 0, sizeof(ev->text.name), c->name);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "phonetic", "[B")), 0, sizeof(ev->text.phonetic), c->phonetic);
						(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, text, (*env)->GetFieldID(env, text_class, "comment", "[B")), 0, sizeof(ev->text.comment), c->comment);
						
						v3_free_channel(c);
					}
				}
				break;
				
			case V3_EVENT_DISPLAY_MOTD:
				{
					// MOTD.
					jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
					jclass data_class = (*env)->GetObjectClass(env, data);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "motd", "[B")), 0, sizeof(ev->data->motd), ev->data->motd);
				}
				break;
				
			case V3_EVENT_CHAN_BADPASS:
				{
					// Channel ID.
					jobject channel = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "channel", "Lorg/mangler/VentriloEventData$_channel;"));
					jclass channel_class = (*env)->GetObjectClass(env, channel);
					(*env)->SetShortField(env, channel, (*env)->GetFieldID(env, channel_class, "id", "S"), ev->channel.id);
					
					// Error message.
					jobject error = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "error", "Lorg/mangler/VentriloEventData$_error;"));
					jclass error_class = (*env)->GetObjectClass(env, error);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, error, (*env)->GetFieldID(env, error_class, "message", "[B")), 0, sizeof(ev->error.message), ev->error.message);
				}
				break;
				
				
			case V3_EVENT_ERROR_MSG:
				{
					// Error message & disconnect flag.
					jobject error = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "error", "Lorg/mangler/VentriloEventData$_error;"));
					jclass error_class = (*env)->GetObjectClass(env, error);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, error, (*env)->GetFieldID(env, error_class, "message", "[B")), 0, sizeof(ev->error.message), ev->error.message);
					(*env)->SetBooleanField(env, error, (*env)->GetFieldID(env, error_class, "disconnected", "Z"), ev->error.disconnected != 0);	
				}
				break;
				
			case V3_EVENT_USER_TALK_START:
				{
					// PCM data.
					jobject pcm = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "pcm", "Lorg/mangler/VentriloEventData$_pcm;"));
					jclass pcm_class = (*env)->GetObjectClass(env, pcm);
					(*env)->SetIntField(env, pcm, (*env)->GetFieldID(env, pcm_class, "rate", "I"), ev->pcm.rate);
					(*env)->SetShortField(env, pcm, (*env)->GetFieldID(env, pcm_class, "send_type", "S"), ev->pcm.send_type);
					
					// User ID.
					jobject user = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "user", "Lorg/mangler/VentriloEventData$_user;"));
					jclass user_class = (*env)->GetObjectClass(env, user);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->user.id);
				}
				break;
				
			case V3_EVENT_CHAT_MESSAGE:
				{
					// User ID.
					jobject user = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "user", "Lorg/mangler/VentriloEventData$_user;"));
					jclass user_class = (*env)->GetObjectClass(env, user);
					(*env)->SetShortField(env, user, (*env)->GetFieldID(env, user_class, "id", "S"), ev->user.id);
					
					// Chat message.
					jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
					jclass data_class = (*env)->GetObjectClass(env, data);
					(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "chatmessage", "[B")), 0, sizeof(ev->data->chatmessage), ev->data->chatmessage);
				}
				break;
				
			case V3_EVENT_USERLIST_REMOVE:
				{
					// Account ID.
					jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
					jclass data_class = (*env)->GetObjectClass(env, data);
					jobject account = (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "account", "Lorg/mangler/VentriloEventData$_data$_account;"));
					jclass account_class = (*env)->GetObjectClass(env, account);
					(*env)->SetShortField(env, account, (*env)->GetFieldID(env, account_class, "id", "S"), ev->account.id);
				}
				break;
			
			case V3_EVENT_USERLIST_CHANGE_OWNER:
				{
					// Account IDs.
					jobject data = (*env)->GetObjectField(env, eventdata, (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/VentriloEventData$_data;"));
					jclass data_class = (*env)->GetObjectClass(env, data);
					jobject account = (*env)->GetObjectField(env, data, (*env)->GetFieldID(env, data_class, "account", "Lorg/mangler/VentriloEventData$_data$_account;"));
					jclass account_class = (*env)->GetObjectClass(env, account);
					(*env)->SetShortField(env, account, (*env)->GetFieldID(env, account_class, "id", "S"), ev->account.id);
					(*env)->SetShortField(env, account, (*env)->GetFieldID(env, account_class, "id2", "S"), ev->account.id2);
				}
				break;
		}
		free(ev);
	}
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_sendaudio(JNIEnv* env, jobject obj, jbyteArray pcm, jint size, jint rate) {
	jboolean isCopy;
	jbyte *data = (*env)->GetByteArrayElements(env, pcm, &isCopy);
	v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, rate, data, size, 0);
	(*env)->ReleaseByteArrayElements(env, pcm, data, 0);
}
