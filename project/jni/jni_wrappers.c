#include <jni.h>
#include "ventrilo3.h"

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_login(JNIEnv* env, jobject obj, jstring server, jstring username, jstring password, jstring phonetic) {
	// Android JNI does not provide JNI_Onload so we must do this here.
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
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_leavechat() {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_sendchatmessage(jstring message) {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_logout() {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_changechannel(jchar channelid, jstring password) {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_phantomadd(jchar channelid) {

}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_phantomremove(jchar channelid) {
	
}
JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_debuglevel(jint level) {
	
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_isloggedin() {
	
}

JNIEXPORT jchar JNICALL Java_org_mangler_VentriloInterface_getuserid() {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterfaceInterface_settext(jstring comment, jstring url, jstring jintegrationtext, jboolean silent) {
	
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_messagewaiting(jint block) {
	
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_getsoundqlength() {
	
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_getmaxclients() {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_clearevents() {
	
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_getcodecrate(jchar codec, jchar format) {
	
}

JNIEXPORT jchar JNICALL Java_org_mangler_VentriloInterface_getuserchannel(jchar id) {
	
}

JNIEXPORT jchar JNICALL Java_org_mangler_VentriloInterface_channelrequirespassword(jchar channelid) {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_startaudio(jchar sendtype) {
	
}

JNIEXPORT void JNICALL Java_org_mangler_VentriloInterface_stopaudio() {
	
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_usercount() {
	
}

JNIEXPORT jint JNICALL Java_org_mangler_VentriloInterface_channelcount() {
	
}
