using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Text;
using System.Runtime.InteropServices;

public class XRNetworkAPI
{
    private XRNetworkAPI() { }

    public enum MESSAGE_HEAD : Int16 { HEAD_PARTICIPANT_INFO, HEAD_AVATAR_MESSAGE, HEAD_CHAT_MESSAGE, HEAD_EVENT };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_RAW_MESSAGE
    {
        public MESSAGE_HEAD head;
        public UInt32 buffersize;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024 * 64 - 2 - 4 )] public byte[] buffer;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_PARTICIPANT_INFO
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)] public string name;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 512)] public string ip;
        public UInt64 id;
        public UInt64 server_id;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_PARTICIPANT_MESSAGE
    {
        UInt64 origin_timestamp;
        UInt64 reception_timestamp;
        Int64 position_x, position_y, position_z;
        Int64 rot_w, rot_i, rot_j, rot_k;
        UInt16 text_message_size; // in UTF-8
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10124 * 64 - 8 * 6)] public string text_message;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_MESSAGE
    {
        UInt64 participant_id;
        UInt64 timestamp; // in ms since epoch
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 64)]
        char[] message;
        UInt16 size;
    };


}
