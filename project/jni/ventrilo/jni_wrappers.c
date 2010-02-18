#include <jni.h>
#include <stdint.h>
#include "ventrilo3.h"
#include "debug.h"

JNIEXPORT jboolean JNICALL Java_org_mangler_VentriloInterface_recv() {
	return _v3_recv(1) != NULL;
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_pcmlengthforrate(jint rate) {
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

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_messagewaiting(jint block) {
	return v3_message_waiting(block);
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
	v3_start_audio(sendtype);
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

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_login(JNIEnv* env, jobject obj, jstring server, jstring username, jstring password, jstring phonetic) {
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

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_getevent(JNIEnv* env, jobject obj, jobject eventdata) {
	v3_event *ev = v3_get_event(V3_BLOCK);
	if(ev != NULL) {
		jclass event_class = (*env)->GetObjectClass(env, eventdata);
		jfieldID _data = (*env)->GetFieldID(env, event_class, "data", "Lorg/mangler/EventData$_data;");
		
		// Returning NULL when it shouldn't.
		jobject data  = (*env)->GetObjectField(env, eventdata, _data);
		jclass object_class = (*env)->GetObjectClass(env, data); 
		
		jfieldID _sample = (*env)->GetFieldID(env, object_class, "sample", "[B");
		jbyteArray sample = (*env)->GetObjectField(env, data, _sample);
		
		jfieldID _field = (*env)->GetFieldID(env, event_class, "ping", "S");
		(*env)->SetShortField(env, eventdata, _field, ev->ping);

		jint type = ev->type;
		free(ev);
		return type;
	}
	return 0;
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_sendaudio(JNIEnv* env, jobject obj, jshortArray pcm, jint size, jint rate) {
	jboolean isCopy;
	jshort *data = (*env)->GetShortArrayElements(env, pcm, &isCopy);

	print((uint8_t*)data, size * 2, "jni_wrappers");
	v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, rate, (uint8_t*)data, size * 2, 0);

	(*env)->ReleaseShortArrayElements(env, pcm, data, 0);
}
