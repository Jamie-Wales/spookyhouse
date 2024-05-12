
#include "BroadCollision.h"
#include "Camera.h"
#include "Model.h"
#include "Object.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

BoundingBox constructCameraBoundingBox(glm::vec3 cameraPosition)
{
    BoundingBox output = BoundingBox();
    output.min = cameraPosition - 5.0f;
    output.max = cameraPosition + 5.0f;
    return output;
}

struct EndPoint {
    int mData;
    float mValue;

    EndPoint(int data, float value)
        : mData(data)
        , mValue(value)
    {
    }

    EndPoint(const EndPoint& end)
    {
        this->mData = end.mData;
        this->mValue = end.mValue;
    }

    [[nodiscard]] bool isMin() const { return (mData & 1) == 0; }

    [[nodiscard]] bool isMax() const { return !isMin(); }
};

struct sweepObject {
    int id;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Camera> camera;

    sweepObject() = default;
    sweepObject(int id, std::shared_ptr<Mesh> mesh, std::shared_ptr<Camera> camera)
        : id(id)
        , mesh(mesh)
        , camera(camera) {};
};

struct Pair {
    int a;
    int b;
    char axis;
};

class SweepAndPrune {
public:
    std::vector<EndPoint> xEndPoints;
    std::vector<EndPoint> yEndPoints;
    std::vector<EndPoint> zEndPoints;
    int id = 0;
    std::unordered_map<int, std::vector<std::shared_ptr<sweepObject>>> modelToSweep;
    std::unordered_map<int, std::shared_ptr<physics::Object>> modToId;
    std::unordered_map<int, std::shared_ptr<sweepObject>> sweepMap;
    std::map<std::pair<int, int>, std::set<char>> collisionMap;
    bool checked = false;

    void SortEndPoints()
    {
        std::sort(xEndPoints.begin(), xEndPoints.end(), [](const EndPoint& a, const EndPoint& b) {
            return a.mValue < b.mValue;
        });

        std::sort(yEndPoints.begin(), yEndPoints.end(), [](const EndPoint& a, const EndPoint& b) {
            return a.mValue < b.mValue;
        });

        std::sort(zEndPoints.begin(), zEndPoints.end(), [](const EndPoint& a, const EndPoint& b) {
            return a.mValue < b.mValue;
        });
    }


    void removeModel(std::shared_ptr<Model>& model)
    {
        auto& vec = modelToSweep[model->id];
        for (auto& sweep : vec) {
            BoundingBox& bb = *sweep->mesh->boundingbox;
            xEndPoints.erase(std::remove_if(xEndPoints.begin(), xEndPoints.end(), [sweep](EndPoint& ep) {
                return (ep.mData >> 1) == sweep->id;
            }),
                xEndPoints.end());
            yEndPoints.erase(std::remove_if(yEndPoints.begin(), yEndPoints.end(), [sweep](EndPoint& ep) {
                return (ep.mData >> 1) == sweep->id;
            }),
                yEndPoints.end());
            zEndPoints.erase(std::remove_if(zEndPoints.begin(), zEndPoints.end(), [sweep](EndPoint& ep) {
                return (ep.mData >> 1) == sweep->id;
            }),
                zEndPoints.end());
            sweepMap.erase(sweep->id);
        }
        modelToSweep.erase(model->id);
    }

    int AddObject(std::shared_ptr<physics::Object>& objectModel)
    {
        for (auto& mesh : objectModel->model->meshes) {

            modToId[objectModel->id] = objectModel;
            BoundingBox& box = *mesh.boundingbox;
            std::shared_ptr<sweepObject> so = std::make_shared<sweepObject>( id, std::make_shared<Mesh>(mesh), nullptr );
            sweepMap[id] = so;
            modToId[id] =objectModel;
            modelToSweep[objectModel->id].push_back(so);
            xEndPoints.emplace_back((id << 1), box.min.x);
            xEndPoints.emplace_back((id << 1) | 1, box.max.x);
            yEndPoints.emplace_back((id << 1), box.min.y);
            yEndPoints.emplace_back((id << 1) | 1, box.max.y);
            zEndPoints.emplace_back((id << 1), box.min.z);
            zEndPoints.emplace_back((id << 1) | 1, box.max.z);
            sweepEndpoints(xEndPoints, 'x', id, box);
            sweepEndpoints(yEndPoints, 'y', id, box);
            sweepEndpoints(zEndPoints, 'z', id, box);
            id++;
        }
        return objectModel->id;
    }

    float getAxisValue(glm::vec3& point, char axis)
    {
        switch (axis) {
        case 'x':
            return point.x;
        case 'y':
            return point.y;
        case 'z':
            return point.z;
        default:
            return 0;
        }
    }

    int findIndex(std::vector<EndPoint>& endPoints, int objectId, bool isMin)
    {
        for (int i = 0; i < endPoints.size(); ++i) {
            if ((endPoints[i].mData >> 1) == objectId && endPoints[i].isMin() == isMin) {
                return i;
            }
        }
        return -1;
    }

    void addPair(int a, int b, char axis)
    {
        std::pair<int, int> objPair = std::minmax(a, b);
        collisionMap[objPair].insert(axis);
    }

    void removePair(int a, int b, char axis)
    {
        std::pair<int, int> objPair = std::minmax(a, b);
        collisionMap[objPair].erase(axis);
    }

