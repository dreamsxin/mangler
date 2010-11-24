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

import java.lang.reflect.Constructor;

import android.content.Context;

public abstract class TTSWrapper {
    public static TTSWrapper getInstance(Context context) {
        try {
            @SuppressWarnings("unused")
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