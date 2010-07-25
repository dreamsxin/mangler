package org.mangler.android;

import java.util.HashMap;

public class ChannelListEntity {
	public short id;
	public String name = "";
	public String phonetic = "";
	public String comment = "";
	public String url = "";
	public String indent = "";
	public int type = -1;
	public int passwordProtected = 0;
	public short parentid = 0;
	public int xmitStatus = R.drawable.transmit_off;
	
	public static String stringFromBytes(byte[] bytes) {
    	return new String(bytes, 0, (new String(bytes).indexOf(0)));
	}
	
	public ChannelListEntity(HashMap<String, Object> entity) {
		id = Short.parseShort(entity.get("id").toString());
		name = entity.get("name").toString();
		phonetic = entity.get("phonetic").toString();
		comment = entity.get("comment").toString();
		url = entity.get("url").toString();
		indent = entity.get("indent").toString();
		type = Integer.parseInt(entity.get("type").toString());
		passwordProtected = Integer.parseInt(entity.get("passwordProtected").toString());
		parentid = Short.parseShort(entity.get("parentid").toString());
		xmitStatus = Integer.parseInt(entity.get("xmitStatus").toString());
	}
	
	public ChannelListEntity(int type, short id) {
		VentriloEventData data = new VentriloEventData();
		switch (type) {
			case ChannelList.USER:
				VentriloInterface.getuser(data, id);
				this.id = id;
				name = stringFromBytes(data.text.name);
				comment = stringFromBytes(data.text.comment);
				url = stringFromBytes(data.text.url);
				this.type = type;
				parentid = VentriloInterface.getuserchannel(id);
				break;
			case ChannelList.CHANNEL:
				VentriloInterface.getchannel(data, id);
				this.id = id;
				name = stringFromBytes(data.text.name);
				comment = stringFromBytes(data.text.comment);
				this.type = type;
				parentid = data.data.channel.parent;
				passwordProtected = VentriloInterface.channelrequirespassword(id);
				break;
		}
		
	}
	
	public ChannelListEntity() {
	}

	public HashMap<String, Object> toHashMap() {
		HashMap<String, Object> entity = new HashMap<String, Object>();;
		entity.put("id", id);
		entity.put("name", name);
		entity.put("comment", comment);
		entity.put("url", url);
		entity.put("indent", indent);
		entity.put("type", type);
		entity.put("passwordProtected", passwordProtected);
		entity.put("parentid", parentid);
		entity.put("xmitStatus", xmitStatus);
		return entity;
	}
}
