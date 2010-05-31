package org.mangler;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;

public class ChatView extends Activity {

	// Menu options.
	private final int OPTION_JOIN_CHAT  = 1;
	private final int OPTION_LEAVE_CHAT = 2;
	
	// Chat events.
	public static final int EVENT_JOIN	= 1;
	public static final int EVENT_LEAVE = 2;
	public static final int EVENT_MSG	= 3;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	    setContentView(R.layout.chat);
	    
	    // Start receiving chat intents.
	    registerReceiver(receiver, new IntentFilter(ServerView.CHAT_ACTION));
	    
	    // Listen for enter events on our message field.
	    ((EditText)findViewById(R.id.message)).setOnKeyListener(onMessageEnter);
	}
	
	private OnKeyListener onMessageEnter = new OnKeyListener() {
		public boolean onKey(View v, int keyCode, KeyEvent event) {
			if ((event.getAction() == KeyEvent.ACTION_DOWN) && (keyCode == KeyEvent.KEYCODE_ENTER)) {
				// Send chat message.
				final EditText message = (EditText)findViewById(R.id.message);
				VentriloInterface.sendchatmessage(message.getText().toString());
				
				// Clear message field.
				message.setText("");
				
				// Hide keyboard.
	           ((InputMethodManager)getSystemService(INPUT_METHOD_SERVICE)).hideSoftInputFromWindow(message.getWindowToken(), 0);
				return true;
			}
			return false;
		}
	};
	
    public boolean onCreateOptionsMenu(Menu menu) {
    	// Create menu buttons.
    	menu.add(0, OPTION_JOIN_CHAT, 0, "Join chat").setIcon(R.drawable.menu_join_chat);
        menu.add(0, OPTION_LEAVE_CHAT, 0, "Leave chat").setIcon(R.drawable.menu_leave_chat);
        return true;
    }
    
    public boolean onOptionsItemSelected(MenuItem item) {
    	final EditText message = (EditText)findViewById(R.id.message);
        switch(item.getItemId()) {
        	case OPTION_JOIN_CHAT:
        		VentriloInterface.joinchat();
        		message.setEnabled(true);
        		break;
        	
        	case OPTION_LEAVE_CHAT:
        		VentriloInterface.leavechat();
        		message.setEnabled(false);
        		break;
        		
        	default:
        		return false;
        }
        return true;
    }
    
	private BroadcastReceiver receiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			final TextView messages = (TextView)findViewById(R.id.messages);
			switch(intent.getIntExtra("event", -1)) {
				case EVENT_JOIN:
					messages.append("\n* " + intent.getStringExtra("username") + " has joined the chat.");
					break;
					
				case EVENT_LEAVE:
					messages.append("\n* " + intent.getStringExtra("username") + " has left the chat.");
					break;
			 		
			 	case EVENT_MSG:
			 		messages.append("\n" + intent.getStringExtra("username") + ": " + intent.getStringExtra("message"));
			 		break;
			 }
			
			// Scroll to bottom.
			final ScrollView chatscroll = (ScrollView)findViewById(R.id.chatscroll);
			chatscroll.post(new Runnable() {
				public void run() {
					chatscroll.fullScroll(ScrollView.FOCUS_DOWN); 
				}
			});
		 }
	 };
    
}
