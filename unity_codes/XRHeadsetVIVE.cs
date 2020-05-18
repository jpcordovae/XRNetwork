using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using Viveport;


public class XRHeadsetVIVE : XRHeadset
{
    float m_referenceSpee = 100.0f;
    float m_shiftHoldTime = 250.0f;
    float m_shiftMaxSpeed = 2.0f;
    float m_mouseSensitivity = 1.0f;
    Vector3 m_lastMousePosition = new Vector3(255, 255, 255);
    Quaternion m_LastMouseRotation = Quaternion.identity;

    public Text uiText;
    static Text myApiResultText;
    private float m_zoomSpeed;
    private float m_speed;
    private readonly static string appId = "4ece5a73-e67a-439e-9fa6-9c62dba12a60";
    private readonly static string apiKey = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCXpjoXQ+tHFmkcTtnDZ+tE8ieBtBw/3mXcXUwvMw5WipnoZkvGfe1myNmjarYFbrCsvsEhNxNzi7LtS7qJhte0V6SVPbXemup/r5TWrSsMAETaeyWSUEuiS7g+EQfPtI/I6UEYeqY9p34X1l/cqPzWzQV04B8XNJJwJbX8ctRtdQIDAQAB";

    private float minX = -360.0f;
    private float maxX = 360.0f;

    private float minY = -45.0f;
    private float maxY = 45.0f;

    private float sensX = 100.0f;
    private float sensY = 100.0f;

    private float rotationY = 0.0f;
    private float rotationX = 0.0f;

    void InitViveport()
    { }

    void OnViveUpdate()
    { }

    void Awake()
    { }

    // Start is called before the first frame update
    override public void Start()
    {
        Api.Init(InitCallback,appId);
    }

    // Update is called once per frame
    override public void Update()
    {
       
    }

    public static void OnDestroyCallback(int nResult)
    { 
        // delete all participants and close service
    }

    void OnDestroy()
    {
        Api.Shutdown(OnDestroyCallback);
    }

    class IAPListener : IAPurchase.IAPurchaseListener
    {
        public override void OnSuccess(string pchCurrencyName)
        {
            Action action = () => { myApiResultText.text = string.Format("The Currency is: {0}", pchCurrencyName); };
            MainThreadDispatcher.Instance().Enqueue(action);
        }
    }

    void InitCallback(int errorCode)
    {
        if (errorCode == 0)
        {
            //IAPurchase.IsReady(new IAPListener(), apiKey);
        }
    }



}
