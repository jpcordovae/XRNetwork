using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class XRNetworkProtocol
{
    /// <summary>
    ///  RAW MESSAGE
    /// </summary>

    public enum EN_RAW_MESSAGE_HEAD : UInt16
    {
        NONE,
        HANDSHAKE_HELLO,
        HANDSHAKE_HELLO_ACK,
        HANDSHAKE_CREDENTIALS,
        HANDSHAKE_CREDENTIALS_ACK,
        HANDSHAKE_STATISTICS_REQUEST,
        HANDSHAKE_STATISTICS_REQUEST_ACK,
        HANDSHAKE_PARTICIPANT_UPDATE,
        HANDSHAKE_PARTICIPANT_UPDATE_ACK,
        PARTICIPANT_JOIN,
        PARTICIPANT_JOIN_ACK,
        MESSAGE,
        MESSAGE_ACK,
        PARTICIPANT_NEW,
        PARTICIPANT_NEW_ACK,
        PARTICIPANT_DELETE,
        PARTICIPANT_DELETE_ACK,
        PARTICIPANT_INFO_REQUEST,
        PARTICIPANT_INFO_REQUEST_ACK,
        PARTICIPANT_UPDATE,
        PARTICIPANT_UPDATE_ACK,
        PARTICIPANT_EVENT,
        PARTICIPANT_EVENT_ACK,
        AVATAR_MESSAGE,
        CHAT_MESSAGE,
        CONTROL
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_XR_MESSAGE_HEADER
    {
        public UInt16 head;
        public UInt32 buffersize;
        public UInt16 payload_is_big_endian; // 0xFFFF for big endian and 0x0000 for little endian
    }

    public static int ST_XR_MESSAGE_HEADER_SIZE = 8; // size of the structure in bytes

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_RAW_MESSAGE
    {
        public ST_XR_MESSAGE_HEADER header;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024*64 - 8)] public byte[] buffer;
    }

    /*ST_RAW_MESSAGE build_raw_message(EN_RAW_MESSAGE_HEAD header, byte[] buffer, UInt32 buffersize)
    {
        ST_RAW_MESSAGE msg = new ST_RAW_MESSAGE();
        msg.header.head = (UInt16) header;
        msg.header.buffersize = buffersize;
        Buffer.BlockCopy(buffer, 0, msg.buffer, 0, (int)buffersize);
        return msg;
    }*/

    /// <sumary>
    /// PARTICIPANT_JOIN
    /// </sumary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_PARTICIPANT_JOIN
    {
        public UInt64 participant_id;
        public UInt16 max_data_rate; // max message frequency [messages/s] supported by the service
        public UInt16 allow_asynchronous_messages; // polling or interrupt data
        public UInt16 message_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 10)] public byte[] message_buffer;
    }

    /// <sumary>
    /// PARTICIPANT_JOIN_ACK
    /// </sumary>



    /// <summary>
    /// HADNSHAKE HELLO
    /// </summary>

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_HANDSHAKE_HELLO
    {
        public UInt64 service_id;
        public UInt64 participant_id;
        public UInt64 service_timestamp;
        public UInt32 service_name_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024)] public byte[] service_name_buffer;
    };

    /// <summary>
    /// HADNSHAKE HELLO ACK (1042 bytes - 0x0412 hexa)
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_HANDSHAKE_HELLO_ACK
    {
        public UInt64 participant_id;
        public UInt64 client_timestamp;
        public UInt16 participant_name_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024)] public byte[] participant_name;
    };

    /// <summary>
    ///  HADNSHAKE CREDENTIALS
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_HANDSHAKE_CREDENTIALS
    {
        public UInt32 server_certificate_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 20 * 1024)] public byte[] server_certificate_buffer;
    };

    /// <summary>
    /// HADNSHAKE CREDENTIALS ACK
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_HANDSHAKE_CREDENTIALS_ACK
    {
        public UInt64 participant_id;
        public UInt16 login_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024)] public byte[] login_buffer;
        public UInt16 password_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024)] public byte[] password_buffer;
        public UInt16 participant_certificate_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024*20)] public byte[] participant_certificate_buffer;
    };

    /// <summary>
    /// PARTICIPANT UPDATE
    /// </summary>

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_PARTICIPANT_UPDATE
    {
        public UInt64 participant_id;
        public UInt64 reception_timestamp;
        public UInt64 deliver_timestamp;
        public UInt16 buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 20)] public byte[] buffer;
    };

    /// <summary>
    /// PARTICIPANT_UPDATE_ACK
    /// </summary>

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_PARTICIPANT_UPDATE_ACK
    {
        public UInt64 origin_timestamp;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_PARTICIPANT_INFO
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024)] public string name;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 512)] public string ip;
        public UInt64 id;
        public UInt64 server_id;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_PARTICIPANT_MESSAGE
    {
        public UInt64 origin_timestamp;
        public UInt64 reception_timestamp;
        public Int64 position_x, position_y, position_z;
        public Int64 rot_w, rot_i, rot_j, rot_k;
        public UInt16 text_message_size; // in UTF-8
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10124 * 64 - 8 * 6)] public string text_message;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_MESSAGE
    {
        public UInt64 participant_id;
        public UInt64 timestamp; // in ms since epoch
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 64)] public char[] message;
        public UInt16 size;
    };

    public static object GetObjectFromBytes(byte[] buffer, int buffersize, Type objType)
    {
        object obj = null;
        if ( (buffer!=null) && (buffer.Length>0) ) 
        {
            IntPtr ptrObj = IntPtr.Zero;
            
            try 
            {
                //int objSize = Marshal.SizeOf(objType);
                int objSize = buffersize;
                if (buffer.Length < objSize)
                    throw new Exception(String.Format("Buffer smaller than needed for creation of object of type {0}", objType));

                ptrObj = Marshal.AllocHGlobal(objSize);
                if (ptrObj != IntPtr.Zero)
                {
                    Marshal.Copy(buffer, 0, ptrObj, objSize);
                    obj = Marshal.PtrToStructure(ptrObj, objType);
                }
                else {
                    throw new Exception(String.Format("Couldn't allocate memory to create object of type {0}", objType));
                }
            } 
            finally 
            {
                if (ptrObj != IntPtr.Zero)
                    Marshal.FreeHGlobal(ptrObj);
            }
        }
        return obj;
    }

    public static byte[] GetBytes<T>(T str, int size)
    {
        //int size = Marshal.SizeOf(str);
        if (size == 0) Console.WriteLine("PROBLEMA AQUI !!!!");
        byte[] arr = new byte[size];
        GCHandle h = default(GCHandle);
        try
        {
            h = GCHandle.Alloc(arr, GCHandleType.Pinned);
            Marshal.StructureToPtr<T>(str, h.AddrOfPinnedObject(), false);
        }
        finally
        {
            if (h.IsAllocated)
            {
                h.Free();
            }
        }
        return arr;
    }

    public static byte[] GetBytes<T>(T str)
    {
        int size = Marshal.SizeOf(str);
        return GetBytes<T>(str,size);
    }

    public static T ByteArrayToStructure<T>(byte[] bytes) where T : struct 
    {
        var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
        var result = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
        handle.Free();
        return result;
    }

    public static byte[] StructureToByteArray(object strc)
    {
        int size = Marshal.SizeOf(strc);
        byte[] arr = new byte[size];
        IntPtr ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(strc, ptr, false);
        Marshal.Copy(ptr, arr, 0, size);
        Marshal.FreeHGlobal(ptr);
        return arr;
    }

    public static byte[] StructureToByteArray(object strc, int object_size)
    {
        int size = object_size;
        byte[] arr = new byte[size];
        IntPtr ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(strc, ptr, false);
        Marshal.Copy(ptr, arr, 0, size);
        Marshal.FreeHGlobal(ptr);
        return arr;
    }

    public static byte[] ConcatByteArrays(byte[] arr1, byte[] arr2)
    {
        int lenght = arr1.Length + arr2.Length;
        if (lenght == 0) return Array.Empty<byte>();
        byte[] tmp = new byte[lenght];
        Array.Copy(arr1, tmp, arr1.Length);
        Array.Copy(arr2, 0, tmp, arr1.Length, arr2.Length);
        return tmp;
    }

    /*object ByteArrayToStructure(byte[] bytearray, object structureObj, int position)
    {
        int length = Marshal.SizeOf(structureObj);
        IntPtr ptr = Marshal.AllocHGlobal(length);
        Marshal.Copy(bytearray, 0, ptr, length);
        structureObj = Marshal.PtrToStructure(Marshal.UnsafeAddrOfPinnedArrayElement(bytearray, position), structureObj.GetType());
        Marshal.FreeHGlobal(ptr);
        return structureObj;
    }*/

}
