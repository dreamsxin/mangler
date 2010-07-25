package org.mangler.android;

import java.lang.reflect.Constructor;

import android.content.Context;

public abstract class TTSWrapper {
    public static TTSWrapper getInstance(Context context) {
        try {
            Class tts = Class.forName("android.speech.tts.TextToSpeech");
            Class impl = Class.forName("org.mangler.android.TTSWrapperImpl");
            Constructor c = impl.getConstructor(Context.class);
            return (TTSWrapper)c.newInstance(context);
        }
        catch(Exception e) {
            return null;
        }
    }
    
    public abstract void shutdown();
    
    public abstract void speak(String message);
}