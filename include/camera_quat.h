#ifndef CAMERA_QUAT_H
#define CAMERA_QUAT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>
#include <vector>


enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera
{
private :

void updateCamera()
{
    glm::quat y = glm::angleAxis(glm::radians(-Right_Angle), glm::vec3(0, 1, 0));
    glm::quat x = glm::angleAxis(glm::radians(Up_Angle), glm::vec3(1, 0, 0));
    Orientation = y * x;
}
public:
    // 카메라 클래스 변수
    glm::vec3 Position;
    glm::quat Orientation;
    float Right_Angle;
    float Up_Angle;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    //벡터로 초기화

    Camera(glm::vec3 cam_pos) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = cam_pos;
        Orientation = glm::quat(0,0,0,-1);
        Right_Angle = 0.f;
        Up_Angle = 0.f;
        updateCamera();
    }

    glm::mat4 GetViewMatrix()
    {
        glm::quat reverse_orientation = glm::conjugate(Orientation);
        glm::mat4 rotation_matrix = glm::mat4_cast(reverse_orientation);
        glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0), -Position);

        return rotation_matrix * translation_matrix;
    }

    void ProcessKeyboard(Camera_Movement dir, float delta_time)
    {
        float velocity = MovementSpeed * delta_time;

        glm::quat f = Orientation * glm::quat(0,0,0,-1) * glm::conjugate(Orientation);
        glm::vec3 Front = glm::vec3(f.x, f.y, f.z);
        glm::vec3 Right = glm::normalize(glm::cross(Front, glm::vec3(0,1,0)));

        if(dir == FORWARD)
            Position += Front * velocity;

        if(dir == BACKWARD)
            Position -= Front * velocity;

        if(dir == RIGHT)
            Position += Right * velocity;

        if(dir == LEFT)
            Position -= Right * velocity;
    }

    void ProcessMouseMove(float x_offset, float y_offset)
    {
        x_offset *= MouseSensitivity;
        y_offset *= MouseSensitivity;

        Right_Angle += x_offset;
        Up_Angle += y_offset;

        updateCamera();
    }

    void ProcessMouseScroll(float offset)
    {
        if(Zoom >= 1.0f && Zoom <=45.0f)  
            Zoom -= offset;

        if(Zoom <= 1.0f)
            Zoom = 1.0f;

        if(Zoom >= 45.0f)
            Zoom = 45.0f;
    }
};

#endif