    void managePairs(std::vector<EndPoint>& endpoints, int currentIndex, int adjacentIndex, char axis, int direction)
    {
        EndPoint& current = endpoints[currentIndex];
        EndPoint& adjacent = endpoints[adjacentIndex];

        int currentId = current.mData >> 1;
        int adjacentId = adjacent.mData >> 1;

        if (current.isMin()) {
            if (direction == 1) { // Moving right
                if (adjacent.isMax() && currentId != adjacentId) {
                    removePair(currentId, adjacentId, axis);
                }
            } else {
                if (adjacent.isMax() && currentId != adjacentId) {
                    addPair(currentId, adjacentId, axis);
                }
            }
        } else {
            if (direction == 1) {
                if (adjacent.isMin() && currentId != adjacentId) {
                    addPair(currentId, adjacentId, axis);
                }
            } else {
                if (adjacent.isMin() && currentId != adjacentId) {
                    removePair(currentId, adjacentId, axis);
                }
            }
        }
    }

    void look(std::vector<EndPoint>& endpoints, int index, char axis, int direction)
    {
        while (index + direction >= 0 && index + direction < endpoints.size() && ((direction == 1 && endpoints[index].mValue > endpoints[index + direction].mValue) || (direction == -1 && endpoints[index].mValue < endpoints[index + direction].mValue))) {
            managePairs(endpoints, index, index + direction, axis, direction);
            std::swap(endpoints[index], endpoints[index + direction]);
            index += direction;
        }
    }

    void updateEndpoint(std::vector<EndPoint>& endpoints, int index, bool isMin, BoundingBox box, char axis)
    {

        EndPoint& current = endpoints[index];
        float newValue = getAxisValue(isMin ? box.min : box.max, axis);
        int direction = (newValue > current.mValue) ? 1 : (newValue < current.mValue) ? -1
                                                                                      : 0;
        current.mValue = newValue;

        if (direction == 0) {
            look(endpoints, index, axis, -1);
            look(endpoints, index, axis, 1);
        } else {
            look(endpoints, index, axis, direction);
        }
    }

    void sweepEndpoints(std::vector<EndPoint>& endPoints, const char axis, int objectId, BoundingBox box)
    {
        int minIndex = -1;
        int maxIndex = -1;

        minIndex = findIndex(endPoints, objectId, true);
        maxIndex = findIndex(endPoints, objectId, false);

        if (minIndex == -1 || maxIndex == -1) {
            std::cerr << "Error: Indices for object not found correctly.\n";
            return;
        }

        updateEndpoint(endPoints, minIndex, true, box, axis);
        updateEndpoint(endPoints, maxIndex, false, box, axis);
    }

        void updateObject(std::shared_ptr<physics::Object> camera) {
        if (!camera->isCamera) {
            auto& vec = modelToSweep[camera->id];
            for (auto& sweep : vec) {
                /*std::cout << "debug info after update" << sweep->mesh->boundingbox.min.x << " "
                          << sweep->mesh->boundingbox.min.y << " " << sweep->mesh->boundingbox.min.z << " "
                          << sweep->mesh->boundingbox.max.x << " " << sweep->mesh->boundingbox.max.y << " "
                          << sweep->mesh->boundingbox.max.z << std::endl;
                */
                sweepEndpoints(xEndPoints, 'x', sweep->id, *sweep->mesh->boundingbox);
                sweepEndpoints(yEndPoints, 'y', sweep->id, *sweep->mesh->boundingbox);
                sweepEndpoints(zEndPoints, 'z', sweep->id, *sweep->mesh->boundingbox);
            }
        } else{
            if (camera->position == glm::vec3(0.0f))
                return;

            auto& vec = modelToSweep[camera->id];

            for (auto& sweep : vec) {
                BoundingBox& bb =   *sweep->camera->boundingBox;
                sweepEndpoints(xEndPoints, 'x', sweep->id, bb);
                sweepEndpoints(yEndPoints, 'y', sweep->id, bb);
                sweepEndpoints(zEndPoints, 'z', sweep->id, bb);
            }
        }
    }

    int addCamera(std::shared_ptr<physics::Object> camera)
    {
        BoundingBox box = *camera->boundingBox;
        std::shared_ptr<sweepObject> so = std::make_shared<sweepObject>(id, nullptr, camera->camera);
        sweepMap[id] = so;
        modToId[id] = camera;
        modelToSweep[camera->id].push_back(so);
        xEndPoints.emplace_back((id << 1), box.min.x);
        xEndPoints.emplace_back((id << 1) | 1, box.max.x);
        yEndPoints.emplace_back((id << 1), box.min.y);
        yEndPoints.emplace_back((id << 1) | 1, box.max.y);
        zEndPoints.emplace_back((id << 1), box.min.z);
        zEndPoints.emplace_back((id << 1) | 1, box.max.z);
        sweepEndpoints(xEndPoints, 'x', id, box);
        sweepEndpoints(yEndPoints, 'y', id, box);
        sweepEndpoints(zEndPoints, 'z', id, box);
        id++;
        return id - 1;
    }

    void printTrueCollisions(std::vector<BroadCollision>& trueCol) const
    {
        for (auto& coll : trueCol) {
            std::cout << "TRUE COLLISION \n";
            std::cout << "First Model: " << coll.modelIdA << "\n";
            std::cout << "Second Model: " << coll.modelIdB << "\n";
        }
    }

    std::vector<BroadCollision> getTrueCollisions() {
        std::vector<BroadCollision> trueCollisions;
        for (const auto& entry : collisionMap) {
            int firstId = entry.first.first;
            int secondId = entry.first.second;
            auto so = modToId[firstId];
            auto so2 = modToId[secondId];
            if (entry.second.size() == 3 && so->id != so2->id)
                trueCollisions.emplace_back(so->id, so2->id);
        }

        return trueCollisions;
    }
};
