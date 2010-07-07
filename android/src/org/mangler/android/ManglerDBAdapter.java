/*
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-06-13 00:25:11 +0200 (Sun, 13 Jun 2010) $
 * $Revision: 918 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/ManglerDBAdapter.java $
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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class ManglerDBAdapter {

	public static final String KEY_ROWID = "_id";
    public static final String KEY_SERVERNAME = "servername";
    public static final String KEY_HOSTNAME = "hostname";
    public static final String KEY_PORTNUMBER = "portnumber";
    public static final String KEY_PASSWORD = "password";
    public static final String KEY_USERNAME = "username";
    public static final String KEY_PHONETIC = "phonetic";

    private static final String TAG = "ManglerDBAdapter";
    
    private DatabaseHelper dbHelper;
    private SQLiteDatabase db;
    
    // SQL string for creating database
    private static final String DATABASE_CREATE =
        "create table servers (_id integer primary key autoincrement, servername text not null, hostname text not null, portnumber integer not null, password text not null, username text not null, phonetic text not null);";

    private static final String DATABASE_NAME = "manglerdata";
    private static final String DATABASE_SERVER_TABLE = "servers";
    private static final int DATABASE_VERSION = 2;

    private final Context context;

    private static class DatabaseHelper extends SQLiteOpenHelper {

        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {

            db.execSQL(DATABASE_CREATE);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Log.w(TAG, "Upgrading database from version " + oldVersion + " to "
                    + newVersion + ", which will destroy all old data");
            db.execSQL("DROP TABLE IF EXISTS servers");
            onCreate(db);
        }
    }

    /**
     * Constructor - takes the context to allow the database to be
     * opened/created
     * 
     * @param context the Context within which to work
     */
    public ManglerDBAdapter(Context context) {
        this.context = context;
    }

    /**
     * Open the mangler database. If it cannot be opened, try to create a new
     * instance of the database. If it cannot be created, throw an exception to
     * signal the failure
     * 
     * @return this (self reference, allowing this to be chained in an
     *         initialization call)
     * @throws SQLException if the database could be neither opened or created
     */
    public ManglerDBAdapter open() throws SQLException {
        dbHelper = new DatabaseHelper(context);
        db = dbHelper.getWritableDatabase();
        return this;
    }
    
    public void close() {
        dbHelper.close();
    }
    
    /**
     * Create a new server. If the server is successfully created return the new rowId for that server, otherwise return
     * a -1 to indicate failure.
     * 
     * @param servername
     * @param hostname
     * @param portnumber
     * @param password
     * @param username
     * @param phonetic
     * @return rowId or -1 if failed
     */
    public long createServer(String servername, String hostname, int portnumber, String password, String username, String phonetic) {
        ContentValues initialValues = new ContentValues();
        initialValues.put(KEY_SERVERNAME, servername);
        initialValues.put(KEY_HOSTNAME, hostname);
        initialValues.put(KEY_PORTNUMBER, portnumber);
        initialValues.put(KEY_PASSWORD, password);
        initialValues.put(KEY_USERNAME, username);
        initialValues.put(KEY_PHONETIC, phonetic);

        return db.insert(DATABASE_SERVER_TABLE, null, initialValues);
    }
    
    /**
     * Clone a server. If the server is successfully created return the new rowId for that server, otherwise return
     * a -1 to indicate failure.
     * 
     * @param rowId of server to copy
     * @return rowId of copied server or -1 if failed
     */
    public long cloneServer(long rowId) {
    	Cursor cursor = fetchServer(rowId);
        ContentValues initialValues = new ContentValues();
        initialValues.put(KEY_SERVERNAME, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERNAME)));
        initialValues.put(KEY_HOSTNAME, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_HOSTNAME)));
        initialValues.put(KEY_PORTNUMBER, cursor.getInt(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PORTNUMBER)));
        initialValues.put(KEY_PASSWORD, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PASSWORD)));
        initialValues.put(KEY_USERNAME, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_USERNAME)));
        initialValues.put(KEY_PHONETIC, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PHONETIC)));

        return db.insert(DATABASE_SERVER_TABLE, null, initialValues);
    }
    
    /**
     * Delete the server with the given rowId
     * 
     * @param rowId id of server to delete
     * @return true if deleted, false otherwise
     */
    public boolean deleteServer(long rowId) {

        return db.delete(DATABASE_SERVER_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
    }
    
    /**
     * Return a Cursor over the list of all servers in the database
     * 
     * @return Cursor over all servers
     */
    public Cursor fetchServers() {

        return db.query(DATABASE_SERVER_TABLE, new String[] {KEY_ROWID, KEY_SERVERNAME, 
        		KEY_HOSTNAME, KEY_PORTNUMBER, KEY_PASSWORD, KEY_USERNAME, KEY_PHONETIC}, null, null, null, null, null);
    }
    
    /**
     * Return a Cursor positioned at the server that matches the given rowId
     * 
     * @param rowId id of server to retrieve
     * @return Cursor positioned to matching server, if found
     * @throws SQLException if server could not be found/retrieved
     */
    public Cursor fetchServer(long rowId) throws SQLException {

        Cursor cursor =
                db.query(true, DATABASE_SERVER_TABLE, new String[] {KEY_ROWID,
                        KEY_SERVERNAME, KEY_HOSTNAME, KEY_PORTNUMBER, KEY_PASSWORD, KEY_USERNAME, KEY_PHONETIC}, 
                        KEY_ROWID + "=" + rowId, null, null, null, null, null);
        if (cursor != null) {
            cursor.moveToFirst();
        }
        return cursor;
    }

    /**
     * Update the server using the details provided. The server to be updated is
     * specified using the rowId, and it is altered to use the values passed in
     * 
     * @param rowId id of server to update
     * @param servername
     * @param hostname
     * @param portnumber
     * @param password
     * @param username
     * @param phonetic
     * @return true if the server was successfully updated, false otherwise
     */
    public boolean updateServer(long rowId, String servername, String hostname, int portnumber, String password, String username, String phonetic) {
        ContentValues args = new ContentValues();
        args.put(KEY_SERVERNAME, servername);
        args.put(KEY_HOSTNAME, hostname);
        args.put(KEY_PORTNUMBER, portnumber);
        args.put(KEY_PASSWORD, password);
        args.put(KEY_USERNAME, username);
        args.put(KEY_PHONETIC, phonetic);

        return db.update(DATABASE_SERVER_TABLE, args, KEY_ROWID + "=" + rowId, null) > 0;
    }
}
