package org.mangler;

import android.app.ListActivity;
import android.os.Bundle;
import android.widget.ArrayAdapter;

public class ChannelList extends ListActivity {
	
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.channel_list);
        
        int numChannels = -1;
        
    	Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		numChannels = extras.getInt("numchannels");
    	}
        
    	fillData(numChannels);
        registerForContextMenu(getListView());
    }

    // Populate channel list
    private void fillData(int numChannels) {
    	String[] channelnames = new String[numChannels];
    	
    	for (int i = 1; i <= numChannels; i++) {
    		channelnames[i - 1] = "Channel " + i;
    	}
    	
    	ArrayAdapter<String> channels = new ArrayAdapter<String>(this, R.layout.channel_row, R.id.crowtext, channelnames);

        setListAdapter(channels);
    }
}
