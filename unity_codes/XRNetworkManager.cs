using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime;
using System.Collections.Concurrent;
using UnityEngine;
using System.Text;
using Unity.Collections;
using UnityEngine.UI;
using UnityEditor;
using System.Runtime.CompilerServices;

public class XRNetworkManager : MonoBehaviour
{
    public GameObject m_local_participant_go;
    private XRNetworkManager() { }

    private static XRNetworkManager m_instance;
    private static object m_synLock = new object();

    public static Vector3 xr_reference_position;
    public static Quaternion xr_reference_rotation;

    public static XRNetworkManager Instance
    {
        get 
        {
            if (m_instance == null)
            {
                lock (m_synLock)
                {
                    if (m_instance == null)
                        m_instance = GameObject.Find("[XRNETWORK]").GetComponent<XRNetworkManager>();
                }
            }
            return m_instance;
        }
    }

    //public GameObject player_game_object;
    private string player_descriptor_json;

    // concurrent dictinary that maps id's users with messages from server, it will keep the last message only so far
    public string m_host; //"139.162.56.62";
    public uint m_port;
    [ReadOnly] public string m_player_name;
    [ReadOnly] public UInt64 m_service_id;
    [ReadOnly] private UInt64 m_participant_id_number;
    [ReadOnly] public string m_participant_id;
    [ReadOnly] public UInt64 m_service_timestamp;
    [ReadOnly] public string m_service_name;
    [ReadOnly] public UInt16 m_max_data_rate;
    public string m_login = "login";
    public string m_password = "password";
    static private bool b_on_room;

    public bool on_room {
        get { return b_on_room; }
    }

    public void SetHost(string _host)
    {
        // check if is a valid IP address
        m_host = _host;
    }

    public void SetPort(string _port)
    {
        // check if is a valid port value
        m_port = Convert.ToUInt16(_port);
    }

    //private ConcurrentDictionary<UInt64, byte[]> xrn_devices_messages = new ConcurrentDictionary<ulong, byte[]>();
    private ConcurrentQueue<XRNetworkProtocol.ST_RAW_MESSAGE> pending_updates = new ConcurrentQueue<XRNetworkProtocol.ST_RAW_MESSAGE>();
    private ConcurrentQueue<byte[]> to_tcp_service_queue = new ConcurrentQueue<byte[]>();

    public void QueueMessageToService(byte [] msg)
    {
        to_tcp_service_queue.Enqueue(msg);
    }

    GameObject GetGameObjectbyID(UInt64 ID)
    {
        foreach (Transform child in transform)
        {
            XRNetworkObject myGO = child.GetComponent<XRNetworkObject>();
            if ( myGO != null)
                if(myGO.m_id == ID)
                    return child.gameObject;
        }
        return null;
    }

    private static XRAsyncTCPClient m_tcpClient = XRAsyncTCPClient.GetInstance();
    private static UInt32 m_tcp_buffersize; // actual buffer size
    private static byte[] m_tcp_buffer = new byte[] { }; // temporal tcp buffer (before transform to a message)
    private static UInt16 m_tcp_head; // header protocol
    private static UInt16 m_payload_big_endian;
    private static string dbgText;
    private int m_dt_ms = 25; // ms 
    private long ms_last_timestamp;

    void Start()
    {
        //XRNetworkObject NO = ;
        GameObject.Find("LOCAL_PARTICIPANT").GetComponent<XRNetworkObject>().SetPlayerName(GameObject.Find("InputFieldName").GetComponent<InputField>().text);
        player_descriptor_json = Save<XRNetworkObject>(m_local_participant_go);
        ms_last_timestamp = 0;
        b_on_room = false;
    }

    // this doesn't run on the main thread, so... it sucks...
    private static void OnConnect()
    {
        //Debug.Log("OnConnect triggered, waiting for hello message");
        //GameObject.Find("ConnectButton").GetComponentInChildren<Text>().text = "Disconnect";
        m_tcpClient.AsyncReceive();
    }

    private static void OnDisconnect()
    {
        Debug.Log("OnDisconnect triggered");
        b_on_room = false;
    }

