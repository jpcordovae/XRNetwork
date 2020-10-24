using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

/*
void xrn_start_service_thread();
void xrn_stop_service_thread();
uint32_t xrn_connect(const char* host, const char* port, const char* login, const char* password);
void xrn_disconnect();
uint32_t xrn_set_on_connect_callback(on_connect_callback occ);
uint32_t xrn_set_on_disconect_callback(on_disconnect_callback odc);
uint32_t xrn_set_on_new_message_callback(on_new_message_callback onmc);
uint32_t xrn_send_message(uint16_t header, const std::byte* buffer, uint32_t buffersize);
uint32_t xrn_set_on_room_callback(on_room_callback onmc);
uint64_t xrn_get_service_id();
uint64_t xrn_get_participant_id();
uint32_t xrn_is_connected();
 */

public class XRNetworkClient
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void OnConnectCallbackDelegate();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void OnDisconnectCallbackDelegate();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void OnNewMessageCallbackDelegate(UInt16 head, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] byte[] buffer, UInt32 buffersize);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void OnRoomCallbackDelegate();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void xrn_start_service_thread();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void xrn_stop_service_thread();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt32 xrn_connect([MarshalAs(UnmanagedType.LPUTF8Str)] string host,
                                            [MarshalAs(UnmanagedType.LPUTF8Str)] string port,
                                            [MarshalAs(UnmanagedType.LPUTF8Str)] string login,
                                            [MarshalAs(UnmanagedType.LPUTF8Str)] string password);
    
    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void xrn_disconnect();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt32 xrn_set_on_connect_callback(OnConnectCallbackDelegate occ);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt32 xrn_set_on_disconect_callback(OnDisconnectCallbackDelegate odc);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt32 xrn_set_on_new_message_callback(OnNewMessageCallbackDelegate omc);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt32 xrn_set_on_room_callback(OnRoomCallbackDelegate orc);

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt64 xrn_get_service_id();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt64 xrn_get_participant_id();

    [DllImport("XRNetworkClient.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern UInt64 xrn_send_message(UInt16 head,
                                                 byte[] buffer, 
                                                 UInt32 buffersize);

}
