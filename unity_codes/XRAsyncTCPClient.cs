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
    public bool IsConnected { get { return m_client.Connected; } set { } }
    
    // BUFFERS
    private static int m_txBufferSize = 1024 * 64; // 64kb
    private byte[] m_txBuffer = new byte[m_txBufferSize];
    private static int m_rxBufferSize = 1024 * 64; // 64kb 
    private byte[] m_rxBuffer = Array.Empty<byte>();

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
    // send
    private Thread ThreadSendMessages;
    public ConcurrentQueue<byte[]> m_concurrent_send_message_queue = new ConcurrentQueue<byte[]>();
    private static ManualResetEvent m_send_message_event = new ManualResetEvent(false);
    private int m_send_dt = 100; // in milliseconds
    public XRAsyncTCPClient()
    {
            
    }

    public void StartSendingThread()
    {
        ThreadSendMessages = new Thread(SendMessagesThreaded);
    }

    public void StopSendingThread()
    {
        if (ThreadSendMessages.IsAlive)
        {
            ThreadSendMessages.Abort();
        }
    }

    public void QueueMessage(byte[] buffer)
    {
        m_concurrent_send_message_queue.Enqueue(buffer);
        m_send_message_event.Set();
    }

    private void SendMessagesThreaded()
    {
       /* while(true)
        {
            m_send_message_event.WaitOne();
            m_send_message_event.Reset();
            while (!m_concurrent_send_message_queue.IsEmpty)
            {
                byte[] message;
                if (m_concurrent_send_message_queue.TryDequeue(out message))
                {
                    AsyncSend(message, message.Length);
                }
            }
            //Thread.Sleep(m_send_dt);
        }*/
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
            //Debug.Log(bytesSent.ToString() + " bytes sended ");
#endif
            //sendDone.Set();
            oscd();
        }
        catch (SocketException e) 
        {
            oecd("asyncSendCallback (SE):" + e.Message);
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
            Debug.Log(string.Format("AsyncSend {0} bytes:\n",buffer_size) + BitConverter.ToString(message));
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
            StateObject state = (StateObject)ar.AsyncState;
            Socket client = state.workSocket;
            int bytesRead = client.EndReceive(ar);
            //Debug.Log(string.Format("asyncReceiveCallback {0} bytes\nbuffer: {1}", bytesRead,BitConverter.ToString(state.buffer)));
            
            if (bytesRead > 0)
            {
                byte[] tmp = SplitByteArray(state.buffer, 0, bytesRead); 
                m_rxBuffer = XRNetworkProtocol.ConcatByteArrays(m_rxBuffer,tmp);
                Array.Clear(state.buffer, 0, StateObject.BufferSize);
                
            }

            if (bytesRead == 0 || client.Available == 0)
            {
                //byte[] new_byte = new byte[m_rxBuffer.Length];
                //Array.Copy(m_rxBuffer,new_byte,m_rxBuffer.Length);
                //orcd(new_byte, new_byte.Length);
                //Debug.Log(string.Format("asyncReceiveCallback {0} bytes\nbuffer: {1}", bytesRead, BitConverter.ToString(m_rxBuffer)));
                orcd(m_rxBuffer,m_rxBuffer.Length);
                Array.Clear(state.buffer, 0, StateObject.BufferSize);
                m_rxBuffer = Array.Empty<byte>();
            }

            //Array.Clear(state.buffer, 0, StateObject.BufferSize); // DELETE THIS IF MAKE TROUBLES
            client.BeginReceive(state.buffer,//m_rxBuffer, 
                    0,
                    StateObject.BufferSize,//m_rxBufferSize, 
                    0,
                    new AsyncCallback(asyncReceiveCallback),
                    state);

        }
        catch (ObjectDisposedException) // can occur when closing
        {
            // ignore
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
        //state.buffer = new byte[StateObject.BufferSize];
        Array.Clear(state.buffer, 0, StateObject.BufferSize);
        m_client.BeginReceive(  state.buffer, 
                                0, 
                                state.buffer.Length,
                                0, 
                                new AsyncCallback(asyncReceiveCallback), 
                                state);
    }

    // DISCONNECT

    public void Disconnect()
    {
        //StopSendingThread();
        if (m_client != null) { 
            m_client.Shutdown(SocketShutdown.Both);
            m_client.Close();
            //m_client.Dispose();
            //m_client = null;
        }
        odcd();
    }

    // UTILS
    public static byte[] SplitByteArray(byte[] buffer, int index, int count)
    {
        byte[] ret = new byte[count];
        Buffer.BlockCopy(buffer, index, ret, 0, count);
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
