using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;
using UnityEngine;

public class XRNetworkLocalPlayer : XRNetworkPlayer
{
    private long timestamp;
    // Start is called before the first frame update
    void Start()
    {
        m_prefab_path = UnityEditor.AssetDatabase.GetAssetPath(m_prefab);
        objectType = EN_XRN_ObjectType.XRN_OT_PLAYER;
        timestamp = DateTime.UtcNow.Millisecond;
    }

    // Update is called once per frame
    void Update()
    {
        long now = DateTimeOffset.Now.ToUnixTimeMilliseconds();
        if (XRNetworkManager.Instance.on_room && (now-timestamp) > 100)
        {
            XRNetworkObject NO = GetComponent<XRNetworkObject>();
            NO.m_position = transform.position;
            NO.m_rotation = transform.rotation;
            XRNetworkManager NM = XRNetworkManager.Instance;
            
            XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK pack = new XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK();
            pack.participant_id = m_id;
            pack.origin_timestamp = (UInt64)DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            pack.buffer = new byte[XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK_PAYLOAD_LENGTH];
            pack.buffersize = (UInt32)pack.buffer.Length;// (UInt32)pack.buffer.Length;
            //Debug.Log("dt: " + DateTimeOffset.Now.ToUnixTimeMilliseconds());
            string json_go = JsonUtility.ToJson(this.GetComponent<XRNetworkObject>());
            byte[] json_array = Encoding.UTF8.GetBytes(json_go);
            //Debug.Log("ID: " + pack.participant_id + "\nbuffer: " + json_go + "\ndt: " + (now - timestamp).ToString() + "\npack buffer length: " + pack.buffer.Length.ToString());
            Array.Clear(pack.buffer, 0, XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK_PAYLOAD_LENGTH);
            Array.Copy(json_array, pack.buffer, json_array.Length);
            
            byte[] pack_array = XRNetworkProtocol.GetBytes(pack,XRNetworkProtocol.ST_PARTICIPANT_UPDATE_ACK_LENGTH);
            NM.QueueMessageToService(pack_array);
            timestamp = now;
            
        }
    }
}
