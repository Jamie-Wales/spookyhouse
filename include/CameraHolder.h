//
// Created by jamielinux on 2/20/24.
//

#ifndef MODELS_CAMERAHOLDER_H
#define MODELS_CAMERAHOLDER_H

#include "Camera.h"
#include <vector>

class CameraHolder {
public:
    std::vector<Camera> hold;
    int currentCameraIndex = 0;
    void incrementCurrentCamera()
    {
        currentCameraIndex++;
        currentCameraIndex = currentCameraIndex % hold.size();
    }

    void addCamera(Camera& camera)
    {
        hold.push_back(camera);
    }

    glm::mat4 getCam()
    {
        return hold[currentCameraIndex].getCameraView();
    }

    void processCamera(Camera::Movement move, float deltaTime, Terrain& terrain)
    {
        hold[currentCameraIndex].processKeyboard(move, deltaTime, true, terrain);
    }
    CameraHolder()
    {
        hold = {};
    }

    void processMouseMovement(float d, float d1)
    {
        hold[currentCameraIndex].processMouseMovement(d, d1);
    }

    glm::vec3 camPos()
    {
        return hold[currentCameraIndex].position;
    }
};

#endif // MODELS_CAMERAHOLDER_H
