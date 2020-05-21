using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using System.Collections;
using UnityEngine;
using UnityEditor;

public class StateObject
{
    public Socket workSocket = null;
    public const int BufferSize = 256;
    public byte[] buffer = new byte[BufferSize];
    public StringBuilder sb = new StringBuilder();
}

public class XRAsyncTCPClient
{
    private static Socket m_client = null;
    private static int m_port;
    public int port { get { return m_port; } set { m_port = value; } }

    private static string m_host;
    public string host { get { return m_host; } set { m_host = value; } }

    private static bool m_IsConnected;
    public bool IsConnected { get { return m_IsConnected; } set { } }

    // manual signals completions
    private static ManualResetEvent connectDone = new ManualResetEvent(false);
    private static ManualResetEvent sendDone = new ManualResetEvent(false);
    private static ManualResetEvent receiveDone = new ManualResetEvent(false);
    private static String response = String.Empty;

    // BUFFERS
    private static int m_txBufferSize = 1024 * 64; // 64kb
    private static byte[] m_txBuffer = new byte[m_txBufferSize];
    private static int m_rxBufferSize = 1024 * 64; // 64kb 
    private static byte[] m_rxBuffer = new byte[m_rxBufferSize];

    //CALLBACKS
    public delegate void OnConnectCallbackDelegate();
    public delegate void OnSendCallbackDelegate();
    public delegate void OnReceiveCallbackDelegate(byte[] message, int buffersize);
    public delegate void OnDisconnectCallbackDelegate();
    public delegate void OnErrorCallbackDelegate(string message);

    private static OnConnectCallbackDelegate occd;
    private static OnSendCallbackDelegate oscd;
    private static OnReceiveCallbackDelegate orcd;
    private static OnDisconnectCallbackDelegate odcd;
    private static OnErrorCallbackDelegate oecd;

    public void RegisterCallbackOnConnect(OnConnectCallbackDelegate oc)
    {
        occd = new OnConnectCallbackDelegate(oc);
    }

    public void RegisterCallbackOnReceive(OnReceiveCallbackDelegate ormc)
    {
        orcd = new OnReceiveCallbackDelegate(ormc);
    }

    public void RegisterCallbackOnSend(OnSendCallbackDelegate osc)
    {
        oscd = new OnSendCallbackDelegate(osc);
    }

    public void RegisterCallbackOnDisconnect(OnDisconnectCallbackDelegate odc)
    {
        odcd = new OnDisconnectCallbackDelegate(odc);
    }

    public void RegisterCallbackOnError(OnErrorCallbackDelegate oec)
    {
        oecd = new OnErrorCallbackDelegate(oec);
    }

    // class callbacks
    private static void asyncConnectCallback(IAsyncResult ar)
    {
        try
        {
            if (m_client == null) return; // case of asyn destruction or not initialized
            m_client.EndConnect(ar);
            m_IsConnected = true;
#if UNITY_EDITOR
            Debug.Log("connected to " + m_client.RemoteEndPoint.ToString());
#endif
            occd();
        }
        catch (Exception e)
        {
            oecd("connectCallback error: " + e.Message);
        }
    }

    public void Connect(string host, int port)
    {
        Debug.Log("Attempting to connet to " + host + " through port " + port.ToString());
        //sendDone.Reset();
        AsyncConnect(host, port);
        //sendDone.WaitOne();
    }

    public void Connect()
    {
        Connect(m_host, m_port);
    }

    public static void AsyncConnect(string host, int port)
    {
        m_host = host;
        m_port = port;
        IPHostEntry ipHostInfo = Dns.GetHostEntry(host);
        IPAddress ipAddress = ipHostInfo.AddressList[0];
        IPEndPoint remoteEP = new IPEndPoint(ipAddress, port);

        m_client = new Socket(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
        m_client.BeginConnect(remoteEP, new AsyncCallback(asyncConnectCallback), m_client);
    }

    private static void asyncSendCallback(IAsyncResult ar)
    {
        try
        {
            int bytesSent = m_client.EndSend(ar);
#if UNITY_EDITOR
            Debug.Log("sending " + bytesSent + " bytes of data.");
#endif
            //sendDone.Set();
            oscd();
        }
        catch (Exception e)
        {
            oecd(e.Message);
        }
    }

    public static void AsyncSend(byte[] message, int buffer_size)
    {
        if (!m_IsConnected)
        {
            return;
        }
        m_client.BeginSend(message, 0, buffer_size, SocketFlags.DontRoute, new AsyncCallback(asyncSendCallback), m_client);
    }

    private static void asyncReceiveCallback(IAsyncResult ar)
    {
        try
        {
            int bytesRead = m_client.EndReceive(ar);
            /*if (bytesRead > 0)
            {
                m_client.BeginReceive(m_rxBuffer, 0, m_rxBufferSize, SocketFlags.DontRoute, new AsyncCallback(asyncReceiveCallback), m_client);
            } else
            {
                //receiveDone.Set();

            }*/
            m_client.BeginReceive(m_rxBuffer, 0, m_rxBufferSize, SocketFlags.DontRoute, new AsyncCallback(asyncReceiveCallback), m_client);
        }
        catch (Exception e)
        {
            oecd(e.Message);
        }
    }

    public static void AsyncReceive()
    {
        StateObject state = new StateObject();
        state.workSocket = m_client;
        m_client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, new AsyncCallback(asyncReceiveCallback), state);
        //receiveDone.WaitOne();
    }

    public void Disconnect()
    {
        if (m_client != null) { 
            m_client.Shutdown(SocketShutdown.Both);
            m_client.Close();
        }
    }

}
