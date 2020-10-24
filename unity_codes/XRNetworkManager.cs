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
using System.Threading;
using System.Runtime.InteropServices;

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
    private readonly object joined_lock_object = new object();
    static private bool m_joined;

    public static bool joined {
        get {
            bool j;
            Thread.MemoryBarrier();
            j = m_joined;
            Thread.MemoryBarrier();
            return j;
        }
        set {
            Thread.MemoryBarrier();
            m_joined = value;
            Thread.MemoryBarrier();
        }
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

    private static ConcurrentQueue<XRNetworkProtocol.ST_RAW_MESSAGE> pending_input_message_queue = new ConcurrentQueue<XRNetworkProtocol.ST_RAW_MESSAGE>();
    private ConcurrentQueue<byte[]> pending_output_messages_queue = new ConcurrentQueue<byte[]>();

    public void send_message_to_server(byte[] barray)
    {
        pending_output_messages_queue.Enqueue(barray);
    }

    GameObject GetGameObjectbyID(UInt64 ID)
    {
        foreach (Transform child in transform)
        {
            XRNetworkObject myGO = child.GetComponent<XRNetworkObject>();
            if (myGO != null)
                if (myGO.m_id == ID)
                    return child.gameObject;
        }
        return null;
    }

    //private static XRAsyncTCPClient m_tcpClient = XRAsyncTCPClient.GetInstance();
    private static string dbgText;
    private int m_dt_ms = 100; // ms 
    private long ms_last_timestamp;

    void Start()
    {
        //XRNetworkObject NO = ;
        GameObject.Find("LOCAL_PARTICIPANT").GetComponent<XRNetworkObject>().SetPlayerName(GameObject.Find("InputFieldName").GetComponent<InputField>().text);
        player_descriptor_json = Save<XRNetworkObject>(m_local_participant_go);
        ms_last_timestamp = 0;
        joined = false;
    }

    private void OnDisable()
    {
        XRNetworkClient.xrn_disconnect();
    }

    private void OnDestroy()
    {
        XRNetworkClient.xrn_disconnect();
    }

    public void ConnectToService()
    {
        XRNetworkClient.xrn_start_service_thread();
        RegisterCallbacks();
        XRNetworkClient.xrn_connect("192.168.1.6", "1080", "login", "password");
    }

    public void DisconnectFromService()
    {
        XRNetworkClient.xrn_disconnect();
    }

    // this doesn't run on the main thread, so... it sucks...
    private static void OnConnect()
    {
        
    }

    private static void OnDisconnect()
    {
        joined = false;
    }

    public bool IsJoined()
    {
        return joined;
    }

    void OnRoom()
    {
        Debug.Log("<< JOINED");
        joined = true;
    }

    static void OnNewMessage(UInt16 head, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)]  byte[] buffer, UInt32 buffersize)
    {
        try
        {
            XRNetworkProtocol.ST_RAW_MESSAGE rw = new XRNetworkProtocol.ST_RAW_MESSAGE();
            //rw = (XRNetworkProtocol.ST_RAW_MESSAGE) Marshal.PtrToStructure(pMsg, typeof(XRNetworkProtocol.ST_RAW_MESSAGE));
            rw.header.head = head;
            rw.header.buffersize = buffersize;
            rw.buffer = new byte[XRNetworkProtocol.ST_RAW_MESSAGE_PAYLOAD_SIZE];
            Debug.Log(string.Format("<< NEW MESSAGE\nhead: {0}\nbuffersize: \n", rw.header.head.ToString(), rw.header.buffersize.ToString()));
            Buffer.BlockCopy(buffer, 0, rw.buffer, 0, (int)rw.header.buffersize);
            pending_input_message_queue.Enqueue(rw);
        } catch (Exception e)
        {
            Debug.Log(string.Format("{0} Exception caught.", e));
        }
    }

    private void RegisterCallbacks()
    {
        XRNetworkClient.xrn_set_on_connect_callback(OnConnect);
        XRNetworkClient.xrn_set_on_disconect_callback(OnDisconnect);
        XRNetworkClient.xrn_set_on_new_message_callback(OnNewMessage);
        XRNetworkClient.xrn_set_on_room_callback(OnRoom);
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
            while (pending_input_message_queue.TryDequeue(out raw))
            {
                Debug.Log("Processing Message");
                ProcessMessage(raw.header.head, raw.buffer, raw.header.buffersize);
            }

            if (m_joined)
            {
                long ms_timestamp = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                if (ms_timestamp - ms_last_timestamp > m_dt_ms)
                {
                    byte[] barray = new byte[] { };
                    //XRNetworkProtocol.ST_RAW_MESSAGE msg = new XRNetworkProtocol.ST_RAW_MESSAGE();
                    while (pending_output_messages_queue.TryDequeue(out barray))
                    {
                        byte[] msg_buffer = new byte[XRNetworkProtocol.ST_RAW_MESSAGE_PAYLOAD_SIZE];
                        Array.Copy(barray, msg_buffer, barray.Length);
                        //Debug.Log(">> PARTICIPANT_UPDATE_ACK");
                        //Debug.Log(  "ms_timestamp: " + ms_timestamp.ToString() + ", ms_last_timestamp: " + ms_last_timestamp.ToString() + ", dt: " + (ms_timestamp - ms_last_timestamp).ToString() + "\nsize: " + msg.header.buffersize.ToString());
                        //byte[] msgarray = XRNetworkProtocol.GetBytes(msg, XRNetworkProtocol.ST_RAW_MESSAGE_SIZE);
                        XRNetworkClient.xrn_send_message((UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_UPDATE_ACK,
                                                         msg_buffer,
                                                         (UInt32)msg_buffer.Length);
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

    private void ProcessMessage(UInt16 header, byte[] payload, UInt32 buffersize)
    {
        GameObject GO = GameObject.Find("LOCAL_PARTICIPANT");
        XRNetworkObject NO = GO.GetComponent<XRNetworkObject>() as XRNetworkObject;
        
        switch ((XRNetworkProtocol.EN_RAW_MESSAGE_HEAD)header)
        {
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_INFO_REQUEST:
                dbgText += "PARTICIPANT_INFO_REQUEST";
                Debug.Log(dbgText);
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_NEW:
                dbgText = "<< PARTICIPANT_NEW\n";
                XRNetworkProtocol.ST_PARTICIPANT_NEW newp = new XRNetworkProtocol.ST_PARTICIPANT_NEW();
                newp = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_NEW>(payload);
                UInt64 new_pid = newp.participant_id;
                string jsonNewp = Encoding.UTF8.GetString(newp.descriptor_buffer, 0, (int)newp.descriptor_buffersize);
                dbgText += "\nparticipant_id: " + newp.participant_id.ToString("X") + "\nJSON: " + jsonNewp;
                GameObject tmp3 = Instantiate(GO.GetComponent<XRNetworkPlayer>().m_prefab,transform);
                tmp3.name = new_pid.ToString("X");
                XRNetworkPlayer xrnp = tmp3.AddComponent<XRNetworkPlayer>();
                xrnp.m_id = new_pid;
                xrnp.m_id_hex = new_pid.ToString("X");
                Debug.Log(dbgText);
                dbgText = "";
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_JOIN:
                XRNetworkProtocol.ST_PARTICIPANT_JOIN pj = new XRNetworkProtocol.ST_PARTICIPANT_JOIN();
                pj = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_JOIN>(payload);
                NO.m_id = pj.participant_id;
                NO.m_id_hex = pj.participant_id.ToString("X");
                m_service_id = XRNetworkClient.xrn_get_service_id();
                XRNetworkProtocol.ST_PARTICIPANT_JOIN_ACK pjack = new XRNetworkProtocol.ST_PARTICIPANT_JOIN_ACK();
                pjack.participant_id = NO.m_id;
                //string json_string = ;
                pjack.json_buffer = Encoding.UTF8.GetBytes(Save<XRNetworkObject>(NO.gameObject));
                pjack.json_buffersize = (UInt32)pjack.json_buffer.Length;
                byte[] pjack_buffer = XRNetworkProtocol.GetBytes<XRNetworkProtocol.ST_PARTICIPANT_JOIN_ACK>(pjack);
                //pending_output_messages_queue.Enqueue(pjcak_buffer);
                XRNetworkClient.xrn_send_message((UInt16)XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_JOIN_ACK,
                                                    pjack_buffer,
                                                    (UInt32)pjack_buffer.Length);
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_UPDATE_ACK:
                XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK uack = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK>(payload);
                /*dbgText += "<< PARTICIPANT_UPDATE_ACK" +
                           "\nparticipant_id: " + uack.participant_id.ToString("X") +
                           "\ntimestamp: " + uack.origin_timestamp.ToString() +
                           "\nbuffersize: " + uack.buffersize.ToString() +
                           "\nbuffer: " + Encoding.UTF8.GetString(uack.buffer);
                Debug.Log(dbgText);*/
                //Debug.Log( "<< PARTICIPANT_UPDATE_ACK (" + uack.participant_id.ToString("X") + ")");
                //Debug.Log("<< PARTICIPANT_UPDATE_ACK");
                string jsonStringIN = Encoding.UTF8.GetString(uack.buffer);
                //Debug.Log("<< PARTICIPANT_UPDATE_ACK " + uack.participant_id.ToString("X") + "\nJSON: " + jsonStringIN);
                GameObject fGO = GameObject.Find(uack.participant_id.ToString("X"));
                if (fGO == null) break;
                XRNetworkObject mNO = Restore<XRNetworkObject>(jsonStringIN,fGO);
                //XRNetworkObject mNO = GameObject.Find(uack.participant_id.ToString("X")).GetComponent<XRNetworkObject>();
                mNO.UpdateFromService();
                dbgText = "";
                break;
            case XRNetworkProtocol.EN_RAW_MESSAGE_HEAD.PARTICIPANT_LEAVE:
                XRNetworkProtocol.ST_PARTICIPANT_LEAVE p_leave = XRNetworkProtocol.ByteArrayToStructure<XRNetworkProtocol.ST_PARTICIPANT_LEAVE>(payload);
                Debug.Log("<< PARTICIPANT_LEAVE\nparticipant_id: " + p_leave.participant_id.ToString("X") + "\n");
                
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

    public T Restore<T>(string _json, GameObject obj)
    {
        JsonUtility.FromJsonOverwrite(_json, obj.GetComponent<T>());
        return obj.GetComponent<T>();
    }

}