    public void OnReceive(byte[] buffer, int buffersize)
    {
        if (m_tcp_buffer.Length != 0)
            m_tcp_buffer = XRNetworkProtocol.ConcatByteArrays(m_tcp_buffer, buffer);
        else
            m_tcp_buffer = buffer;

        // check minimum size
        if (m_tcp_buffer.Length < XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE) return;

        //get the headers
        m_tcp_head = BitConverter.ToUInt16(m_tcp_buffer, 0);
        m_tcp_buffersize = BitConverter.ToUInt32(m_tcp_buffer, 2);
        m_payload_big_endian = BitConverter.ToUInt16(m_tcp_buffer, 6);

        /*Debug.Log(  string.Format("OnReceive {0} bytes: \n",buffersize) + 
                    buffer.Length.ToString() + 
                    string.Format("\nm_tcp_header: {0}\nm_tcp_buffersize: {1}\n",m_tcp_header,m_tcp_buffersize) +
                    BitConverter.ToString(buffer));*/

        // check the size of the payload be as shuould be
        if (m_tcp_buffer.Length >= m_tcp_buffersize + XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE)
        {
            if (m_tcp_head == 0 /*|| m_tcp_buffersize < 1024*/) return;

            byte[] payload = XRAsyncTCPClient.SplitByteArray(m_tcp_buffer, 
                                                             XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE, 
                                                             (int)m_tcp_buffersize);

            //ProcessMessage(m_tcp_header, m_tcp_buffersize, payload);
            XRNetworkProtocol.ST_RAW_MESSAGE rw = new XRNetworkProtocol.ST_RAW_MESSAGE();
            rw.header.head = m_tcp_head;
            rw.header.buffersize = m_tcp_buffersize;
            rw.header.payload_is_big_endian = 0;
            rw.buffer = payload;
            pending_updates.Enqueue(rw);

            if (m_tcp_buffer.Length > m_tcp_buffersize + XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE)
            {
                //Debug.Log("HERE");
                int total_length = (int)m_tcp_buffersize + XRNetworkProtocol.ST_XR_MESSAGE_HEADER_SIZE;
                // not sure if I can do this on a single call, if the parameter is copied, then yes, but not sure
                byte[] tmp = XRAsyncTCPClient.SplitByteArray(m_tcp_buffer,
                                                             total_length,
                                                             (int)m_tcp_buffer.Length - total_length);
                //m_tcp_buffer = Array.Empty<byte>();
                m_tcp_buffer = tmp;
            }

            m_tcp_buffer = Array.Empty<byte>();
        }
    }

    private static void OnSend()
    {
        //Debug.Log("OnSend triggered");
    }

    void OnDestroy()
    {
        if (m_tcpClient != null)
            if (m_tcpClient.IsConnected)
                m_tcpClient.Disconnect();
    }

    private static void OnError(string msg)
    {
        Debug.LogError(msg);
    }

