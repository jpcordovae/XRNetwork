using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class XRNetworkPlayer : MonoBehaviour
{
    // Start is called before the first frame update
    private UInt64 m_participant_id;
    public UInt64 participant_id { get { return m_participant_id; } set { m_participant_id = value; } }
    private Vector3 m_reference_position = Vector3.forward;
    private Quaternion m_reference_rotation = Quaternion.identity;

    private XRNetworkPlayer() { }

    public XRNetworkPlayer(UInt64 _participant_id)
    {
        m_participant_id = _participant_id;
    }

    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void OnLeaveXRService()
    { 
        
    }

    public void XRServiceUpdate(string message)
    {
        string[] spltmsg = message.Split(';');
        
    }

    // timed update
    public void LocalUpdate()
    { 
        
    }

}
