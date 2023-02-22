#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
using namespace std;
const int pi = 3.141592653589793;
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         =  0.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.03f;
const float ZOOM        =  45.0f; //(FOV)


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp=glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 DPosition;
    glm::vec3 LastPosition;
    glm::quat orientation;
    glm::quat Lastq;
    glm::quat Startq;
    glm::quat Dq;

    float stairs=100;
    // Euler Angles
    float LastYaw;
    float LastPitch;
    float DYaw;
    float DPitch;

    float Yaw;
    float Pitch;
    float Roll=0.0f;

    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        LastPosition=Position = position;
        WorldUp = up;
       
        LastYaw=Yaw = yaw;
        LastPitch=Pitch = pitch;
        Lastq = glm::quat(glm::vec3(glm::radians(Pitch), glm::radians(Yaw), glm::radians(Roll)));
        updataeorientation();
        updateCameraVectors();
    }

    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw=YAW, float pitch=PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updataeorientation();
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        /*updataeorientation();
        updateCameraVectors();*/
        //// TODO : fill in
        /*glm::mat4 view = glm::lookAt(Position, Position + Front, Up);
        return view;*/
        glm::quat reverseorient = glm::conjugate(orientation);
        glm::mat4 rot = glm::mat4_cast(reverseorient);
        glm::mat4 translation = glm::translate(glm::mat4(1.0), -Position);

        return rot * translation;
       
    }


    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float camSpeed = static_cast<float>(SPEED * deltaTime);
        // TODO : fill in
        if (direction == FORWARD) {
            Position += camSpeed * Front;
        }
        if (direction == BACKWARD) {
            Position -= camSpeed * Front;
        }
        if (direction == LEFT) {
            Position -= glm::normalize(glm::cross(Front, Up)) * camSpeed;
        }
        if (direction == RIGHT) {
            Position += glm::normalize(glm::cross(Front, Up)) * camSpeed;
        }
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        // TODO : fill in
        // pitch between -89 and 89 degree
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        Yaw += xoffset;
        Pitch += yoffset;

        if (Yaw > 180.0f) {
            Yaw = -360.0f + Yaw;
        }
        else if (Yaw < -180.0f) {
            Yaw = 360.0f + Yaw;
        }
        if (constrainPitch) {
            if (Pitch > 89.0f) {
                Pitch = 89.0f;
            }
            if (Pitch < -89.0f) {
                Pitch = -89.0f;
            }
        }
        updataeorientation();
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= yoffset;
        if (Zoom < 1.0f) {
            Zoom = 1.0f;
        }
        if (Zoom > 45.0f) {
            Zoom = 45.0f;
        }
            
    }
   
    void Qclick() {
        LastPosition = Position;
        LastYaw = Yaw;
        LastPitch = Pitch;
        Lastq = glm::quat(glm::vec3(glm::radians(Pitch), glm::radians(Yaw), glm::radians(Roll)));
    }

    void EClick() {
        DPosition=(LastPosition - Position)/stairs;
        DYaw = LastYaw - Yaw;
        if ((DYaw) > 180.0f) {
            DYaw = -360.0f + DYaw;
        }
        else if (DYaw < -180.0f) {
            DYaw = 360 + DYaw;
        }
        

        DYaw = DYaw / stairs;
       

        DPitch = (LastPitch - Pitch) / stairs;
    }

    void Echange() {
        Position += DPosition;
        Yaw += DYaw;
       
        if (Yaw > 180.0f) {
            Yaw = -360.0f + Yaw;
        }
        else if (Yaw < -180.0f) {
            Yaw = 360.0f + Yaw;
        }

        Pitch += DPitch;

       
        updataeorientation();
        updateCameraVectors();
    }

    void RClick() {
        DPosition = (LastPosition - Position) / stairs;
        Startq = glm::quat(glm::vec3(glm::radians(Pitch), glm::radians(Yaw), glm::radians(Roll)));
    }
    void Rchange(float t) {
        Position += DPosition;
        /// <summary>
        /// 쿼터네리 구현하기 
        /// 
        Dq = (glm::slerp(Lastq, Startq, t));
        orientation = Dq;

        Pitch = atan2(2.0 * (Dq.y * Dq.z + Dq.w * Dq.x), Dq.w * Dq.w - Dq.x * Dq.x - Dq.y * Dq.y + Dq.z * Dq.z)*180/pi;
        Yaw = asin(-2.0 * (Dq.x * Dq.z - Dq.w * Dq.y))*180/pi;
        Roll = atan2(2.0 * (Dq.x * Dq.y + Dq.w * Dq.z), Dq.w * Dq.w + Dq.x * Dq.x - Dq.y * Dq.y - Dq.z * Dq.z)*180/pi;

      
       
        orientation = Dq;
        updateCameraVectors();
       
    }
    void goback() {
        Yaw = LastYaw;
        Pitch = LastPitch;
        Roll = 0;
        updataeorientation();
        updateCameraVectors();

    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updataeorientation() {
       
        orientation = glm::quat(glm::vec3(glm::radians(Pitch), glm::radians(Yaw), glm::radians(Roll)));
    }


    void updateCameraVectors()
    {
        
        
        glm::quat qF = orientation * glm::quat(0, 0, 0.0, -1.0f) * glm::conjugate(orientation);
        glm::vec3 front = { qF.x,qF.y,qF.z };
        Front = glm::normalize(front);
        
        /*glm::quat qr = orientation * glm::quat(0, 1.0f, 0.0, 0.0f) * glm::conjugate(orientation);
        glm::vec3 right = { qr.x,qr.y,qr.z };
        Right = glm::normalize(right);*/



        glm::vec4 tmp = glm::vec4(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f)), 1.0f);
        glm::mat4 mod= glm::rotate(glm::mat4(1.0f), glm::radians(Roll), glm::vec3(0.0f, 0.0f, 1.0f));
        tmp = mod * tmp;
        Right = glm::normalize(glm::vec3(tmp.x,tmp.y,tmp.z));
        

        Up = glm::normalize(glm::cross(Right, Front));
    }

   
};
#endif
