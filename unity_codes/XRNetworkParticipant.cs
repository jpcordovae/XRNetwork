using System;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

public class XRNetworkParticipant : MonoBehaviour
{
    [SerializeField] private UInt64 m_id;
    [SerializeField] private string m_name;
    [SerializeField] private string m_host;
    [SerializeField] private uint m_port;
    [SerializeField] private XRNetworkProtocol.ST_PARTICIPANT_INFO m_info = new XRNetworkProtocol.ST_PARTICIPANT_INFO();
    [SerializeField] public XRNetworkProtocol.ST_PARTICIPANT_INFO info { get { return m_info; } set { } }
    private XRAsyncTCPClient m_tcpClient;// = new XRAsyncTCPClient();
    XRNetworkProtocol.ST_PARTICIPANT_MESSAGE message = new XRNetworkProtocol.ST_PARTICIPANT_MESSAGE();

    public UInt64 m_service_id;
    public UInt64 m_participant_id;
    public UInt64 m_service_timestamp;
    public string m_service_name;

    // callbacks 
    private void OnConnect() 
    {
        Debug.Log("OnConnect triggered, waiting for hello message");
        m_tcpClient.AsyncReceive();
    }

    private void OnDisconnect()
    {
        Debug.Log("OnDisconnect triggered");
    }

    public UInt64 id {
        get { return m_id; }
        set { m_id = value; }
    }

    public string participant_name {
        get { return m_name; }
        set { m_name = value; }
    }

    public string host {
        get { return m_host; }
        set { m_host = value; }
    }

    public int port {
        get { return (int) m_port; }
        set { m_port = (uint) value; }
    }

