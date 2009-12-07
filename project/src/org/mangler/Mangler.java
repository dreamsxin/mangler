package org.mangler;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class Mangler extends Activity
{
	@Override
	public void onCreate(Bundle savedInstanceState) 
	{
	    super.onCreate(savedInstanceState);
	
	    // This native function is nasty. Either implement recv() into java, or properly do it in libventrilo.
	    VentriloInterface.startprocessing();
	    
	    // TODO: Thread this.
	    EventData ev = new EventData();
	    int event_id;
	    while((event_id = VentriloInterface.getevent(ev)) != 0) 
	    {
	    	switch(event_id) 
	    	{
	    	case VentriloEvents.V3_EVENT_PING:
	    			Log.v("Mangler", "Received a ping event: " + ev.ping + "ms");
	    		break;
	    	
	    		default:
	    			Log.v("Mangler", "Unknown event: " + Integer.toString(event_id));
	    	}
	    }
	}

}
