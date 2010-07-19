/*
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-07-06 17:50:28 +0200 (Tue, 06 Jul 2010) $
 * $Revision: 958 $
 * $LastChangedBy: killy $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/ServerView.java $
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

import java.util.HashMap;

import android.app.AlertDialog;
import android.app.TabActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.speech.tts.TextToSpeech;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ScrollView;
import android.widget.SimpleAdapter;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;

import com.nullwire.trace.ExceptionHandler;

public class ServerView extends TabActivity {

	// Actions.
	public static final String CHANNELLIST_ACTION = "org.mangler.android.ChannelListAction";
	public static final String USERLIST_ACTION 	  = "org.mangler.android.UserListAction";
	public static final String CHATVIEW_ACTION	  = "org.mangler.android.ChatViewAction";
	public static final String NOTIFY_ACTION	  = "org.mangler.android.NotifyAction";
	public static final String TTS_NOTIFY_ACTION  = "org.mangler.android.TtsNotifyAction";

	// Events.
	public static final int EVENT_CHAT_JOIN	  = 1;
	public static final int EVENT_CHAT_LEAVE  = 2;
	public static final int EVENT_CHAT_MSG	  = 3;

	// Menu options.
	private final int OPTION_JOIN_CHAT  = 1;
	private final int OPTION_LEAVE_CHAT = 2;
	private final int OPTION_DISCONNECT = 3;
	private final int OPTION_SETTINGS = 4;
	
	// List adapters.
	private SimpleAdapter channelAdapter;
	private SimpleAdapter userAdapter;
	
	// Text to Speech
	TextToSpeech tts;
	boolean ttsInitialized = false;

	// State variables.
	private boolean userInChat = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.server_view);
        
        // Send crash reports to server
        ExceptionHandler.register(this, "http://www.mangler.org/errors/upload.php");
        
        // Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
        // Text to speech init
        tts = new TextToSpeech(this, null);

        // Add tabs.
        TabHost tabhost = getTabHost();
        tabhost.addTab(tabhost.newTabSpec("talk").setContent(R.id.talkView).setIndicator("Talk"));
        tabhost.addTab(tabhost.newTabSpec("channel").setContent(R.id.channelView).setIndicator("Channels"));
        tabhost.addTab(tabhost.newTabSpec("user").setContent(R.id.userView).setIndicator("Users"));
    	tabhost.addTab(tabhost.newTabSpec("chat").setContent(R.id.chatView).setIndicator("Chat"));

        // Create adapters.
	    channelAdapter 	= new SimpleAdapter(this, ChannelList.data, R.layout.channel_row, new String[] { "channelname", "passworded" }, new int[] { R.id.crowtext, R.id.crowpass } );
	    userAdapter 	= new SimpleAdapter(this, UserList.data, R.layout.user_row, new String[] { "userstatus", "username", "channelname" }, new int[] { R.id.urowimg, R.id.urowtext, R.id.urowid } );

	    // Set adapters.
	    ((ListView)findViewById(R.id.channelList)).setAdapter(channelAdapter);
	    ((ListView)findViewById(R.id.userList)).setAdapter(userAdapter);

	    // List item clicks.
	    ((ListView)findViewById(R.id.channelList)).setOnItemClickListener(onListClick);
	    ((ListView)findViewById(R.id.userList)).setOnItemClickListener(onListClick);
	    ((ListView)findViewById(R.id.userList)).setOnItemLongClickListener(onLongListClick);

	    // Register receivers.
        registerReceiver(chatReceiver, new IntentFilter(CHATVIEW_ACTION));
        registerReceiver(channelReceiver, new IntentFilter(CHANNELLIST_ACTION));
        registerReceiver(userReceiver, new IntentFilter(USERLIST_ACTION));
        registerReceiver(notifyReceiver, new IntentFilter(NOTIFY_ACTION));
        registerReceiver(ttsNotifyReceiver, new IntentFilter(TTS_NOTIFY_ACTION));


        // Control listeners.
	    ((EditText)findViewById(R.id.message)).setOnKeyListener(onChatMessageEnter);
	    ((Button)findViewById(R.id.talkButton)).setOnTouchListener(onTalkPress);
    
	    // Restore state.
	    if(savedInstanceState != null) {
	    	userInChat = savedInstanceState.getBoolean("chatopen");
	    	((TextView)findViewById(R.id.messages)).setText(savedInstanceState.getString("chatmessages"));
	    	((EditText)findViewById(R.id.message)).setEnabled(userInChat);
	    }
	
	    ((EditText)findViewById(R.id.message)).setVisibility(userInChat ? TextView.VISIBLE : TextView.GONE);

    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	outState.putString("chatmessages", ((TextView)findViewById(R.id.messages)).getText().toString());
    	outState.putBoolean("chatopen", userInChat);
    	super.onSaveInstanceState(outState);
    }

    @Override
    public void onDestroy() {
    	super.onDestroy();

    	if (Recorder.recording()) {			
    		Recorder.stop();
    	}
    	
    	// Unregister receivers.
		unregisterReceiver(chatReceiver);
        unregisterReceiver(channelReceiver);
        unregisterReceiver(userReceiver);
        unregisterReceiver(notifyReceiver);
        unregisterReceiver(ttsNotifyReceiver);
    }
    

    public boolean onCreateOptionsMenu(Menu menu) {
    	 // Create our menu buttons.
    	menu.add(0, OPTION_JOIN_CHAT, 0, "Join chat").setIcon(R.drawable.menu_join_chat);
        menu.add(0, OPTION_LEAVE_CHAT, 0, "Leave chat").setIcon(R.drawable.menu_leave_chat);
        menu.add(0, OPTION_SETTINGS, 0, "Settings").setIcon(R.drawable.menu_settings);
        menu.add(0, OPTION_DISCONNECT, 0, "Disconnect").setIcon(R.drawable.menu_disconnect);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
    	// Handle menu buttons.
    	final EditText message = (EditText)findViewById(R.id.message);
        switch(item.getItemId()) {
        	case OPTION_JOIN_CHAT:
        		if (!userInChat) {
        			VentriloInterface.joinchat();
        			message.setEnabled(true);
                	message.setVisibility(TextView.VISIBLE);
        		}
        		break;

        	case OPTION_LEAVE_CHAT:
        		if (userInChat) {
        			VentriloInterface.leavechat();
        			message.setEnabled(false);
                	message.setVisibility(TextView.GONE);
        		}
        		break;

        	case OPTION_DISCONNECT:
        		VentriloInterface.logout();
        		finish();
        		return true;
        		
        	case OPTION_SETTINGS:
				Intent intent = new Intent(ServerView.this, Settings.class);
				startActivity(intent);
        		return true;

        	default:
        		return false;
        }
        userInChat = !userInChat;
        return true;
    }

	private BroadcastReceiver chatReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			final TextView messages = (TextView)findViewById(R.id.messages);
			switch(intent.getIntExtra("event", -1)) {
				case EVENT_CHAT_JOIN:
					messages.append("\n* " + intent.getStringExtra("username") + " has joined the chat.");
					break;

				case EVENT_CHAT_LEAVE:
					messages.append("\n* " + intent.getStringExtra("username") + " has left the chat.");
					break;

			 	case EVENT_CHAT_MSG:
			 		messages.append("\n" + intent.getStringExtra("username") + ": " + intent.getStringExtra("message"));
			 		break;
			 }

			// Scroll to bottom.
			final ScrollView chatscroll = (ScrollView)findViewById(R.id.chatScroll);
			chatscroll.post(new Runnable() {
				public void run() {
					chatscroll.fullScroll(ScrollView.FOCUS_DOWN);
				}
			});
		}
	};

	private BroadcastReceiver userReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			userAdapter.notifyDataSetChanged();
		}
	};

	private BroadcastReceiver channelReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			channelAdapter.notifyDataSetChanged();
		}
	};
	
	private BroadcastReceiver notifyReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			String message = intent.getExtras().getString("message");
			Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
		}
	};
	
	private BroadcastReceiver ttsNotifyReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			String message = intent.getExtras().getString("message");
			tts.speak(message, TextToSpeech.QUEUE_ADD, null);
		}
	};

	private void changeChannel(final short channelid) {
		if(VentriloInterface.getuserchannel(VentriloInterface.getuserid()) != channelid) {
			if(VentriloInterface.channelrequirespassword(channelid)) {
				final EditText input = new EditText(this);
				// Create dialog box for password.
				AlertDialog.Builder alert = new AlertDialog.Builder(this)
					.setTitle("Channel is password protected")
					.setMessage("Please insert a password to join this channel.")
					.setView(input)
					.setPositiveButton("OK", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int whichButton) {
							VentriloInterface.changechannel(channelid, input.getText().toString());
						}
					})
					.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int whichButton) {
							// No password entered.
						}
					});
				alert.show();
			}
			else {
				// No password required.
				VentriloInterface.changechannel(channelid, "");
			}
		}
	}

	private OnItemClickListener onListClick = new OnItemClickListener() {
		@SuppressWarnings("unchecked")
		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
			short channelid = (Short)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("channelid");
			changeChannel(channelid);
		}
	};
	
	private void setUserVolume(short id) {
		final CharSequence[] items = {"5 - Loudest", "4", "3", "2", "1 - Muted"};
		final short userid = id;
		AlertDialog.Builder alert = new AlertDialog.Builder(this);
		if (id == VentriloInterface.getuserid()) {
			alert.setTitle("Set Transmit Level");
		} else {
			alert.setTitle("Set User Volume Level");
		}
		alert.setItems(items, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int item) {
				short[] levelList = { 0, 0, 39, 79, 118, 148 };
				int level = levelList[Integer.parseInt(items[item].toString().substring(0, 1))];
				if (userid == VentriloInterface.getuserid()) {
					VentriloInterface.setxmitvolume(level);
				} else {
					VentriloInterface.setuservolume(userid, level);
				} 
				
			}
		});		
		alert.show();
	}
	
	private OnItemLongClickListener onLongListClick = new OnItemLongClickListener() {
		@SuppressWarnings("unchecked")
		public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
			short userid = (Short)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("userid");
			setUserVolume(userid);
			return true;
		}
	};


	private OnTouchListener onTalkPress = new OnTouchListener() {
		public boolean onTouch(View v, MotionEvent m) {
			boolean ptt_toggle = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("ptt_toggle", false);
			
			switch (m.getAction()) {
				case MotionEvent.ACTION_DOWN:
					if (!Recorder.recording()) {
						boolean force_8khz = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("force_8khz", false);
						Recorder.setForce_8khz(force_8khz);
						if (!Recorder.start()) {
							Intent broadcastIntent = new Intent(ServerView.NOTIFY_ACTION);
		    				broadcastIntent.putExtra("message", "Unsupported recording rate for hardware: " + Integer.toString(Recorder.rate()) + "Hz");
		    			    sendBroadcast(broadcastIntent);
		    			    return true;
						}
						((ImageView)findViewById(R.id.transmitStatus)).setImageResource(R.drawable.transmit_on);
					} else if (ptt_toggle) {
						((TextView)findViewById(R.id.recorderInfo)).setText(
								"Last Xmit Info\n\n" +
								"Channel Rate: " + VentriloInterface.getchannelrate(VentriloInterface.getuserchannel(VentriloInterface.getuserid())) + "\n" +
								"Record Rate: " + Recorder.rate() + "\n" +
								"Buffer Size: " + Recorder.buflen() + "\n");
						Recorder.stop();
						((ImageView)findViewById(R.id.transmitStatus)).setImageResource(R.drawable.transmit_off);
					}
					break;
				case MotionEvent.ACTION_UP:
					if (! ptt_toggle) {
						((TextView)findViewById(R.id.recorderInfo)).setText(
								"Last Xmit Info\n\n" +
								"Channel Rate: " + VentriloInterface.getchannelrate(VentriloInterface.getuserchannel(VentriloInterface.getuserid())) + "\n" +
								"Record Rate: " + Recorder.rate() + "\n" +
								"Buffer Size: " + Recorder.buflen() + "\n");
						Recorder.stop();
						((ImageView)findViewById(R.id.transmitStatus)).setImageResource(R.drawable.transmit_off);
					}
					break;
			}
			return true;
		}
	};

	private OnKeyListener onChatMessageEnter = new OnKeyListener() {
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
}
