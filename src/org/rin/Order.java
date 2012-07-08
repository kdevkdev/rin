package org.rin;

//order error codes:
//-100: no blocking order/call
//-7: blocking, but order did not assign a return value
// 1: general success
// 0: general failure

public class Order {
	
	public static final int O_LOAD_ROM = 1;
	public static final int O_LOAD_STATE = 2;
	public static final int O_SAVE_STATE = 3;
	public static final int O_REWIND = 4;
	public static final int O_TAKESCREENSHOT = 5;
	public static final int O_LOAD_CONFIG = 6;
	public static final int O_SAVE_CONFIG = 7;
	public static final int O_LOAD_CONFIG_GLOBAL = 8;
	public static final int O_SAVE_CONFIG_GLOBAL = 9;


	
	public Order()
	{
	}
	
	public int stringorder(int order, String val)
	{
		return giveString(order, val, val.length());
	}
	public int intorder(int order, int val)
	{
		return giveInt(order,val);
	}
	public int stringorderBlocking(int order, String val)
	{
		return giveStringBlocking(order, val, val.length());
	}
	public int intorderBlocking(int order, int val)
	{
		return giveIntBlocking(order,val);
	}
	
	private native int giveString(int request, String val, int length);
	private native int giveInt(int request, int val);
	private native int giveStringBlocking(int request, String val, int length);
	private native int giveIntBlocking(int request, int val);
}
