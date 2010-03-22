package org.mangler;

public class VentriloEventData {
	public short 	type;
	
	public class _pcm {
		public int length;
		public short send_type;
		public int rate;
		public byte channels;
	};
	_pcm pcm = new _pcm();
	
	public class _user {
		public short id;
	}
	_user user = new _user();
	
	public class _data {
		byte[] sample = new byte[32768];
	}
	_data data = new _data();
	
}
