package org.mangler;

/* I welcome anyone to figure out why my native GetObjectField calls are failing for objects inside an instance of this class. For now, just a list of prefixed variables. */

public class VentriloEventData {
	public short 	type;
	
	/* pcm */
	public int 		pcm_length;
    public short 	pcm_send_type;
    public int		pcm_rate;
    public byte 	pcm_channels;
    
    /* data */
    public byte[] 	data_sample = new byte[32768];
    
    /* user */
    public short	user_id;
    
}
