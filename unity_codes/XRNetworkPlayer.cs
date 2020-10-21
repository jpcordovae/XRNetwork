using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Text;
using System.Xml;
using UnityEditor;
using UnityEngine;

public class XRNetworkPlayer : XRNetworkObject
{
    public List<XRNetworkDevice> m_devices = new List<XRNetworkDevice>();

    [SerializeField] public GameObject m_prefab;
    public XRNetworkPlayer()
    {
        objectType = EN_XRN_ObjectType.XRN_OT_PLAYER;
        // m_prefab = PrefabUtility.InstantiatePrefab(;
    }

    // Start is called before the first frame update
    void Start()
    {
        // fill the list of devices for the client
        //m_prefab_path = AssetDatabase.GetAssetPath(m_prefab);
    }

    // Update is called once per frame
    void Update()
    {

    }

}
