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

import java.util.HashMap;

import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.app.TabActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TabHost;
import android.widget.TabWidget;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;

import com.nullwire.trace.ExceptionHandler;

public class ServerView extends TabActivity {
	// Server ID that we're connected to
	private static int serverid = -1;
	public String servername = "Server View";
	
	// Database connection
	private ManglerDBAdapter dbHelper;

	// Actions.
	public static final String EVENT_ACTION				= "org.mangler.android.EventAction";
	public static final String NOTIFY_ACTION			= "org.mangler.android.NotifyAction";

	// Menu options.
	private final int OPTION_JOIN_CHAT  = 1;
	private final int OPTION_HIDE_TABS  = 2;
	private final int OPTION_DISCONNECT = 3;
	private final int OPTION_SETTINGS = 4;
	
	// Context menu options (does java not have enums?)
	private final int CM_OPTION_VOLUME = 1;
	private final int CM_OPTION_COMMENT = 2;
	private final int CM_OPTION_SEND_PAGE = 3;
	private final int CM_OPTION_KICK= 4;
	private final int CM_OPTION_BAN = 6;
	private final int CM_OPTION_MUTE = 7;
	private final int CM_OPTION_GLOBAL_MUTE= 8;
	private final int CM_OPTION_MOVE_USER = 9;
	
	// List adapters.
	private SimpleAdapter channelAdapter;
	private SimpleAdapter userAdapter;
	
	EventHandler eventHandler = null;
	
	// Text to Speech
	TTSWrapper ttsWrapper = null;

	// State variables.
	private boolean userInChat = false;
	private boolean tabsHidden = false;
	
	// WakeLock
	private PowerManager.WakeLock wl;
	
	// Notifications
	private NotificationManager notificationManager;
	private static final int ONGOING_NOTIFICATION = 1;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
		boolean fullscreen = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("fullscreen", false);
		if (fullscreen) {
			getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
			requestWindowFeature(Window.FEATURE_NO_TITLE);
		}
        setContentView(R.layout.server_view);
        
        // Get the server id that we're connected to and set up the database adapter
        serverid = getIntent().getIntExtra("serverid", 0);
        
        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();
        
        // Send crash reports to server
        ExceptionHandler.register(this, "http://www.mangler.org/errors/upload.php");
        
        // Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
        // Text to speech init     
        if (ttsWrapper == null) {
        	ttsWrapper = TTSWrapper.getInstance(this);
        }
        
        // Chat TextView
        TextView chatMessages = (TextView)findViewById(R.id.messages);
        
        // Add tabs.
        TabHost tabhost = getTabHost();
        tabhost.addTab(tabhost.newTabSpec("channel").setContent(R.id.channelView).setIndicator("Channels"));
        tabhost.addTab(tabhost.newTabSpec("user").setContent(R.id.userView).setIndicator("Users"));
    	tabhost.addTab(tabhost.newTabSpec("chat").setContent(R.id.chatView).setIndicator("Chat"));
        tabhost.addTab(tabhost.newTabSpec("talk").setContent(R.id.talkView).setIndicator("Debug"));

        // Create adapters.
	    channelAdapter 	= new SimpleAdapter(this, ChannelList.data, R.layout.channel_row, new String[] { "indent", "xmitStatus", "name" }, new int[] { R.id.indent, R.id.crowimg, R.id.crowtext } );
	    userAdapter 	= new SimpleAdapter(this, UserList.data, R.layout.user_row, new String[] { "userstatus", "username", "comment" }, new int[] { R.id.urowimg, R.id.urowtext, R.id.urowid } );

	    // Set adapters.
	    ((ListView)findViewById(R.id.channelList)).setAdapter(channelAdapter);
	    ((ListView)findViewById(R.id.userList)).setAdapter(userAdapter);

	    // List item clicks.
	    ((ListView)findViewById(R.id.channelList)).setOnItemClickListener(onListClick);

	    // Register receivers.
        registerReceiver(eventReceiver, new IntentFilter(EVENT_ACTION));
        registerReceiver(notifyReceiver, new IntentFilter(NOTIFY_ACTION));

        // Control listeners.
	    ((EditText)findViewById(R.id.message)).setOnKeyListener(onChatMessageEnter);
	    ((Button)findViewById(R.id.talkButton)).setOnTouchListener(onTalkPress);
    
	    // Restore state.
	    if(savedInstanceState != null) {
	    	servername = savedInstanceState.getString("servername");
	    	this.setTitle(servername + " - Ping: checking");
	    	userInChat = savedInstanceState.getBoolean("chatopen");
	    	chatMessages.setText(savedInstanceState.getString("chatmessages"));
	    	((EditText)findViewById(R.id.message)).setEnabled(userInChat);
	    }
	    
