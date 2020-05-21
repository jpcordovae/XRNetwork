using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class XRHeadset : MonoBehaviour
{
    private delegate void OnPollingCallbackDelegate();
    private delegate void OnInitCallbackDelegate();
    private delegate void OnStopCallbackDelegate();
    private delegate void OnQueryCallbackDelegate();
    private delegate void OnUpdateCallbackDelegate();

    public delegate void OnConnectCallback(bool connected);
    public delegate void OnDisconnectCallback(bool connected);

    public enum EN_DEVICE_STATUS { CONNECTED, DISCONNECTED }
    public EN_DEVICE_STATUS device_status;

    [SerializeField]
    public struct ST_PARTICIAPNT_ROT_STATUS {
        public Transform body;
        public Transform lshoulder;
        public Transform rshoulrder;
        public Transform lhip;
        public Transform rhip;
        public Transform lhelbow;
        public Transform rhelbow;
        public Transform lknee;
        public Transform rknee;
        public Transform lhand;
        public Transform rhand;
    };

    private Transform m_HeadSetTransform;

    public virtual void Awake()
    { }

    // Start is called before the first frame update
    public virtual void Start()
    { }

    // Update is called once per frame
    public virtual void Update()
    { }

    public void OnTrackingData()
    { }

    public virtual void ConnectDevice(OnConnectCallback occ)
    { }

    public virtual void DisconnectDevice(OnDisconnectCallback odc)
    { }

}
