using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using System.Collections;
using UnityEngine;
using UnityEditor;
using System.Collections.Concurrent;

public class StateObject
{
    public Socket workSocket = null;
    public const int BufferSize = 64 * 1024;
    public byte[] buffer = new byte[BufferSize];
    public const int tx_buffersize = 64 * 1024;
    public byte[] tx_buffer = new byte[tx_buffersize];
}

public class XRAsyncTCPClient
{
    private Socket m_client = null;
    private int m_port;
    public int port { get { return m_port; } set { m_port = value; } }

    private static string m_host;
    public string host { get { return m_host; } set { m_host = value; } }

    private bool m_IsConnected;
    public bool IsConnected { get { return m_IsConnected; } set { } }

    // BUFFERS
    private static int m_txBufferSize = 1024 * 64; // 64kb
    private byte[] m_txBuffer = new byte[m_txBufferSize];
    private static int m_rxBufferSize = 1024 * 64; // 64kb 
    private byte[] m_rxBuffer = new byte[m_rxBufferSize];
    //private byte[] m_tmp_rx_buffer = new byte[m_rxBufferSize];
    //private int rx_tmp_index = 0;

    //CALLBACKS
    public delegate void OnConnectCallbackDelegate();
    public delegate void OnSendCallbackDelegate();
    public delegate void OnReceiveCallbackDelegate(byte[] message, int buffersize);
    public delegate void OnDisconnectCallbackDelegate();
    public delegate void OnErrorCallbackDelegate(string message);

    private OnConnectCallbackDelegate occd;
    private OnSendCallbackDelegate oscd;
    private OnReceiveCallbackDelegate orcd;
    private OnDisconnectCallbackDelegate odcd;
    private OnErrorCallbackDelegate oecd;

    private ConcurrentDictionary<int, byte[]> ByteContainer;
    private static StateObject state = new StateObject();

    public XRAsyncTCPClient()
    {
    }

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

    // CONNECT 
    public void Connect(string host, int port)
    {
        Debug.Log("Connecting to " + host + ":" + port.ToString());
        //sendDone.Reset();
        AsyncConnect(host, port);
        //sendDone.WaitOne();
    }

    public void Connect()
    {
        Connect(m_host, m_port);
    }

    public void AsyncConnect(string host, int port)
    {
        try
        {
            m_host = host;
            m_port = port;

            IPHostEntry ipHostInfo = Dns.GetHostEntry(host);
            IPAddress ipAddress = ipHostInfo.AddressList[0];
            IPEndPoint remoteEP = new IPEndPoint(ipAddress, port);

            m_client = new Socket(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            m_client.ReceiveBufferSize = 64 * 1024;
            state.workSocket = m_client;
            m_client.BeginConnect(remoteEP, new AsyncCallback(asyncConnectCallback), m_client);
        }
        catch (Exception e)
        {
            oecd("AsyncConnect:" + e.ToString());
        }
    }

    private void asyncConnectCallback(IAsyncResult ar)
    {
        try
        {
            Socket client = (Socket)ar.AsyncState;
            client.EndConnect(ar);
            //m_IsConnected = true;
#if UNITY_EDITOR
            Debug.Log("connected to " + m_client.RemoteEndPoint.ToString());
#endif
            occd();
        }
        catch (Exception e)
        {
            oecd("connectCallback error: " + e.ToString());
        }
    }

    // SEND

    private void asyncSendCallback(IAsyncResult ar)
    {
        try
        {
            StateObject so = (StateObject)ar.AsyncState;
            Socket snd_socket = so.workSocket;
            int bytesSent = snd_socket.EndSend(ar);
            
#if UNITY_EDITOR
            Debug.Log(bytesSent.ToString() + " bytes sended ");
#endif
            //sendDone.Set();
            oscd();
        }
        catch (Exception e)
        {
            oecd("asyncSendCallback:" + e.Message);
            //Disconnect();
        }
    }

    public void AsyncSend(byte[] message, int buffer_size)
    { 
        
        if (!m_client.Connected)
        {
            return;
        }
        try
        {
            Array.Clear(state.tx_buffer, 0, state.tx_buffer.Length);
            Buffer.BlockCopy(message, 0, state.tx_buffer, 0, buffer_size);
            m_client.BeginSend( state.tx_buffer, 
                                0, 
                                state.tx_buffer.Length, 
                                0/*SocketFlags.DontRoute*/, 
                                new AsyncCallback(asyncSendCallback), 
                                state);
        }
        catch (Exception e)
        {
            oecd("AsyncSend: " + e.Message);
        }
        
    }

    // RECEIVE
    private void asyncReceiveCallback(IAsyncResult ar)
    {
        try
        {
            //StateObject state = (StateObject)ar.AsyncState;
            Socket client = state.workSocket;
            int bytesRead = client.EndReceive(ar);
            //Debug.Log(string.Format("receiving {0} bytes.", bytesRead));
            if (bytesRead > 0)
            {
                byte[] tmp = SplitByteArray(state.buffer, bytesRead); 
                m_rxBuffer = tmp;
                
            }

            if (bytesRead == 0 || client.Available == 0)
            {
                orcd(m_rxBuffer, m_rxBuffer.Length);
                //m_rxBuffer = new byte[StateObject.BufferSize];
                state.buffer = new byte[StateObject.BufferSize];
            }

            client.BeginReceive(state.buffer,//m_rxBuffer, 
                    0,
                    StateObject.BufferSize,//m_rxBufferSize, 
                    SocketFlags.DontRoute,
                    new AsyncCallback(asyncReceiveCallback),
                    state);

        }
        catch (SocketException e)
        {
            oecd("asyncReceiveCallback:" + e.ToString());
            //return false;
        }
        catch (Exception e)
        {
            oecd("asyncReceiveCallback:" + e.ToString());
        }

    }

    public void AsyncReceive()
    {
        state.buffer = new byte[StateObject.BufferSize];
        m_client.BeginReceive(state.buffer, 0, state.buffer.Length, 0, new AsyncCallback(asyncReceiveCallback), state);
    }

    // DISCONNECT

    public void Disconnect()
    {
        if (m_client != null) { 
            m_client.Shutdown(SocketShutdown.Both);
            m_client.Close();
        }
        odcd();
    }

    // UTILS
    public static byte[] SplitByteArray(byte[] buffer, int count)
    {
        byte[] ret = new byte[count];
        Buffer.BlockCopy(buffer, 0, ret, 0, count);
        return ret;
    }

    public static byte[] Combine(byte[] first, byte[] second)
    {
        byte[] ret = new byte[first.Length + second.Length];
        Buffer.BlockCopy(first, 0, ret, 0, first.Length);
        Buffer.BlockCopy(second, 0, ret, first.Length, second.Length);
        return ret;
    }

    public UInt64 UTCTiemstampSinceEpoch()
    {
        DateTimeOffset now = DateTimeOffset.UtcNow;
        return (UInt64)now.ToUnixTimeMilliseconds();
    }

}
