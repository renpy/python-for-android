#include <jni.h>
#include <stdio.h>
#include <android/log.h>
#include <string.h>
#include <stdlib.h>

void *SDL_AndroidGetJNIEnv(void);

#define aassert(x) { if (!x) { __android_log_print(ANDROID_LOG_ERROR, "python", "Assertion failed. %s:%d", __FILE__, __LINE__); abort(); }}
#define PUSH_FRAME { (*env)->PushLocalFrame(env, 16); }
#define POP_FRAME  { (*env)->PopLocalFrame(env, NULL); }

void android_vibrate(double seconds) {
    static JNIEnv *env = NULL;
    static jclass *cls = NULL;
    static jmethodID mid = NULL;

    if (env == NULL) {
        env = (JNIEnv *) SDL_AndroidGetJNIEnv();
        aassert(env);
        cls = (*env)->FindClass(env, "org/renpy/android/Hardware");
        aassert(cls);
        mid = (*env)->GetStaticMethodID(env, cls, "vibrate", "(D)V");
        aassert(mid);
    }

    (*env)->CallStaticVoidMethod(
        env, cls, mid,
        (jdouble) seconds);
}

int android_get_dpi(void) {
    static JNIEnv *env = NULL;
    static jclass *cls = NULL;
    static jmethodID mid = NULL;

    if (env == NULL) {
        env = (JNIEnv *) SDL_AndroidGetJNIEnv();
        aassert(env);
        cls = (*env)->FindClass(env, "org/renpy/android/Hardware");
        aassert(cls);
        mid = (*env)->GetStaticMethodID(env, cls, "getDPI", "()I");
        aassert(mid);
    }

    return (*env)->CallStaticIntMethod(env, cls, mid);
}

char* BUILD_MANUFACTURER = NULL;
char* BUILD_MODEL = NULL;
char* BUILD_PRODUCT = NULL;
char* BUILD_VERSION_RELEASE = NULL;

void android_get_buildinfo() {
    static JNIEnv *env = NULL;

    if (env == NULL) {
        jclass *cls = NULL;
        jfieldID fid;
        jstring sval;

        env = (JNIEnv *) SDL_AndroidGetJNIEnv();
        aassert(env);

        cls = (*env)->FindClass(env, "android/os/Build");

        fid = (*env)->GetStaticFieldID(env, cls, "MANUFACTURER", "Ljava/lang/String;");
        sval = (jstring) (*env)->GetStaticObjectField(env, cls, fid);
        BUILD_MANUFACTURER = (*env)->GetStringUTFChars(env, sval, 0);

        fid = (*env)->GetStaticFieldID(env, cls, "MODEL", "Ljava/lang/String;");
        sval = (jstring) (*env)->GetStaticObjectField(env, cls, fid);
        BUILD_MODEL = (*env)->GetStringUTFChars(env, sval, 0);

        fid = (*env)->GetStaticFieldID(env, cls, "PRODUCT", "Ljava/lang/String;");
        sval = (jstring) (*env)->GetStaticObjectField(env, cls, fid);
        BUILD_PRODUCT = (*env)->GetStringUTFChars(env, sval, 0);

        cls = (*env)->FindClass(env, "android/os/Build$VERSION");

        fid = (*env)->GetStaticFieldID(env, cls, "RELEASE", "Ljava/lang/String;");
        sval = (jstring) (*env)->GetStaticObjectField(env, cls, fid);
        BUILD_VERSION_RELEASE = (*env)->GetStringUTFChars(env, sval, 0);
    }
}

void android_action_send(char *mimeType, char *filename, char *subject, char *text, char *chooser_title) {
    static JNIEnv *env = NULL;
    static jclass *cls = NULL;
    static jmethodID mid = NULL;

    if (env == NULL) {
        env = (JNIEnv *) SDL_AndroidGetJNIEnv();
        aassert(env);
        cls = (*env)->FindClass(env, "org/renpy/android/Action");
        aassert(cls);
        mid = (*env)->GetStaticMethodID(env, cls, "send",
			"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
        aassert(mid);
    }

	jstring j_mimeType = (*env)->NewStringUTF(env, mimeType);
	jstring j_filename = NULL;
	jstring j_subject = NULL;
	jstring j_text = NULL;
	jstring j_chooser_title = NULL;
	if ( filename != NULL )
		j_filename = (*env)->NewStringUTF(env, filename);
	if ( subject != NULL )
		j_subject = (*env)->NewStringUTF(env, subject);
	if ( text != NULL )
		j_text = (*env)->NewStringUTF(env, text);
	if ( chooser_title != NULL )
		j_chooser_title = (*env)->NewStringUTF(env, text);

    (*env)->CallStaticVoidMethod(
        env, cls, mid,
		j_mimeType, j_filename, j_subject, j_text,
		j_chooser_title);
}

void android_open_url(char *url) {

	JNIEnv* env = (JNIEnv*) SDL_AndroidGetJNIEnv();
	jobject activity = (jobject) SDL_AndroidGetActivity();
	jclass clazz = (*env)->GetObjectClass(env, activity);
	jmethodID method_id = (*env)->GetMethodID(env, clazz, "openUrl", "(Ljava/lang/String;)V");

	PUSH_FRAME;

	(*env)->CallVoidMethod(env, activity, method_id, (*env)->NewStringUTF(env, url));

    POP_FRAME;

	(*env)->DeleteLocalRef(env, activity);
	(*env)->DeleteLocalRef(env, clazz);

}

