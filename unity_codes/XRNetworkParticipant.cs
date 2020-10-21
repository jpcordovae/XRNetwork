using System;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using System.Collections;
using System.Collections.Concurrent;
using System.Threading;
using System.Xml;
using System.ComponentModel;
using Unity.Collections;
using ReadOnlyAttribute = Unity.Collections.ReadOnlyAttribute;

public class XRNetworkParticipant : MonoBehaviour
{
    // callbacks 
    // Threads
    //private ConcurrentQueue<string> m_concurrent_queue;
    // time management
  
    //#region ParticipantThreads
    /*public void ParticipantUpdateFunction()
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
    }*/
    //#endregion 
    
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    

    

}
