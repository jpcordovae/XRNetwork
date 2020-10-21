using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Security.Cryptography;
using UnityEngine;

[Serializable]
public class XRNetworkObject : MonoBehaviour
{
    [DataMember] public Vector3 m_position;
    [DataMember] public Quaternion m_rotation;
    [DataMember] public string m_name;
    [DataMember] public UInt64 m_id;
    [DataMember] public string m_id_hex;
    [DataMember] public string m_prefab_path;
    [DataMember] public bool m_auto_update;

    public void SetID(UInt64 id)
    {
        m_id = id;
        m_id_hex = id.ToString("X");
        //gameObject.tag = id.ToString("X");
    }
    public void SetPlayerName(string _name)
    {
        m_name = _name;
    }

    UInt64 GetIDFromTag()
    {
        return Convert.ToUInt64(tag);
    }

    public enum EN_XRN_ObjectType : UInt16 {
        XRN_OT_NONE,
        XRN_OT_DEVICE,
        XRN_OT_WINDOW,
        XRN_OT_BUTTON,
        XRN_OT_PLAYER,
        XRN_OT_MARKER
    };

    public EN_XRN_ObjectType objectType;
    
    /*struct in networked message */
    
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ST_XRN_OBJECT_DATA {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public float[] position;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public float[] rotation;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public float[] speed;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public float[] acceleration;
    };

    private ST_XRN_OBJECT_DATA m_st_xrn_object_data;

    Vector3 m_speed;
    Vector3 m_acceleration;

    private void Awake()
    {
        // give an ID to the object
        /*byte[] u64btmp = new Byte[8];
        RNGCryptoServiceProvider rng = new RNGCryptoServiceProvider();
        rng.GetBytes(u64btmp);
        ID = BitConverter.ToUInt64(u64btmp,0);
        SetID(ID);*/
        // set up some variables
        objectType = EN_XRN_ObjectType.XRN_OT_NONE;

        // add to the devices list
        //GameObject GO = GameObject.Find("[CLIENT]");
        
    }

    public string SaveToString()
    {
        return JsonUtility.ToJson(this);
    }

    private void OnDestroy()
    {
        // remove from devices list
    }

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        string JSONMSG = JsonUtility.ToJson(this);
        /*
         * transform.position = new Vector3(m_st_xrn_object_data.position[0], m_st_xrn_object_data.position[1], m_st_xrn_object_data.position[2]);
        transform.rotation = new Quaternion(m_st_xrn_object_data.rotation[0], m_st_xrn_object_data.rotation[1], m_st_xrn_object_data.rotation[2], m_st_xrn_object_data.rotation[4]);
        m_speed = new Vector3(m_st_xrn_object_data.speed[0], m_st_xrn_object_data.speed[1], m_st_xrn_object_data.speed[2]);
        m_acceleration = new Vector3(m_st_xrn_object_data.acceleration[0], m_st_xrn_object_data.acceleration[1], m_st_xrn_object_data.acceleration[2]);
        */
    }

    public virtual void UpdateFromService()
    {
        transform.position = m_position;
        transform.rotation = m_rotation;
    }
}
