package org.mangler;

import java.nio.ByteBuffer;

public class EventData 
{
	private class _status {
		byte	percent;
		String	message;
	}
	
	private class _error {
		short	code;
		byte	disconnected;
		String	message;
	}
	
	private class _user {
		short	id;
	}
	
	private class _channel {
		short	id;
	}
	
	private class _text {
		String	name;
		String 	password;
		String	phonetic;
		String	comment;
		String	url;
		String	integration_text;
	}
	
	private class _pcm {
		int   	length;
		short 	send_type;
		int   	rate;
	}
	
	private class _data {
		ByteBuffer sample;
	}
	
	public short 	type;
	public short	ping;
	public int		timestamp;
	public int		flags;
	public _status	status;
	public _error	error;
	public _user	user;
	public _channel	channel;
	public _text	text;
	public _pcm 	pcm;
	public _data	data;
}
