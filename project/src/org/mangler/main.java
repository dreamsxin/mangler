/*
 * $LastChangedDate: 2010-05-28 20:39:54 +0200 (Fri, 28 May 2010) $
 * $Revision: 846 $
 * $LastChangedBy: clearscreen $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/libventrilo3.c $
 *
 * This file is part of Mangler.
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

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class main extends Activity{
	
	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	System.loadLibrary("ventrilo_interface");
    	
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        Button slist = (Button)findViewById(R.id.ServerListButton);
        
        slist.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				Intent intent = new Intent(main.this, ServerList.class);
				startActivity(intent);
			}
        });

        // Connection code currently at bottom of ServerList class
        
    	/*if(VentriloInterface.login("vent.mangler.org:9047", "droid_test", "", "") != 0) {
	    	
	    	Runnable runnable = new Runnable() {
	    		public void run() {
	    			while(true) {
	    				if(!VentriloInterface.recv()) break;
	    			}
	    		}
	    	};
	    	Thread t = new Thread(runnable);
	    	t.start();
	    */
	    	/*
	    	// Play incoming audio.
	    	AudioTrack at = new AudioTrack(
	    		AudioManager.STREAM_VOICE_CALL, 
	    		8000, // set this to channel rate
	    		AudioFormat.CHANNEL_CONFIGURATION_MONO, 
	    		AudioFormat.ENCODING_PCM_16BIT, 
	    		32768, 
	    		AudioTrack.MODE_STREAM
	    	);
	    	at.play();
	    	*/
	    	
	    	/*
	    	// Send recorded audio.
	    	try {
		    	Recorder rec = new Recorder(8000);
		    	rec.start();
		    	try { Thread.sleep(5000); } catch(InterruptedException e) {}
		    	rec.stop();
	    	}
	    	catch(RuntimeException e) {
	    		Log.e("RuntimeException", e.toString());
	    	}
	    	*/
	        
	    //}
    }
}