	    eventHandler = new EventHandler(this);
	
	    ((EditText)findViewById(R.id.message)).setVisibility(userInChat ? TextView.VISIBLE : TextView.GONE);
	    
	    // Get a wakelock to prevent sleeping and register an onchange preference callback
	    PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
	    wl = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "Mangler");
		boolean prevent_sleep = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("prevent_sleep", false);
		if (prevent_sleep) {
			if (!wl.isHeld()) {
				wl.acquire();
			}
		}
		
		registerForContextMenu(((ListView)findViewById(R.id.channelList)));
		
		notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		if (! VentriloInterface.isloggedin()) {
			connectToServer();
		}
		eventHandler.process();
		notifyAdaptersDataSetChanged();
	}
    
    @Override
    protected void onResume() {
		boolean prevent_sleep = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("prevent_sleep", false);
    	super.onResume();
		if (prevent_sleep) {
			if (!wl.isHeld()) {
				wl.acquire();
			}
		} else {
			if (wl.isHeld()) {
				wl.release();
			}
		}
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	outState.putString("servername", servername);
    	outState.putString("chatmessages", ((TextView)findViewById(R.id.messages)).getText().toString());
    	outState.putBoolean("chatopen", userInChat);
    	super.onSaveInstanceState(outState);
    }
    
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_CAMERA) {
			startPtt();
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_CAMERA) {
			stopPtt();
			return true;
		}
		return super.onKeyUp(keyCode, event);
	}

    @Override
    public void onDestroy() {
    	super.onDestroy();
    	
    	if (Recorder.recording()) {			
    		Recorder.stop();
    	}
    	
    	// release a wakelock if we have one
    	if (wl.isHeld()) {
    		wl.release();
    	}
    	    	
    	if (ttsWrapper != null) {
    		ttsWrapper.shutdown();
    	}
    	
    	dbHelper.close();
    	
    	// Unregister receivers.
        unregisterReceiver(eventReceiver);
        unregisterReceiver(notifyReceiver);
    }
    

    public boolean onCreateOptionsMenu(Menu menu) {
    	 // Create our menu buttons.
    	menu.add(0, OPTION_JOIN_CHAT, 0, "Join chat").setIcon(R.drawable.menu_join_chat);
    	if (tabsHidden) {
    		menu.add(0, OPTION_HIDE_TABS, 0, "Hide tabs").setIcon(R.drawable.menu_show_tabs);
        	final TabWidget tabWidget = (TabWidget)findViewById(android.R.id.tabs);
			tabWidget.setEnabled(false);
			tabWidget.setVisibility(TextView.GONE);
    	} else {
    		menu.add(0, OPTION_HIDE_TABS, 0, "Hide tabs").setIcon(R.drawable.menu_hide_tabs);
    	}
        menu.add(0, OPTION_SETTINGS, 0, "Settings").setIcon(R.drawable.menu_settings);
        menu.add(0, OPTION_DISCONNECT, 0, "Disconnect").setIcon(R.drawable.menu_disconnect);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
    	// Handle menu buttons.
    	final EditText message = (EditText)findViewById(R.id.message);
    	final TabWidget tabWidget = (TabWidget)findViewById(android.R.id.tabs);
        switch(item.getItemId()) {
        	case OPTION_JOIN_CHAT:
        		if (!userInChat) {
        			VentriloInterface.joinchat();
        			message.setEnabled(true);
        			message.setVisibility(TextView.VISIBLE);
        			userInChat = true;
        			item.setIcon(R.drawable.menu_leave_chat);
        			item.setTitle("Leave chat");
        		} else {
        			VentriloInterface.leavechat();
        			message.setEnabled(false);
        			message.setVisibility(TextView.GONE);
        			userInChat = false;
        			item.setIcon(R.drawable.menu_join_chat);
        			item.setTitle("Join chat");
        		}
        		break;
        		
        	case OPTION_HIDE_TABS:
        		if (tabsHidden) {
        			tabsHidden = false;
        			item.setIcon(R.drawable.menu_hide_tabs);
        			item.setTitle("Hide tabs");
        			tabWidget.setEnabled(true);
        			tabWidget.setVisibility(TextView.VISIBLE);
        		} else {
        			tabsHidden = true;
        			item.setIcon(R.drawable.menu_show_tabs);
        			item.setTitle("Show tabs");
        			tabWidget.setEnabled(false);
        			tabWidget.setVisibility(TextView.GONE);
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
        return true;
    }

	public void addChatMessage(String username, String message) {
		final TextView messages = (TextView)findViewById(R.id.messages);
		messages.append("\n" + username  + ": " + message);
	}
	
	public void addChatUser(String username) {
		final TextView messages = (TextView)findViewById(R.id.messages);
		messages.append("\n* " + username + " has joined the chat.");
	}
	
	public void removeChatUser(String username) {
		final TextView messages = (TextView)findViewById(R.id.messages);
		messages.append("\n* " + username + " has left the chat.");
	}
	
	public void notifyAdaptersDataSetChanged() {
		ChannelListEntity entity = new ChannelListEntity(
				ChannelListEntity.CHANNEL,
				VentriloInterface.getuserchannel(VentriloInterface.getuserid()));
		if (findViewById(R.id.userViewHeader) != null) {
			((TextView)findViewById(R.id.userViewHeader)).setText(
					"" + UserList.data.size() + " Users | " +
					(entity.id == 0 ? "Lobby" : entity.name)
					);
		}
		userAdapter.notifyDataSetChanged();
		channelAdapter.notifyDataSetChanged();
	}
	
	
	private BroadcastReceiver eventReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			eventHandler.process();
		}
	};
	
	private BroadcastReceiver notifyReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			Toast.makeText(ServerView.this, intent.getStringExtra("message"), Toast.LENGTH_SHORT).show();
		}
	};
	
	private void changeChannel(final short channelid) {
		if(VentriloInterface.getuserchannel(VentriloInterface.getuserid()) != channelid) {
			final int protectedby;
			if((protectedby = VentriloInterface.channelrequirespassword(channelid)) > 0) {
				final String password = dbHelper.getPassword(serverid, protectedby);
				final EditText input = new EditText(this);
				input.setText(password);
				// Create dialog box for password.
				AlertDialog.Builder alert = new AlertDialog.Builder(this)
					.setTitle("Channel is password protected")
					.setMessage("Please insert a password to join this channel.")
					.setView(input)
					.setPositiveButton("OK", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int whichButton) {
							if (input.getText().toString() != password) {
								dbHelper.setPassword(serverid, protectedby, input.getText().toString());
							}
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
			short channelid = 0;
			int type = (Integer)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("type");
			if (type == ChannelListEntity.CHANNEL) {
				channelid = (Short)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("id");
			} else {
				channelid = (Short)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("parentid");
			}
			changeChannel(channelid);
		}
	};
	
	private void setUserVolume(short id) {
		final CharSequence[] items = {"5 - Loudest", "4", "3", "2", "1"};
		final short userid = id;
		AlertDialog.Builder alert = new AlertDialog.Builder(this);
		if (id == VentriloInterface.getuserid()) {
			alert.setTitle("Set Transmit Level");
		} else {
			alert.setTitle("Set User Volume Level");
		}
		VentriloEventData evdata = new VentriloEventData();
		VentriloInterface.getuser(evdata, id);
		final String username = new String(evdata.text.name, 0, (new String(evdata.text.name).indexOf(0)));
		alert.setItems(items, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int item) {
				short[] levelList = { 0, 5, 10, 79, 118, 148 };
				int level = levelList[Integer.parseInt(items[item].toString().substring(0, 1))];
				if (userid == VentriloInterface.getuserid()) {
					Log.d("mangler", "setting xmit volume for me (" + username + ") to volume level " + level);
					dbHelper.setVolume(serverid, username, level);
					VentriloInterface.setxmitvolume(level);
				} else {
					Log.d("mangler", "setting volume for " + username + " to volume level " + level);
					dbHelper.setVolume(serverid, username, level);
					VentriloInterface.setuservolume(userid, level);
				} 
				
			}
		});		
		alert.show();
	}
	
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		AdapterContextMenuInfo cmi = (AdapterContextMenuInfo) menuInfo;
		
		ChannelListEntity entity = new ChannelListEntity(ChannelList.data.get(cmi.position));
		
		if (entity.type == ChannelListEntity.USER) {
			if (entity.id != VentriloInterface.getuserid()) {
				// create the menu for other users
				int itempos = 1;
				boolean serveradmin = VentriloInterface.getpermission("serveradmin");
				Log.d("mangler", "am i a server admin? " + serveradmin);
				menu.setHeaderTitle(entity.name);
				
				menu.add(Menu.NONE, CM_OPTION_VOLUME, itempos++, "Set Volume");
				if (entity.comment != "" ||	entity.url != "") {
					menu.add(Menu.NONE, CM_OPTION_COMMENT, itempos++, "View Comment/URL");
				}
				if (dbHelper.getVolume(serverid, entity.name) == 0) {
					menu.add(Menu.NONE, CM_OPTION_MUTE, itempos++, "Unmute");
				} else {
					menu.add(Menu.NONE, CM_OPTION_MUTE, itempos++, "Mute");
				}
				if (serveradmin || VentriloInterface.getpermission("sendpage")) {
					menu.add(Menu.NONE, CM_OPTION_SEND_PAGE, itempos++, "Send Page");
				}
				if (!entity.inMyChannel()) {
					if (serveradmin || VentriloInterface.getpermission("moveuser")) {
						menu.add(Menu.NONE, CM_OPTION_MOVE_USER, itempos++, "Move User to Your Channel");
					}
				}
				if (serveradmin || VentriloInterface.getpermission("kickuser")) {
					menu.add(Menu.NONE, CM_OPTION_KICK, itempos++, "Kick");
				}
				if (serveradmin || VentriloInterface.getpermission("banuser")) {
					menu.add(Menu.NONE, CM_OPTION_BAN, itempos++, "Ban");
				}
				if (serveradmin) {
					menu.add(Menu.NONE, CM_OPTION_GLOBAL_MUTE, itempos++, "Global Mute");
				}
			} else {
				// create menu for our own options
				int itempos = 1;

				menu.add(Menu.NONE, CM_OPTION_VOLUME, itempos++, "Set Transmit Level");
				menu.add(Menu.NONE, CM_OPTION_COMMENT, itempos++, "Set Comment/URL").setVisible(false);
			}
		}
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo cmi = (AdapterContextMenuInfo) item.getMenuInfo();

		short id = Short.parseShort(ChannelList.data.get(cmi.position).get("id").toString());
		ChannelListEntity entity = new ChannelListEntity(ChannelListEntity.USER, id); 
		switch (item.getItemId()) {
			case CM_OPTION_VOLUME:
				setUserVolume(id);
				break;
			case CM_OPTION_MUTE:
				if ((dbHelper.getVolume(serverid, entity.name)) == 0) {
					dbHelper.setVolume(serverid, entity.name, 79);
					VentriloInterface.setuservolume(entity.id, 79);
				} else {
					dbHelper.setVolume(serverid, entity.name, 0);
					VentriloInterface.setuservolume(entity.id, 0);
				}
				break;
			case CM_OPTION_GLOBAL_MUTE:
				VentriloInterface.globalmute(entity.id);
				break;
			case CM_OPTION_SEND_PAGE:
				VentriloInterface.sendpage(entity.id);
				break;
			case CM_OPTION_MOVE_USER:
				Log.e("mangler", "moving user" + entity.id + " to channel " + VentriloInterface.getuserchannel(VentriloInterface.getuserid()));
				if (!entity.inMyChannel()) {
					VentriloInterface.forcechannelmove(entity.id, VentriloInterface.getuserchannel(VentriloInterface.getuserid()));
				}
				break;
			case CM_OPTION_KICK:
				kickUser(id);
				break;
			case CM_OPTION_BAN:
				banUser(id);
				break;
			case CM_OPTION_COMMENT:
				if (id == VentriloInterface.getuserid()) {
					//setComment();
				} else {
					viewComment(id);
				}
				break;
		}
		return super.onContextItemSelected(item);
	}

	private void kickUser(short id) {
		final EditText input = new EditText(this);
		final short userid = id;
		// Create dialog box for password.
		AlertDialog.Builder alert = new AlertDialog.Builder(this)
			.setTitle("Kick Reason")
			.setMessage("Please enter a reason for kicking this user")
			.setView(input)
			.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					VentriloInterface.kick(userid, input.getText().toString());
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
				}
			});
		alert.show();
	}

	private void banUser(short id) {
		final EditText input = new EditText(this);
		final short userid = id;
		// Create dialog box for password.
		AlertDialog.Builder alert = new AlertDialog.Builder(this)
			.setTitle("Ban Reason")
			.setMessage("Please enter a reason for banning this user (note: you cannot unban users from this interface)")
			.setView(input)
			.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					VentriloInterface.ban(userid, input.getText().toString());
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
				}
			});
		alert.show();
	}

	public void setUserVolumeFromDatabase(ChannelListEntity entity) {
		int level = dbHelper.getVolume(serverid, entity.name);
		VentriloInterface.setuservolume(entity.id, level);
	}
	
	private void viewComment(short id) {
		ChannelListEntity entity = new ChannelListEntity(ChannelListEntity.USER, id);
		AlertDialog.Builder alert = new AlertDialog.Builder(this)
		.setTitle(entity.name)
		.setMessage("Comment: " + entity.comment + "\n" + "URL: " + entity.url);
		alert.show();
	}

	private OnTouchListener onTalkPress = new OnTouchListener() {
		public boolean onTouch(View v, MotionEvent m) {
			boolean ptt_toggle = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("ptt_toggle", false);
			
			switch (m.getAction()) {
				case MotionEvent.ACTION_DOWN:
					if (!Recorder.recording()) {
						startPtt();
					} else if (ptt_toggle) {
						stopPtt();
					}
					break;
				case MotionEvent.ACTION_UP:
					if (! ptt_toggle) {
						stopPtt();
					}
					break;
			}
			return true;
		}
	};
	
	private void startPtt() {
		boolean force_8khz = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("force_8khz", false);
		Recorder.setForce_8khz(force_8khz);
		if (!Recorder.start()) {
			Toast.makeText(this, "Unsupported recording rate for hardware: " + Integer.toString(Recorder.rate()) + "Hz", Toast.LENGTH_SHORT).show();
			return;
		}
		((ImageView)findViewById(R.id.transmitStatus)).setImageResource(R.drawable.xmit_on);
		UserList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_on);
		userAdapter.notifyDataSetChanged();
		ChannelList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_on);
		channelAdapter.notifyDataSetChanged();
	}
	
	private void stopPtt() {
		((TextView)findViewById(R.id.recorderInfo)).setText(
				"Last Xmit Info\n\n" +
				"Channel Rate: " + VentriloInterface.getchannelrate(VentriloInterface.getuserchannel(VentriloInterface.getuserid())) + "\n" +
				"Record Rate: " + Recorder.rate() + "\n" +
				"Buffer Size: " + Recorder.buflen() + "\n");
		Recorder.stop();
		((ImageView)findViewById(R.id.transmitStatus)).setImageResource(R.drawable.xmit_off);
		UserList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_off);
		userAdapter.notifyDataSetChanged();
		ChannelList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_off);
		channelAdapter.notifyDataSetChanged();
	}

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
	
	private void connectToServer() {
		final ProgressDialog dialog = ProgressDialog.show(this, "", "Connecting. Please wait...", true);

		final Cursor servers = dbHelper.fetchServer(serverid);
		startManagingCursor(servers);
		final String servername = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_SERVERNAME));
		this.servername = servername;
		final String hostname = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_HOSTNAME));
		final String port = Integer.toString(servers.getInt(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PORTNUMBER)));
		final String username = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_USERNAME));
		final String password = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PASSWORD));
		final String phonetic = servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PHONETIC));
		
		this.setTitle(servername + " - Ping: checking");
		
		// Get rid of any data from previous connections.
		UserList.clear();
		ChannelList.clear();

		// Add lobby.
		ChannelListEntity entity = new ChannelListEntity();
		entity.id = 0;
		entity.name = "Lobby";
		entity.type = ChannelListEntity.CHANNEL;
		entity.parentid = 0;
		ChannelList.add(entity);

		Thread t = new Thread(new Runnable() {
			public void run() {
				if (VentriloInterface.login(
						hostname + ":" + port, 
						username, 
						password, 
						phonetic)) {
					dialog.dismiss();
					// Start receiving packets.
					startRecvThread();

					Intent notificationIntent = new Intent(ServerView.this, ServerView.class);
					notificationIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
					Notification notification = new Notification(R.drawable.notification, "Connected to server", System.currentTimeMillis());
					notification.setLatestEventInfo(getApplicationContext(), "Mangler", "Connected to " + servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_SERVERNAME)), PendingIntent.getActivity(ServerView.this, 0, notificationIntent, 0));
					notification.flags = Notification.FLAG_ONGOING_EVENT;
					notificationManager.notify(ONGOING_NOTIFICATION, notification);
				} else {
					dialog.dismiss();
					VentriloEventData data = new VentriloEventData();
					VentriloInterface.error(data);
					sendBroadcast(new Intent(ServerView.NOTIFY_ACTION)
						.putExtra("message", "Connection to server failed:\n" + EventService.StringFromBytes(data.error.message)));
				}
			}
		});
		t.setPriority(10);
		t.start();
	}

	private void startRecvThread() {
		Runnable recvRunnable = new Runnable() {
			public void run() {
				while (VentriloInterface.recv())
					;
			}
		};
		(new Thread(recvRunnable)).start();

	}
	
	public void tts(String text) {
		boolean enable_tts = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("enable_tts", false);
		Log.e("mangler", "enable tts" + enable_tts);
		if (enable_tts) {
			ttsWrapper.speak(text);
		}
	}
}