    void OnReceive(byte[] buffer, int buffersize)
    {
        Debug.Log("OnReceive: " + buffer.Length.ToString());
        UInt16 h = (UInt16)BitConverter.ToUInt16(buffer, 0);
        UInt32 bs = (UInt32)BitConverter.ToUInt32(buffer, 2);
        UInt16 origin_is_big_endian = (UInt16)BitConverter.ToUInt16(buffer, 6);
        UInt16 h2 = h;
        UInt32 bs2 = bs;

        if (BitConverter.IsLittleEndian)
        {
            h2 = (UInt16)IPAddress.NetworkToHostOrder((Int16)h);
            bs2 = (UInt32)IPAddress.NetworkToHostOrder((Int32)bs);
        }
        
        //Debug.Log("h2: " + h2.ToString("X") + ", bs2: " + bs2.ToString("X"));

        byte[] payload = new byte[bs2];

        Buffer.BlockCopy(buffer, 8, payload, 0, (Int32)bs2);
        /*if (origin_is_big_endian == 0xFFFF)
        {
            Array.Reverse(payload,0,payload.Length);
        }*/
        //string tmp = BitConverter.ToString(buffer);
        string dbgText = "RAW_MESSAGE.";
        switch ((XRNetworkProtocol.EN_RAW_MESSAGE_HEAD)h2)
        {
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_HELLO:
                dbgText += "HADNSHAKE_HELLO";
                XRNetworkProtocol.ST_HANDSHAKE_HELLO st_hello = new XRNetworkProtocol.ST_HANDSHAKE_HELLO();
                st_hello = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_HANDSHAKE_HELLO>(payload);
                if (origin_is_big_endian == 0x0000) // 0x0000 = little endian (Intel), 0xFFFF = Big Endian (network and others)
                {
                    /*st_hello.service_id = (UInt64)IPAddress.NetworkToHostOrder((Int64)st_hello.service_id);
                    st_hello.participant_id = (UInt64)IPAddress.NetworkToHostOrder((Int64)st_hello.participant_id);
                    st_hello.service_timestamp = (UInt64)IPAddress.NetworkToHostOrder((Int64)st_hello.service_timestamp);
                    st_hello.service_name_buffersize = (UInt32)IPAddress.NetworkToHostOrder((Int32)st_hello.service_name_buffersize);
                    */
                    // TODO: check if we have to swap the buffer too (I think yes doh)
                }
                else { 
                
                }
                m_service_name = Encoding.UTF8.GetString(st_hello.service_name_buffer);
                dbgText += "\n service_id: " + st_hello.service_id.ToString("X") +
                            "\n participant_id: " + st_hello.participant_id.ToString("X") +
                            "\n service_timestamp: " + st_hello.service_timestamp.ToString("X") +
                            "\n srvs_name_buffersize: " + st_hello.service_name_buffersize.ToString("X") +
                            "\n service_name: " + m_service_name;
                Debug.Log(dbgText);
                XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK hello_ack = new XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK();
                hello_ack.client_timestamp = m_tcpClient.UTCTiemstampSinceEpoch();
                hello_ack.participant_id = m_participant_id;
                string part_name = "participant name";
                hello_ack.participant_name = Encoding.UTF8.GetBytes(part_name);
                hello_ack.participant_name_buffersize = (UInt16)hello_ack.participant_name.Length;
                byte[] byte_hello_ack = XRNetworkProtocol.StructureToByteArray(hello_ack);// 1044 is the ST_HANDSHAKE_HELLO_ACK size
                //Debug.Log("byte_hello_ack lenght: " + byte_hello_ack.Length.ToString());
                // header for the new raw message
                XRNetworkProtocol.ST_XR_MESSAGE_HEADER new_header;
                new_header.head = (UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_HELLO_ACK;
                new_header.buffersize = (UInt32)byte_hello_ack.Length; // sizeof ST_HANDHSAKE_HELLO_ACK
                if (BitConverter.IsLittleEndian)
                {
                    new_header.payload_is_big_endian = 0x0000;
                }
                else {
                    new_header.payload_is_big_endian = 0xFFFF;
                }
                XRNetworkProtocol.ST_RAW_MESSAGE ret_raw_message = new XRNetworkProtocol.ST_RAW_MESSAGE();
                ret_raw_message.header = new_header;
                ret_raw_message.buffer = new byte[byte_hello_ack.Length];
                Array.Copy(byte_hello_ack,ret_raw_message.buffer,byte_hello_ack.Length);
                byte[] byte_ret_raw_message = XRNetworkProtocol.StructureToByteArray(ret_raw_message,XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE+byte_hello_ack.Length);
                Debug.Log("byte_ret_raw_message lenght: " + byte_ret_raw_message.Length.ToString());
                m_tcpClient.AsyncSend(byte_ret_raw_message,byte_ret_raw_message.Length);
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_CREDENTIALS:
                dbgText += "HANDSHAKE_CREDENTIALS";
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_PARTICIPANT_UPDATE:
                dbgText += "HANDSHAKE_PARTICIPANT_UPDATE";
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_INFO_REQUEST:
                dbgText += "PARTICIPANT_INFO_REQUEST";
                break;
            default:
                break;
        }
        //Debug.Log(tmp);
    }

    void OnSend()
    {
        Debug.Log("OnSend triggered");
    }

    void OnAwake()
    {
        m_port = 1080;
        m_host = "139.162.56.62";
    }

    // Start is called before the first frame update
    void Start()
    {
        m_tcpClient = new XRAsyncTCPClient();
        RegisterCallbacks();
        if (m_tcpClient == null)
        {
            Debug.LogError("m_tcpClient is null");
        }
        else
        {
            m_tcpClient.Connect(m_host, (int)m_port);
        }
    }

    void OnDestroy()
    {
        m_tcpClient.Disconnect();
    }

    // Update is called once per frame
    void Update()
    {
        if (m_tcpClient==null) return;
        if (!m_tcpClient.IsConnected)
        { 
            
        }

    }

    private void SetID(UInt64 _id)
    {
        m_id = _id;
    }

    private void RegisterCallbacks()
    {
        m_tcpClient.RegisterCallbackOnConnect(OnConnect);
        m_tcpClient.RegisterCallbackOnDisconnect(OnDisconnect);
        m_tcpClient.RegisterCallbackOnReceive(OnReceive);
        m_tcpClient.RegisterCallbackOnSend(OnSend);
        m_tcpClient.RegisterCallbackOnError(OnError);
    }

    void OnError(string msg)
    {
        Debug.LogError(msg);
    }

}
