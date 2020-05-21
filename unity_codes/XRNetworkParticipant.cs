using System;
using System.Collections;
using System.Runtime.InteropServices;
using UnityEngine;

public class XRNetworkParticipant : MonoBehaviour
{
    [SerializeField] private UInt64 m_id;
    [SerializeField] private string m_name;
    [SerializeField] private string m_host;
    [SerializeField] private uint m_port;
    [SerializeField] private XRNetworkAPI.ST_PARTICIPANT_INFO m_info = new XRNetworkAPI.ST_PARTICIPANT_INFO();
    [SerializeField] public XRNetworkAPI.ST_PARTICIPANT_INFO info { get { return m_info; } set { } }
    private XRAsyncTCPClient m_tcpClient;// = new XRAsyncTCPClient();
    XRNetworkAPI.ST_PARTICIPANT_MESSAGE message = new XRNetworkAPI.ST_PARTICIPANT_MESSAGE();

    // callbacks 
    private void OnConnect() 
    {
        Debug.Log("OnConnect triggered");
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
        Debug.Log("OnReceive triggered");
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
        m_tcpClient.RegisterCallbackOnError(OnTCPError);
    }

    void OnTCPError(string msg)
    {
        Debug.LogError(msg);
    }

}
