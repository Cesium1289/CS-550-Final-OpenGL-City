#ifndef CAMERA_H
#define CAMERA_H

#include "glut.h"
#include <math.h>

class Camera {
public:
    // Camera position, front and up vectors
    float position[3];
    float front[3];
    float up[3];

    // Movement speed and mouse sensitivity
    float speed;
    float sensitivity;

    // Mouse control variables
    float lastX, lastY;
    float yaw, pitch;

    // Constructor
    Camera(float startX, float startY, float startZ, float speed = 15.0f, float sensitivity = 0.5f)
        : speed(speed), sensitivity(sensitivity), yaw(-90.0f), pitch(0.0f), lastX(400), lastY(300) {
        position[0] = startX;
        position[1] = startY;
        position[2] = startZ;

        up[0] = 0.0f;
        up[1] = 1.0f;
        up[2] = 0.0f;

        // Initial front vector (looking along the negative Z axis)
        front[0] = 0.0f;
        front[1] = 0.0f;
        front[2] = -1.0f;
    }

    void move(float deltaTime, bool moveForward, bool moveBackward, bool strafeLeft, bool strafeRight, bool moveUp, bool moveDown) {
        // Move forward/backward along the front vector
        if (moveForward) {
            position[0] += front[0] * speed * deltaTime;
            position[1] += front[1] * speed * deltaTime;
            position[2] += front[2] * speed * deltaTime;
        }
        if (moveBackward) {
            position[0] -= front[0] * speed * deltaTime;
            position[1] -= front[1] * speed * deltaTime;
            position[2] -= front[2] * speed * deltaTime;
        }

        // Strafe left/right by calculating strafe vector perpendicular to front and up
        if (strafeLeft || strafeRight) {
            float strafe[3] = { up[1] * front[2] - up[2] * front[1],
                               up[2] * front[0] - up[0] * front[2],
                               up[0] * front[1] - up[1] * front[0] };

            if (strafeLeft) {
                position[0] -= strafe[0] * speed * deltaTime;
                position[1] -= strafe[1] * speed * deltaTime;
                position[2] -= strafe[2] * speed * deltaTime;
            }
            if (strafeRight) {
                position[0] += strafe[0] * speed * deltaTime;
                position[1] += strafe[1] * speed * deltaTime;
                position[2] += strafe[2] * speed * deltaTime;
            }
        }

        // Move up/down along the Y axis
        if (moveUp) {
            position[1] += speed * deltaTime;
        }
        if (moveDown) {
            position[1] -= speed * deltaTime;
        }
    }

    void lookAround(int x, int y) {
        // Calculate the offset
        float xOffset = x - lastX;
        float yOffset = lastY - y; 
        lastX = x;
        lastY = y;

        // Apply sensitivity to the mouse movement
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        // Update yaw and pitch based on mouse movement
        yaw += xOffset;
        pitch += yOffset;

        // Clamp the pitch to avoid gimbal lock
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // Calculate the new direction from yaw and pitch
        front[0] = cosf(yaw * (3.14 / 180.0f)) * cosf(pitch * (3.14 / 180.0f));
        front[1] = sinf(pitch * (3.14 / 180.0f));
        front[2] = sinf(yaw * (3.14 / 180.0f)) * cosf(pitch * (3.14 / 180.0f));

        // Normalize front vector
        float length = sqrt(front[0] * front[0] + front[1] * front[1] + front[2] * front[2]);
        front[0] /= length;
        front[1] /= length;
        front[2] /= length;
    }

    void rotate(float angle) {
        yaw += angle;  

        if (yaw > 360.0f) yaw -= 360.0f;  // Ensure yaw stays within [0, 360]

        // Recalculate the front vector based on the new yaw angle
        front[0] = cosf(yaw * (3.14 / 180.0f)); 
        front[1] = 0.0f;  
        front[2] = sinf(yaw * (3.14 / 180.0f));  

        // Normalize the front vector to avoid scaling issues
        float length = sqrt(front[0] * front[0] + front[1] * front[1] + front[2] * front[2]);
        front[0] /= length;
        front[1] /= length;
        front[2] /= length;
    }

    void updateViewMatrix() {
        // Calculate the look-at target based on the front vector
        float lookAtTarget[3] = { position[0] + front[0],
                                  position[1] + front[1],
                                  position[2] + front[2] };

        // Set the view matrix with gluLookAt
        gluLookAt(position[0], position[1], position[2],  // Camera position
            lookAtTarget[0], lookAtTarget[1], lookAtTarget[2], // Look-at target
            up[0], up[1], up[2]);  // Up vector
    }
};
#endif