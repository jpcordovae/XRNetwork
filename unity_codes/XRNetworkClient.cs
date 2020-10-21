using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

/*
void xrn_start_service_thread();
void xrn_stop_service_thread();
uint32_t xrn_connect(const char* host, const char* port, const char* login, const char* password);
uint32_t xrn_disconnect();
uint32_t xrn_set_on_connect_callback(on_connect_callback occ);
uint32_t xrn_set_on_disconect_callback(on_disconnect_callback odc);
uint32_t xrn_set_on_new_message_callback(on_new_message_callback onmc);
uint32_t xrn_send_message(uint16_t header, const std::byte* buffer, uint32_t buffersize);
uint32_t xrn_set_on_room_callback(on_room_callback onmc);
uint64_t xrn_get_service_id();
uint64_t xrn_get_participant_id();
typedef void(*on_new_message_callback)(char* buffer, uint32_t buffer_size);
 */

public class XRNetworkClient 
{
    public delegate UInt32 OnConnectCallbackDelegate();
    public delegate UInt32 OnDisconnectCallbackDelegate();
    public delegate UInt32 OnNewMessageCallbackDelegate(UInt16 head, byte[] buffer, UInt32 buffersize);
    public delegate UInt32 OnRoomCallbackDelegate();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void xrn_start_service_thread();
    //private static extern void XRNClientStart([MarshalAs(UnmanagedType.LPUTF8Str, SizeConst = 1024)] string name, UInt16 port);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void xrn_stop_service_thread();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt32 xrn_connect([MarshalAs(UnmanagedType.LPUTF8Str)] string host,
                                            [MarshalAs(UnmanagedType.LPUTF8Str)] string port,
                                            [MarshalAs(UnmanagedType.LPUTF8Str)] string login,
                                            [MarshalAs(UnmanagedType.LPUTF8Str)] string password);
    
    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt32 xrn_disconnect();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern UInt32 xrn_set_on_connect_callback(OnConnectCallbackDelegate occ);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern UInt32 xrn_set_on_disconect_callback(OnDisconnectCallbackDelegate odc);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern UInt32 xrn_set_on_new_message_callback(OnNewMessageCallbackDelegate omc);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern UInt32 xrn_set_on_room_callback(OnRoomCallbackDelegate orc);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern UInt64 xrn_get_service_id();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern UInt64 xrn_get_participant_id();


}
