using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq.Expressions;
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
        PARTICIPANT_LEAVE,
        PARTICIPANT_LEAVE_ACK,
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

    public static int ST_XR_MESSAGE_HEADER_SIZE = 8; // size of the structure in bytes

    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size=8*/)]
    public struct ST_XR_MESSAGE_HEADER
    {
        public UInt16 head;
        public UInt32 buffersize;
        public UInt16 payload_is_big_endian; // 0xFFFF for big endian and 0x0000 for little endian

        public void Setup(UInt16 _head)
        {
            head = _head;
            //buffersize = _buffersize;
            //payload_is_big_endian = _payload_is_big_endian;
        }

        public byte[] ToArray()
        {
            var stream = new MemoryStream();
            var writer = new BinaryWriter(stream);

            writer.Write(this.head);
            writer.Write(this.buffersize);
            writer.Write(this.payload_is_big_endian);

            return stream.ToArray();
        }

        public static ST_XR_MESSAGE_HEADER FromArray(byte[] bytes)
        {
            var reader = new BinaryReader(new MemoryStream(bytes));

            var s = default(ST_XR_MESSAGE_HEADER);

            s.head = reader.ReadUInt16();
            s.buffersize = reader.ReadUInt32();
            s.payload_is_big_endian = reader.ReadUInt16();

            return s;
        }
    }


    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024*64*/)]
    public struct ST_RAW_MESSAGE
    {
        public ST_XR_MESSAGE_HEADER header;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024*64 - 8)] public byte[] buffer;

        public void Setup(UInt16 _head, byte[] _payload)
        {
            header.head = _head;
            header.buffersize = (UInt32)_payload.Length;
        }

        public byte[] ToArray()
        {
            var stream = new MemoryStream();
            var writer = new BinaryWriter(stream);

            writer.Write(header.ToArray());
            writer.Write(buffer);

            return stream.ToArray();
        }

        public static ST_RAW_MESSAGE FromArray(byte[] _payload)
        {
            var reader = new BinaryReader(new MemoryStream(_payload));

            var s = default(ST_RAW_MESSAGE);

            s.header = ST_XR_MESSAGE_HEADER.FromArray(reader.ReadBytes(8));
            s.buffer = reader.ReadBytes(1024 * 64 - 8);

            return s;
        }
    }

    /*ST_RAW_MESSAGE build_raw_message(EN_RAW_MESSAGE_HEAD header, byte[] buffer, UInt32 buffersize)
    {
        ST_RAW_MESSAGE msg = new ST_RAW_MESSAGE();
        msg.header.head = (UInt16) header;
        msg.header.buffersize = buffersize;
        Buffer.BlockCopy(buffer, 0, msg.buffer, 0, (int)buffersize);
        return msg;
    }*/

    public static int ST_RAW_MESSAGE_SIZE = 65536;
    public static int ST_RAW_MESSAGE_PAYLOAD_SIZE = 65528;

    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size =8*/)]
    public struct ST_PARTICIPANT_LEAVE
    {
        public UInt64 participant_id; 
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size =8*/)]
    public struct ST_PARTICIPANT_LEAVE_ACK
    {
        public UInt64 participant_id;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1 /*, Size =1024*20+8+2*/)]
    public struct ST_PARTICIPANT_NEW
    {
        public UInt64 participant_id;
        public UInt32 descriptor_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 20)] public byte[] descriptor_buffer;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1),Serializable]
    public struct ST_PARTICIPANT_NEW_ACK
    {
        UInt64 participant_id;
    };

    /// <sumary>
    /// PARTICIPANT_JOIN
    /// </sumary>
    public static int ST_PARTICIPANT_JOIN_SIZE = 1024 * 10 + 4 + 1 + 1 + 8;
    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024 * 10 + 4 + 1 + 1 + 8*/)]
    public struct ST_PARTICIPANT_JOIN
    {
        public UInt64 participant_id;
        public UInt16 max_data_rate; // max message frequency [messages/s] supported by the service
        public UInt16 allow_asynchronous_messages; // polling or interrupt data
        public UInt32 message_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 10)] public byte[] message_buffer;
    }

    /// <sumary>
    /// PARTICIPANT_JOIN_ACK
    /// </sumary>
    public static int ST_PARTICIPANT_JOIN_ACK_SIZE = 1024 * 20 + 8 + 4;
    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024 * 20 + 8 + 2*/)]
    public struct ST_PARTICIPANT_JOIN_ACK
    {
        public UInt64 participant_id;
        public UInt32 json_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 20)] public byte[] json_buffer;
    }

    /// <summary>
    /// HADNSHAKE HELLO
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 8 + 8 + 8 + 4 + 1024*/)]
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
    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 8 + 8 + 4 + 1024 * 20*/)]
    public struct ST_HANDSHAKE_HELLO_ACK
    {
        public UInt64 participant_id;
        public UInt64 client_timestamp;
        //public UInt16 participant_name_buffersize;
        //[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024)] public byte[] participant_name;
        public UInt32 configuration_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 20)] public byte[] participant_buffer;
    };

    /// <summary>
    ///  HADNSHAKE CREDENTIALS
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 4 + 1024 * 20*/)]
    public struct ST_HANDSHAKE_CREDENTIALS
    {
        public UInt32 server_certificate_buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 20 * 1024)] public byte[] server_certificate_buffer;
    };

    /// <summary>
    /// HADNSHAKE CREDENTIALS ACK
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 8 + 2 + 1024 + 2 + 1024 + 2 + 1024 * 20*/)]
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
    public static int ST_PARTICIPANT_UPDATE_SIZE = 1024 * 20 + 4 + 8 + 8 + 8;
    public static int ST_PARTICIPANT_UPDATE_PAYLOAD_SIZE = 1024 * 20;
    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024 * 20 + 4 + 8 + 8 + 8*/)]
    public struct ST_PARTICIPANT_UPDATE
    {
        public UInt64 participant_id;
        public UInt64 reception_timestamp;
        public UInt64 deliver_timestamp;
        public UInt32 buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 20)] public byte[] buffer;
    };

    /// <summary>
    /// PARTICIPANT_UPDATE_ACK
    /// </summary>
    
    public static int ST_PARTICIPANT_UPDATE_ACK_LENGTH = 20500;
    public static int ST_PARTICIPANT_UPDATE_ACK_PAYLOAD_LENGTH = 20480;

    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024*20+ 4 + 8 + 8*/)]
    public struct ST_PARTICIPANT_UPDATE_ACK
    {
        public UInt64 participant_id;
        public UInt64 origin_timestamp;
        public UInt32 buffersize;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 20)] public byte[] buffer;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024 + 512 + 8 + 8*/)]
    public struct ST_PARTICIPANT_INFO
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024)] public string name;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 512)] public string ip;
        public UInt64 id;
        public UInt64 server_id;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024 * 60 +2+8+8+8+8*/)]
    public struct ST_PARTICIPANT_MESSAGE
    {
        public UInt64 origin_timestamp;
        public UInt64 reception_timestamp;
        public Int64 position_x, position_y, position_z;
        public Int64 rot_w, rot_i, rot_j, rot_k;
        public UInt16 text_message_size; // in UTF-8
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024 * 60)] public string text_message;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1/*, Size = 1024*60 + 8 + 8 + 2*/)]
    public struct ST_MESSAGE
    {
        public UInt64 participant_id;
        public UInt64 timestamp; // in ms since epoch
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1024 * 60)] public char[] message;
        public UInt16 size;
    };

    public static ST_RAW_MESSAGE BuildRawMessage(UInt16 head, byte[] payload, UInt32 payload_buffersize)
    {
        ST_RAW_MESSAGE raw_message = new ST_RAW_MESSAGE();
        raw_message.header.head = head;
        raw_message.header.payload_is_big_endian = 0x0000;
        raw_message.header.buffersize = payload_buffersize;
        raw_message.buffer = new byte[ST_RAW_MESSAGE_PAYLOAD_SIZE];
        Buffer.BlockCopy(payload,0,raw_message.buffer,0,(int)payload_buffersize);
        return raw_message;
    }

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
        if (size == 0 || str == null) Console.WriteLine("PROBLEMA AQUI !!!!");
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
        Buffer.BlockCopy(arr1, 0, tmp, 0, arr1.Length);
        Buffer.BlockCopy(arr2, 0, tmp, arr1.Length, arr2.Length);
        return tmp;
    }

    public static byte[] Serialize<T>(T s) where T : struct
    {
        var size = Marshal.SizeOf(typeof(T));
        var array = new byte[size];
        var ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(s, ptr, true);
        Marshal.Copy(ptr, array, 0, size);
        Marshal.FreeHGlobal(ptr);
        return array;
    }

    public static T Deserialize<T>(byte[] array) where T : struct
    {
        var size = Marshal.SizeOf(typeof(T));
        var ptr = Marshal.AllocHGlobal(size);
        Marshal.Copy(array, 0, ptr, size);
        var s = (T)Marshal.PtrToStructure(ptr, typeof(T));
        Marshal.FreeHGlobal(ptr);
        return s;
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
