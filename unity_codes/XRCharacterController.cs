using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class XRCharacterController : MonoBehaviour
{
    CharacterController m_characterController;
    
    public float m_speed = 5f; // m/s
    public float m_gravity = 9.8f; // m/s^2
    public float m_jumpSpeed = 8.0f;
    private Vector3 m_moveDirection = Vector3.zero;
    // Start is called before the first frame update
    void Start()
    {
        m_characterController = GetComponent<CharacterController>();
    }

    // Update is called once per frame
    void Update()
    {
        //if (m_characterController.isGrounded)
        {
            m_moveDirection = new Vector3(Input.GetAxis("Horizontal"),0.0f,Input.GetAxis("Vertical"));
            m_moveDirection = transform.TransformDirection(m_moveDirection);
            m_moveDirection *= m_speed;

            if (Input.GetButton("Jump"))
            {
                m_moveDirection.y = m_jumpSpeed;
            }
        }

        m_moveDirection.y -= m_gravity * Time.deltaTime;
        m_characterController.Move(m_moveDirection*Time.deltaTime);
    }
}
