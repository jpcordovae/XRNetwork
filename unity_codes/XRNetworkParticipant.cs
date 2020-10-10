using System;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using System.Collections;
using System.Collections.Concurrent;
using System.Threading;

public class XRNetworkParticipant : MonoBehaviour
{
    [SerializeField] private UInt64 m_id;
    [SerializeField] private string m_name;
    [SerializeField] private string m_host = "139.162.56.62";
    [SerializeField] private uint m_port=1080;
    [SerializeField] private XRNetworkProtocol.ST_PARTICIPANT_INFO m_info = new XRNetworkProtocol.ST_PARTICIPANT_INFO();
    [SerializeField] public XRNetworkProtocol.ST_PARTICIPANT_INFO info { get { return m_info; } set { } }
    private XRAsyncTCPClient m_tcpClient;// = new XRAsyncTCPClient();
    XRNetworkProtocol.ST_PARTICIPANT_MESSAGE message = new XRNetworkProtocol.ST_PARTICIPANT_MESSAGE();

    public UInt64 m_service_id;
    public UInt64 m_participant_id;
    public UInt64 m_service_timestamp;
    public string m_service_name;
    public string m_participant_name = "JP Participant";
    private static UInt32 m_tcp_buffersize; // actual buffer size
    private static byte[] m_tcp_buffer = new byte[] { }; // temporal tcp buffer (before transform to a message)
    private static UInt16 m_tcp_header; // header protocol
    private static UInt16 m_payload_big_endian;
    private string dbgText;
    private int m_dt_ms;
    private UInt16 m_max_data_rate;
    //
    public string m_login = "login";
    public string m_password = "password";
    // callbacks 
    // Threads
    private Thread ThreadParticipantUpdate;
    private ConcurrentQueue<string> m_concurrent_queue;
    // time management
    private long m_last_timestamp;
    public volatile bool can_update_to_the_server=false;
    //#region ParticipantThreads
    public void ParticipantUpdateFunction()
    {
        Debug.Log("Update");
        float px = transform.position.x;
        float py = transform.position.y;
        float pz = transform.position.z;
        float rx = transform.rotation.x;
        float ry = transform.rotation.y;
        float rz = transform.rotation.z;
        float rw = transform.rotation.w;

        float lpx = transform.localPosition.x;
        float lpy = transform.localPosition.y;
        float lpz = transform.localPosition.z;
        float lrx = transform.localRotation.x;
        float lry = transform.localRotation.y;
        float lrz = transform.localRotation.z;
        float lrw = transform.localRotation.w;

        string message =    "px:" + px.ToString() + ";" +
                            "py:" + py.ToString() + ";" +
                            "pz:" + pz.ToString() + ";" +
                            "rx:" + rx.ToString() + ";" +
                            "ry:" + ry.ToString() + ";" +
                            "rz:" + rz.ToString() + ";" +
                            "rw:" + rw.ToString() + ";" +
                            "lpx:" + lpx.ToString() + ";" +
                            "lpy:" + lpy.ToString() + ";" +
                            "lpz:" + lpz.ToString() + ";" +
                            "lrx:" + lrx.ToString() + ";" +
                            "lry:" + lry.ToString() + ";" +
                            "lrz:" + lrz.ToString() + ";" +
                            "lrw:" + lrw.ToString();

        XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK updt = new XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK();
        updt.origin_timestamp = m_tcpClient.UTCTiemstampSinceEpoch();

        XRNetworkProtocol.ST_RAW_MESSAGE raw = new XRNetworkProtocol.ST_RAW_MESSAGE();
        raw.header.head = (UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_UPDATE_ACK;
        byte[] payload = Encoding.UTF8.GetBytes(message);
        raw.buffer = new byte[1024 * 64 - 8];
        raw.header.buffersize = (UInt16)payload.Length;
        Array.Copy(payload, 0, raw.buffer, 0, raw.header.buffersize);
        byte[] raw_message = new byte[1024 * 64];
        raw_message = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_RAW_MESSAGE>(raw);
        //Debug.Log(BitConverter.ToString(raw_message));
        m_tcpClient.AsyncSend(raw_message, raw_message.Length);
    }
    //#endregion 
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

    void ProcessMessage(UInt16 header, UInt32 buffersize, byte[] payload)
    {
        //if (BitConverter.IsLittleEndian)
        //{
        //    m_tcp_header = (UInt16)IPAddress.NetworkToHostOrder((Int16)h);
        //    m_tcp_buffersize = (UInt32)IPAddress.NetworkToHostOrder((Int32)bs);
        //}

        /*if (origin_is_big_endian == 0xFFFF)
        {
            Array.Reverse(payload,0,payload.Length);
        }*/
        //string tmp = BitConverter.ToString(payload);
        
        switch ((XRNetworkProtocol.EN_RAW_MESSAGE_HEAD)header)
        {
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_HELLO:
                //dbgText += "HANDSHAKE_HELLO";
                XRNetworkProtocol.ST_HANDSHAKE_HELLO st_hello = new XRNetworkProtocol.ST_HANDSHAKE_HELLO();
                st_hello.service_name_buffer = new byte[1024];
                Array.Clear(st_hello.service_name_buffer, 0, 1024);
                st_hello = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_HANDSHAKE_HELLO>(payload);
                
                //if (origin_is_big_endian == 0x0000) // 0x0000 = little endian (Intel), 0xFFFF = Big Endian (network and others)
                //{
                //st_hello.service_id = (UInt64)IPAddress.NetworkToHostOrder((Int64)st_hello.service_id);
                //st_hello.participant_id = (UInt64)IPAddress.NetworkToHostOrder((Int64)st_hello.participant_id);
                //st_hello.service_timestamp = (UInt64)IPAddress.NetworkToHostOrder((Int64)st_hello.service_timestamp);
                //st_hello.service_name_buffersize = (UInt32)IPAddress.NetworkToHostOrder((Int32)st_hello.service_name_buffersize);

                // save data 
                m_service_name = Encoding.UTF8.GetString(st_hello.service_name_buffer);
                m_service_id = st_hello.service_id;
                m_participant_id = st_hello.participant_id;
                m_service_timestamp = st_hello.service_timestamp;

                dbgText += "\n service_id: " + m_service_id.ToString("X") + //st_hello.service_id.ToString("X") +
                            "\n participant_id: " + m_participant_id.ToString("X") + // st_hello.participant_id.ToString("X") +
                            "\n service_timestamp: " + m_service_timestamp.ToString("X") + //st_hello.service_timestamp.ToString("X") +
                            "\n srvs_name_buffersize: " + st_hello.service_name_buffersize.ToString("X") +
                            "\n service_name: " + m_service_name;
                //Debug.Log(dbgText);
                Debug.Log("ProcessMessage HANDSHAKE_HELLO: \n" + string.Format("header: {0}, buffersize: {1}\n{2}\npayload: {2}\n", header, buffersize, dbgText, BitConverter.ToString(payload)));
                dbgText = "";
                //Debug.Log(tmp); // raw buffer

                // instantiate hello_ack
                XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK hello_ack = new XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK();
                hello_ack.participant_id = m_participant_id;
                //Debug.Log("helo_ack.participant_id: " + m_participant_id.ToString("X"));
                hello_ack.client_timestamp = m_tcpClient.UTCTiemstampSinceEpoch();
                hello_ack.participant_name = new byte[1024]; // clean the new array Array.empty<byte>(size of the array)
                Array.Clear(hello_ack.participant_name, 0, 1024);
                byte[] pname = Encoding.UTF8.GetBytes(m_participant_name);
                Array.Copy(pname, hello_ack.participant_name, pname.Length);
                hello_ack.participant_name_buffersize = (UInt16)pname.Length;
                byte[] byte_hello_ack = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK>(hello_ack);// 1042 is the ST_HANDSHAKE_HELLO_ACK size
                //Debug.Log("byte_hello_ack (" + byte_hello_ack.Length.ToString() + ") :\n" + BitConverter.ToString(byte_hello_ack));  // CORRECT !!!

                // header for the new raw message
                XRNetworkProtocol.ST_XR_MESSAGE_HEADER new_header;
                new_header.head = (UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_HELLO_ACK;
                new_header.buffersize = (UInt32)byte_hello_ack.Length; // sizeof ST_HANDHSAKE_HELLO_ACK

                if (BitConverter.IsLittleEndian)
                {
                    new_header.payload_is_big_endian = 0x0000;
                }
                else
                {
                    new_header.payload_is_big_endian = 0xFFFF;
                }

                XRNetworkProtocol.ST_RAW_MESSAGE ret_raw_message = new XRNetworkProtocol.ST_RAW_MESSAGE();
                ret_raw_message.header = new_header;
                ret_raw_message.buffer = new byte[byte_hello_ack.Length];
                Array.Copy(byte_hello_ack, ret_raw_message.buffer, byte_hello_ack.Length);
                byte[] byte_ret_raw_message = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_RAW_MESSAGE>(ret_raw_message);
                //byte[] byte_ret_raw_message = XRNetworkProtocol.StructureToByteArray(ret_raw_message);
                //Debug.Log("byte_ret_raw_message lenght: " + byte_ret_raw_message.Length.ToString());
                m_tcpClient.AsyncSend(byte_ret_raw_message, byte_ret_raw_message.Length);
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_CREDENTIALS:
                //dbgText += "HANDSHAKE_CREDENTIALS";
                XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS hcred = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS>(payload);
                dbgText +=  "\ncertificate_buffersize: " + hcred.server_certificate_buffersize.ToString() +
                            "\ncertificate: " + BitConverter.ToString(hcred.server_certificate_buffer);
                //Debug.Log(dbgText);
                Debug.Log("ProcessMessage HANDSHAKE_CREDENTIALS: \n" + string.Format("header: {0}, buffersize: {1}\n{2}\npayload: {2}\n", header, buffersize, dbgText, BitConverter.ToString(payload)));
                dbgText = "";
                // HADNSHAKE CREDENTIALS ACK
                XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS_ACK ack = new XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS_ACK();
                ack.login_buffer = new byte[1024];
                ack.password_buffer = new byte[1024];
                ack.participant_certificate_buffer = new byte[20*1024];

                ack.login_buffersize = (UInt16)m_login.Length;
                byte[] tmp1 = new byte[1024];
                tmp1 = Encoding.UTF8.GetBytes(m_login);
                Array.Copy(tmp1, 0, ack.login_buffer, 0, tmp1.Length);
                ack.password_buffersize = (UInt16)m_password.Length;
                tmp1 = Encoding.UTF8.GetBytes(m_password);
                Array.Copy(tmp1, 0, ack.password_buffer, 0, tmp1.Length);
                ack.participant_id = m_participant_id;
                //ack.participant_certificate_buffer
                //ack.participant_certificate_buffersize

                byte[] ack_byte = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS_ACK>(ack);

                XRNetworkProtocol.ST_RAW_MESSAGE ret_raw_message_2 = new XRNetworkProtocol.ST_RAW_MESSAGE();
                ret_raw_message_2.header.head =(UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_CREDENTIALS_ACK;
                ret_raw_message_2.header.buffersize = (UInt32)ack_byte.Length;
                ret_raw_message_2.buffer = new byte[1024 * 64 - 8];
                Array.Copy(ack_byte,0, ret_raw_message_2.buffer,0,ack_byte.Length);
                byte[] ret_tmp = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_RAW_MESSAGE>(ret_raw_message_2);
                m_tcpClient.AsyncSend(ret_tmp,ret_tmp.Length);
                
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_PARTICIPANT_UPDATE:
                dbgText += "HANDSHAKE_PARTICIPANT_UPDATE";
                Debug.Log(dbgText);
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_INFO_REQUEST:
                dbgText += "PARTICIPANT_INFO_REQUEST";
                Debug.Log(dbgText);
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_JOIN:
                dbgText += "PARTICIPANT_JOIN\n";
                XRNetworkProtocol.ST_PARTICIPANT_JOIN join = new XRNetworkProtocol.ST_PARTICIPANT_JOIN();
                join.message_buffer = new byte[1024];
                Array.Clear(join.message_buffer, 0, 1024);
                join = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_JOIN>(payload);
                dbgText += "participant_id: " + join.participant_id.ToString("X") +
                            "\nmax_data_rate: " + join.max_data_rate.ToString("X") +
                            "\nmessage_buffersize: " + join.message_buffersize.ToString() + 
                            "\nmessage_buffer: " + Encoding.UTF8.GetString(join.message_buffer) + "\n";
                m_participant_id = join.participant_id;
                m_max_data_rate = join.max_data_rate;
                m_dt_ms = (int)(1 / (int)m_max_data_rate);
                Debug.Log(dbgText);
                can_update_to_the_server = true;
                //ThreadParticipantUpdate = new Thread(ParticipantUpdateFunction);
                dbgText = "";
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_NEW:
                dbgText += "PARTICIPANT_NEW\n";
                XRNetworkProtocol.ST_PARTICIPANT_UPDATE upd = new XRNetworkProtocol.ST_PARTICIPANT_UPDATE();
                UInt64 new_pid = upd.participant_id;
                Debug.Log(dbgText);
                dbgText = "";
                break;
            default:
                break;
        }
        dbgText = "";
    }

    void OnReceive(byte[] buffer, int buffersize)
    {
        //if (buffersize == 0) return;

        if (m_tcp_buffer.Length != 0)
            m_tcp_buffer = XRNetworkProtocol.ConcatByteArrays(m_tcp_buffer, buffer);
        else
            m_tcp_buffer = buffer;

        // check minimum size
        //if (m_tcp_buffer.Length < XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE) return;

        //get the headers
        m_tcp_header = BitConverter.ToUInt16(m_tcp_buffer, 0);
        m_tcp_buffersize = BitConverter.ToUInt32(m_tcp_buffer, 2);
        m_payload_big_endian = BitConverter.ToUInt16(m_tcp_buffer, 6);

        /*Debug.Log(  string.Format("OnReceive {0} bytes: \n",buffersize) + 
                    buffer.Length.ToString() + 
                    string.Format("\nm_tcp_header: {0}\nm_tcp_buffersize: {1}\n",m_tcp_header,m_tcp_buffersize) +
                    BitConverter.ToString(buffer));*/

        // check the size of the payload be as shuould be
        if (m_tcp_buffer.Length >= m_tcp_buffersize + XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE)
        {
            if (m_tcp_header == 0 /*|| m_tcp_buffersize < 1024*/) return;
            byte[] payload = XRAsyncTCPClient.SplitByteArray(m_tcp_buffer, XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE, (int)m_tcp_buffersize);
            ProcessMessage(m_tcp_header, m_tcp_buffersize, payload);
            m_tcp_buffer = Array.Empty<byte>();
        }
    }

    void OnSend()
    {
        //Debug.Log("OnSend triggered");
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
            return;
        }
        
        long timestamp = DateTimeOffset.Now.ToUnixTimeMilliseconds();
        if ( (m_dt_ms>0) && (timestamp - m_last_timestamp >= m_dt_ms ) && can_update_to_the_server )
        {
            ParticipantUpdateFunction();
            m_last_timestamp = timestamp;
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
