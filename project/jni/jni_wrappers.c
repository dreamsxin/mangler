#include <jni.h>
#include "ventrilo3.h"

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_login(JNIEnv* env, jobject obj, jstring server, jstring username, jstring password, jstring phonetic) {
	v3_debuglevel(V3_DEBUG_ALL);
	
	const char* _server = (*env)->GetStringUTFChars(env, server, 0);
	const char* _username = (*env)->GetStringUTFChars(env, username, 0);
	const char* _password = (*env)->GetStringUTFChars(env, password, 0);
	const char* _phonetic = (*env)->GetStringUTFChars(env, phonetic, 0);
	jint ret = v3_login(_server, _username, _password, _phonetic);
	(*env)->ReleaseStringUTFChars(env, server, _server);
	(*env)->ReleaseStringUTFChars(env, username, _username);
	(*env)->ReleaseStringUTFChars(env, password, _password);
	(*env)->ReleaseStringUTFChars(env, phonetic, _phonetic);
	return ret;
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_joinchat() {
	v3_join_chat();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_leavechat() {
	v3_leave_chat();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_sendchatmessage(JNIEnv* env, jobject obj, jstring message) {
	const char* _message = (*env)->GetStringUTFChars(env, message, 0);
	v3_send_chat_message(_message);
	(*env)->ReleaseStringUTFChars(env, message, _message);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_logout() {
	v3_logout();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_changechannel(JNIEnv* env, jobject obj, jchar channelid, jstring password) {
	const char* _password = (*env)->GetStringUTFChars(env, password, 0);
	v3_change_channel(channelid, _password);
	(*env)->ReleaseStringUTFChars(env, password, _password);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_phantomadd(jchar channelid) {
	v3_phantom_add(channelid);
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_phantomremove(jchar channelid) {
	v3_phantom_remove(channelid);
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_isloggedin() {
	return v3_is_loggedin();
}

JNIEXPORT jchar JNICALL Java_org_mangler_VentriloInterface_getuserid() {
	return v3_get_user_id();
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterfaceInterface_settext(JNIEnv* env, jobject obj, jstring comment, jstring url, jstring integrationtext, jboolean silent) {
	const char* _comment = (*env)->GetStringUTFChars(env, comment, 0);
	const char* _url = (*env)->GetStringUTFChars(env, url, 0);
	const char* _integrationtext = (*env)->GetStringUTFChars(env, integrationtext, 0);
	v3_set_text(_comment, _url, _integrationtext, silent);
	(*env)->ReleaseStringUTFChars(env, comment, _comment);
	(*env)->ReleaseStringUTFChars(env, url, _url);
	(*env)->ReleaseStringUTFChars(env, integrationtext, _integrationtext);
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
