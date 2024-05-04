//
// Created by jamielinux on 2/20/24.
//

#ifndef MODELS_CAMERAHOLDER_H
#define MODELS_CAMERAHOLDER_H

#include "Camera.h"
#include <initializer_list>
#include <vector>

class CameraHolder {
public:
    std::vector<std::shared_ptr<Camera>> hold;
    int currentCameraIndex = 0;
    void incrementCurrentCamera()
    {
        currentCameraIndex++;
        currentCameraIndex = currentCameraIndex % hold.size();
    }

    void addCamera(std::initializer_list<std::shared_ptr<Camera>> cams)
    {

        for (auto& cam : cams) {
            hold.push_back(cam);
        }
    }

    std::shared_ptr<Camera> getCam()
    {
        return hold[currentCameraIndex];
    }

    void processCamera(Camera::Movement move, float deltaTime, std::shared_ptr<Terrain> terrain)
    {
        hold[currentCameraIndex]->processKeyboard(move, deltaTime, true, terrain);
    }
    CameraHolder()
    {
        hold = {};
    }

    void processMouseMovement(float d, float d1)
    {
        hold[currentCameraIndex]->processMouseMovement(d, d1);
    }

    glm::vec3 camPos()
    {
        return hold[currentCameraIndex]->position;
    }
};

#endif // MODELS_CAMERAHOLDER_H
