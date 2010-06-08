/*
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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class Main extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
    	final ServiceConnection serviceconnection = new ServiceConnection() {
    		public void onServiceConnected(ComponentName className, IBinder service) {
    			// eventservice = ((EventService.EventBinder)service).getService();
    		}
    		
    		public void onServiceDisconnected(ComponentName arg0) {
    			// eventservice = null;
    		}
    	};
    	bindService(new Intent(this, EventService.class), serviceconnection, Context.BIND_AUTO_CREATE);
        
    	// Load native library.
    	System.loadLibrary("ventrilo_interface");
    	
    	// Set debug level.
    	VentriloInterface.debuglevel(VentriloDebugLevels.V3_DEBUG_INFO);
        
        ((Button)findViewById(R.id.ServerListButton)).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				Intent intent = new Intent(Main.this, ServerList.class);
				startActivity(intent);
			}
        });
    }
}