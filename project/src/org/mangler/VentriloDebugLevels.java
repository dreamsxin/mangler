package org.mangler;

public class VentriloDebugLevels {
	public static final int V3_DEBUG_NONE               = 0;
	public static final int V3_DEBUG_STATUS             = 1;
	public static final int V3_DEBUG_ERROR              = 1 << 2;
	public static final int V3_DEBUG_STACK              = 1 << 3;
	public static final int V3_DEBUG_INTERNAL           = 1 << 4;
	public static final int V3_DEBUG_PACKET             = 1 << 5;
	public static final int V3_DEBUG_PACKET_PARSE       = 1 << 6;
	public static final int V3_DEBUG_PACKET_ENCRYPTED   = 1 << 7;
	public static final int V3_DEBUG_MEMORY             = 1 << 8;
	public static final int V3_DEBUG_SOCKET             = 1 << 9;
	public static final int V3_DEBUG_NOTICE             = 1 << 10;
	public static final int V3_DEBUG_INFO               = 1 << 11;
	public static final int V3_DEBUG_MUTEX              = 1 << 12;
	public static final int V3_DEBUG_EVENT              = 1 << 13;
	public static final int V3_DEBUG_ALL                = 65535;
}