    public void Connect()
    {
        UpdateVariablesFromCanvas();
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

    private void RegisterCallbacks()
    {
        m_tcpClient.RegisterCallbackOnConnect(OnConnect);
        m_tcpClient.RegisterCallbackOnDisconnect(OnDisconnect);
        m_tcpClient.RegisterCallbackOnReceive(OnReceive);
        m_tcpClient.RegisterCallbackOnSend(OnSend);
        m_tcpClient.RegisterCallbackOnError(OnError);
    }

    private static void RecursiveFindDevices(Transform parent, List<GameObject> list)
    {
        foreach (Transform child in parent)
        {
            if (child.gameObject.GetComponent<XRNetworkObject>()==null)
            {
                list.Add(child.gameObject);
            }
            RecursiveFindDevices(child, list);
        }
    }

    public void Update()
    {
        try
        {
            XRNetworkProtocol.ST_RAW_MESSAGE raw = new XRNetworkProtocol.ST_RAW_MESSAGE();
            //Debug.Log("in queue: " + pending_updates.Count.ToString() + ", out queue: " + to_tcp_service_queue.Count.ToString());
            /*int index = 0;*/
            while (pending_updates.TryDequeue(out raw) /*&& index<1000*/)
            {
                if (raw.buffer != null)
                    ProcessMessage(raw.header.head, raw.header.buffersize, raw.buffer);
            }

            if (b_on_room)
            {
                long ms_timestamp = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                if (ms_timestamp - ms_last_timestamp > m_dt_ms)
                {
                    byte[] barray = new byte[] { };
                    while (to_tcp_service_queue.TryDequeue(out barray))
                    {
                        XRNetworkProtocol.ST_RAW_MESSAGE msg = new XRNetworkProtocol.ST_RAW_MESSAGE();
                        msg.header.head = (UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_UPDATE_ACK;
                        msg.header.payload_is_big_endian = 0x0000;
                        msg.header.buffersize = (UInt32)barray.Length;
                        //Debug.Log("msg.header.buffersize: " + barray.Length.ToString());
                        msg.buffer = new byte[XRNetworkProtocol.ST_RAW_MESSAGE_PAYLOAD_SIZE];
                        Array.Clear(msg.buffer, 0, XRNetworkProtocol.ST_RAW_MESSAGE_PAYLOAD_SIZE);
                        Array.Copy(barray, msg.buffer, barray.Length);
                        Debug.Log(">> PARTICIPANT_UPDATE_ACK");
                        //Debug.Log(  "ms_timestamp: " + ms_timestamp.ToString() + ", ms_last_timestamp: " + ms_last_timestamp.ToString() + ", dt: " + (ms_timestamp - ms_last_timestamp).ToString() + "\nsize: " + msg.header.buffersize.ToString());
                        byte[] msgarray = XRNetworkProtocol.GetBytes(msg, XRNetworkProtocol.ST_RAW_MESSAGE_SIZE);
                        m_tcpClient.AsyncSend(msgarray, msgarray.Length);
                    }
                    ms_last_timestamp = ms_timestamp;
                }
            }
        }
        catch (Exception e)
        {
            Debug.Log("PLayer Exception: " + e.ToString());
        }
    }

    public void DeletePlayer()
    { 
        
    }

    public void UpdateVariablesFromCanvas()
    {
        m_player_name = GameObject.Find("LOCAL_PARTICIPANT").GetComponent<XRNetworkObject>().m_name;
        m_host = GameObject.Find("InputFieldHost").GetComponent<InputField>().text;
        m_port = Convert.ToUInt16(GameObject.Find("InputFieldPort").GetComponent<InputField>().text);
        m_login = GameObject.Find("InputFieldLogin").GetComponent<InputField>().text;
        m_password = GameObject.Find("InputFieldPassword").GetComponent<InputField>().text;
    }

    private T GetChildComponentByName<T>(string name) where T : Component
    {
        foreach (T component in GetComponentsInChildren<T>(true))
        {
            if (component.gameObject.name == name)
            {
                return component;
            }
        }
        return null;
    }

    private void ProcessMessage(UInt16 header, UInt32 buffersize, byte[] payload)
    {
        //if (BitConverter.IsLittleEndian)
        //{
        //    m_tcp_header = (UInt16)IPAddress.NetworkToHostOrder((Int16)h);
        //    m_tcp_buffersize = (UInt32)IPAddress.NetworkToHostOrder((Int32)bs);
        //}

        /*
        if (origin_is_big_endian == 0xFFFF)
        {
            Array.Reverse(payload,0,payload.Length);
        }
        */
        //string tmp = BitConverter.ToString(payload);
        GameObject GO = GameObject.Find("LOCAL_PARTICIPANT");
        XRNetworkObject NO = GO.GetComponent<XRNetworkObject>() as XRNetworkObject;
        
        switch ((XRNetworkProtocol.EN_RAW_MESSAGE_HEAD)header)
        {
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_HELLO:
                XRNetworkProtocol.ST_HANDSHAKE_HELLO st_hello = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_HANDSHAKE_HELLO>(payload);
                m_service_name = Encoding.UTF8.GetString(st_hello.service_name_buffer);
                m_service_id = st_hello.service_id;
                m_participant_id_number = st_hello.participant_id;
                m_participant_id = st_hello.participant_id.ToString("X");
                m_service_timestamp = st_hello.service_timestamp;
                // instantiate hello_ack
                XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK hello_ack = new XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK();
                hello_ack.participant_id = m_participant_id_number;
                hello_ack.client_timestamp = m_tcpClient.UTCTiemstampSinceEpoch();
                hello_ack.participant_buffer = new byte[1024 * 20];
                Array.Clear(hello_ack.participant_buffer, 0, 1024 * 20);
                byte[] json_array = Encoding.UTF8.GetBytes(player_descriptor_json);
                Array.Copy(json_array, hello_ack.participant_buffer, json_array.Length);
                byte[] byte_hello_ack = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_HANDSHAKE_HELLO_ACK>(hello_ack);
                XRNetworkProtocol.ST_RAW_MESSAGE rmsg = XRNetworkProtocol.BuildRawMessage((UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_HELLO_ACK,
                                                                                          byte_hello_ack,
                                                                                          (UInt32)byte_hello_ack.Length);
                m_tcpClient.AsyncSend(rmsg.ToArray());
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_CREDENTIALS:
                //dbgText += "HANDSHAKE_CREDENTIALS";
                XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS hcred = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS>(payload);
                dbgText = "";
                // HADNSHAKE CREDENTIALS ACK
                XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS_ACK ack = new XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS_ACK();
                ack.login_buffer = new byte[1024];
                ack.password_buffer = new byte[1024];
                ack.participant_certificate_buffer = new byte[20 * 1024];

                ack.login_buffersize = (UInt16)m_login.Length;
                byte[] tmp1 = new byte[1024];
                tmp1 = Encoding.UTF8.GetBytes(m_login);
                Array.Copy(tmp1, 0, ack.login_buffer, 0, tmp1.Length);
                ack.password_buffersize = (UInt16)m_password.Length;
                tmp1 = Encoding.UTF8.GetBytes(m_password);
                Array.Copy(tmp1, 0, ack.password_buffer, 0, tmp1.Length);
                ack.participant_id = m_participant_id_number;
                //ack.participant_certificate_buffer
                //ack.participant_certificate_buffersize
                byte[] ack_byte = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_HANDSHAKE_CREDENTIALS_ACK>(ack);
                XRNetworkProtocol.ST_RAW_MESSAGE ret_raw_message_2 = XRNetworkProtocol.BuildRawMessage( (UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.HANDSHAKE_CREDENTIALS_ACK,
                                                                                                        ack_byte,
                                                                                                        (UInt32)ack_byte.Length);
                m_tcpClient.AsyncSend(ret_raw_message_2.ToArray());
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
                dbgText = "PARTICIPANT_JOIN\n";
                XRNetworkProtocol.ST_PARTICIPANT_JOIN join = new XRNetworkProtocol.ST_PARTICIPANT_JOIN();
                join.message_buffer = new byte[1024];
                Array.Clear(join.message_buffer, 0, 1024);
                join = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_JOIN>(payload);
                dbgText += "participant_id: " + join.participant_id.ToString("X") +
                            "\nmax_data_rate: " + join.max_data_rate.ToString("X") +
                            "\nmessage_buffersize: " + join.message_buffersize.ToString() +
                            "\nmessage_buffer: " + Encoding.UTF8.GetString(join.message_buffer) + "\n";
                m_participant_id_number = join.participant_id;
                m_participant_id = join.participant_id.ToString(); // in case it changes with new value
                m_max_data_rate = join.max_data_rate;
                NO.SetID(m_participant_id_number);
                Debug.Log(dbgText);

                XRNetworkProtocol.ST_PARTICIPANT_JOIN_ACK jack = new XRNetworkProtocol.ST_PARTICIPANT_JOIN_ACK();
                jack.participant_id = m_participant_id_number;
                jack.json_buffersize = (UInt32)1024*20;
                jack.json_buffer = new byte[jack.json_buffersize];
                //////
                string JSONjack = Save<XRNetworkPlayer>(m_local_participant_go);
                byte[] JSONjackArray = Encoding.UTF8.GetBytes(JSONjack);
                //////
                Array.Copy(JSONjackArray, jack.json_buffer, JSONjackArray.Length);

                byte[] jack_array = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_PARTICIPANT_JOIN_ACK>(jack, XRNetworkProtocol.ST_PARTICIPANT_JOIN_ACK_SIZE);
                XRNetworkProtocol.ST_RAW_MESSAGE strm = XRNetworkProtocol.BuildRawMessage((UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_JOIN_ACK,
                                                                                          jack_array,
                                                                                          (UInt32)jack_array.Length);
                //Debug.Log("jack_array length: " + jack_array.Length.ToString()); // correct !!
                //byte[] ret_tmp_2 = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_RAW_MESSAGE>(strm,XRNetworkProtocol.ST_RAW_MESSAGE_SIZE);
                m_tcpClient.AsyncSend(strm.ToArray());
                dbgText += "ST_PARTICIPANT_JOIN_ACK\n";
                dbgText += "participant_id = " + jack.participant_id.ToString("X");
                //Debug.Log(dbgText);
                dbgText = "";
                b_on_room = true;
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_NEW:
                dbgText += "PARTICIPANT_NEW\n";
                XRNetworkProtocol.ST_PARTICIPANT_NEW newp = new XRNetworkProtocol.ST_PARTICIPANT_NEW();
                newp = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_NEW>(payload);
                UInt64 new_pid = newp.participant_id;
                string jsonNewp = Encoding.UTF8.GetString(newp.descriptor_buffer, 0, (int)newp.descriptor_buffersize);
                Debug.Log(dbgText + "\nparticipant_id: " + newp.participant_id.ToString("X") + "\nJSON: " + jsonNewp);
                GameObject tmp3 = Instantiate(GO.GetComponent<XRNetworkPlayer>().m_prefab,transform);
                tmp3.name = new_pid.ToString("X");
                tmp3.AddComponent<XRNetworkPlayer>();
                dbgText = "";
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_UPDATE_ACK:
                XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK uack = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK>(payload);
                
                /*dbgText += "<< PARTICIPANT_UPDATE_ACK" +
                           "\nparticipant_id: " + uack.participant_id.ToString("X") +
                           "\ntimestamp: " + uack.origin_timestamp.ToString() +
                           "\nbuffersize: " + uack.buffersize.ToString() +
                           "\nbuffer: " + Encoding.UTF8.GetString(uack.buffer);
                
                Debug.Log(dbgText);*/
                Debug.Log( "<< PARTICIPANT_UPDATE_ACK (" + uack.participant_id.ToString("X") + ")");
                //Debug.Log("<< PARTICIPANT_UPDATE_ACK");
                //string jsonStringIN = Encoding.UTF8.GetString(uack.buffer);
                //Debug.Log("PARTICIPANT_UPDATE_ACK " + uack.participant_id.ToString("X") + "\nJSON: " + jsonStringIN);

                //GameObject fGO = GameObject.Find(uack.participant_id.ToString());
                //if (fGO == null) break;
                //Restore<XRNetworkObject>(jsonStringIN,fGO);
                //XRNetworkObject mNO = GameObject.Find(uack.participant_id.ToString()).GetComponent<XRNetworkObject>();
                //mNO.UpdateFromService();
                dbgText = "";
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_LEAVE:
                XRNetworkProtocol.ST_PARTICIPANT_LEAVE p_leave = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_LEAVE>(payload);
                Debug.Log("PARTICIPANT_LEAVE\nparticipant_id: " + p_leave.participant_id.ToString("X") + "\n");
                if (GameObject.Find(p_leave.participant_id.ToString("X")) != null)
                {
                    Destroy(GameObject.Find(p_leave.participant_id.ToString("X")));
                }

                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_LEAVE_ACK:
                // we should not be here.
                break;
            default:
                break;
        }
        dbgText = "";
    }

    public GameObject getChildGameObject(GameObject fromGameObject, string withName)
    {
        //Author: Isaac Dart, June-13.
        /*
        Transform[] ts = fromGameObject.transform.GetComponentsInChildren();
        foreach (Transform t in ts) if (t.gameObject.name == withName) return t.gameObject;
        */
        return null;
    }

    public Transform FindChildByRecursion(Transform aParent, string aName)
    {
        if (aParent == null) return null;
        var result = aParent.Find(aName);
        if (result != null)
            return result;
        foreach (Transform child in aParent)
        {
            result = FindChildByRecursion(child,aName);
            if (result != null)
                return result;
        }
        return null;
    }

    public string Save<T>(GameObject obj)
    {
        string _json = JsonUtility.ToJson(obj.GetComponent<T>());
        return _json;
    }

    public void Restore<T>(string _json, GameObject obj)
    {
        JsonUtility.FromJsonOverwrite(_json, obj.GetComponent<T>());
    }

    /*
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
    */

}
