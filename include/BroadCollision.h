#ifndef INCLUDE_BROADCOLLISION_H_
#define INCLUDE_BROADCOLLISION_H_
#include "Camera.h"
#include "Mesh.h"
struct BroadCollision {
public:
    int modelIdA;
    int modelIdB;
    std::shared_ptr<Mesh> firstMesh;
    std::shared_ptr<Mesh> secondMesh;
    std::shared_ptr<Camera> camera;
    BroadCollision(int modelIdA, int modelIdB)
        : modelIdA { modelIdA }
        , modelIdB { modelIdB } {};
};

#endif // INCLUDE_INCLUDE_BROADCOLLISION_H_
