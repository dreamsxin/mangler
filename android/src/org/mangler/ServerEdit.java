/*
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

package org.mangler;

import android.app.Activity;
import android.database.Cursor;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class ServerEdit extends Activity{

	private Long rowID;
	private EditText serverText;
	private EditText hostText;
	private EditText portText;
	private EditText passText;
	private EditText userText;
	private EditText phoneticText;

	private ManglerDBAdapter dbHelper;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

		// Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);

        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();

        setContentView(R.layout.server_edit);

        serverText = (EditText)findViewById(R.id.editServer);
        hostText = (EditText)findViewById(R.id.editHost);
        portText = (EditText)findViewById(R.id.editPort);
        passText = (EditText)findViewById(R.id.editPass);
        userText = (EditText)findViewById(R.id.editUser);
        phoneticText = (EditText)findViewById(R.id.editPhonetic);

        Button saveButton = (Button)findViewById(R.id.editSave);

        rowID = null;

        /*if (savedInstanceState != null) {
        	rowID = savedInstanceState.getLong(ManglerDBAdapter.KEY_ROWID);
        }*/

        //if (rowID == null) {
        	Bundle extras = getIntent().getExtras();
        	if (extras != null) {
        		rowID = extras.getLong(ManglerDBAdapter.KEY_ROWID);
        	}
        //}

        populateFields();

        saveButton.setOnClickListener(new View.OnClickListener() {
        	public void onClick(View view) {
        		saveState();
        		setResult(RESULT_OK);
        		finish();
        	}
		});
    }

    @Override
    protected void onDestroy() {
    	super.onDestroy();
    	dbHelper.close();
    }

    private void populateFields() {
        if (rowID != null) {
            Cursor servers = dbHelper.fetchServer(rowID);
            startManagingCursor(servers);
            serverText.setText(servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERNAME)));
            hostText.setText(servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_HOSTNAME)));
            portText.setText(((Integer)servers.getInt(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PORTNUMBER))).toString());
            passText.setText(servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PASSWORD)));
            userText.setText(servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_USERNAME)));
            phoneticText.setText(servers.getString(servers.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PHONETIC)));
        }
    }

    /*@Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        if (rowID != null) {
        	outState.putLong(ManglerDBAdapter.KEY_ROWID, rowID);
        }
    }*/

    private void saveState() {
        String servername = serverText.getText().toString();
        String hostname = hostText.getText().toString();
        int port = 0;
        if (!portText.getText().toString().equals(""))
        	port = Integer.parseInt(portText.getText().toString());

        if (port > 65535) {
        	port = 3874;
        }

        String password = passText.getText().toString();
        String username = userText.getText().toString();
        String phonetic = phoneticText.getText().toString();

        if (rowID == null) {
        	dbHelper.createServer(servername, hostname, port, password, username, phonetic);
        } else {
            dbHelper.updateServer(rowID, servername, hostname, port, password, username, phonetic);
        }
    }
}
