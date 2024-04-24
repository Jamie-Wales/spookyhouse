#ifndef INCLUDE_BROADCOLLISION_H_
#define INCLUDE_BROADCOLLISION_H_
#include "Mesh.h"
struct BroadCollision {
public:
    int modelIdA;
    int modelIdB;
    std::shared_ptr<Mesh> firstMesh;
    std::shared_ptr<Mesh> secondMesh;

    BroadCollision(int modelIdA, int modelIdB, std::shared_ptr<Mesh> firstMesh, std::shared_ptr<Mesh> secondMesh)
        : modelIdA { modelIdA }
        , modelIdB { modelIdB }
        , firstMesh { firstMesh }
        , secondMesh { secondMesh } {};
};

#endif // INCLUDE_INCLUDE_BROADCOLLISION_H_
