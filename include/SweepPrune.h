#include "BroadCollision.h"
#include "Model.h"
#include "Object.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include <map>

struct EndPoint {
    int mData;
    float mValue;

    EndPoint(int data, float value)
        : mData(data), mValue(value) {}

    EndPoint(const EndPoint& end)
        : mData(end.mData), mValue(end.mValue) {}

    [[nodiscard]] bool isMin() const { return (mData & 1) == 0; }
    [[nodiscard]] bool isMax() const { return !isMin(); }
};

struct sweepObject {
    int id;
    std::shared_ptr<BoundingBox> boundingBox;

    sweepObject(int id, std::shared_ptr<BoundingBox> boundingBox)
        : id(id), boundingBox(boundingBox) {}
};

class SweepAndPrune {
public:
    std::vector<EndPoint> xEndPoints;
    std::vector<EndPoint> yEndPoints;
    std::vector<EndPoint> zEndPoints;
    int id = 0;

    std::map<std::pair<int, int>, std::set<char>> collisionMap;

    void SortEndPoints() {
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

    void removeModel(std::shared_ptr<Model>& model) {
        int objectId = model->id;

        xEndPoints.erase(std::remove_if(xEndPoints.begin(), xEndPoints.end(), [objectId](EndPoint& ep) {
            return (ep.mData >> 1) == objectId;
        }), xEndPoints.end());

        yEndPoints.erase(std::remove_if(yEndPoints.begin(), yEndPoints.end(), [objectId](EndPoint& ep) {
            return (ep.mData >> 1) == objectId;
        }), yEndPoints.end());

        zEndPoints.erase(std::remove_if(zEndPoints.begin(), zEndPoints.end(), [objectId](EndPoint& ep) {
            return (ep.mData >> 1) == objectId;
        }), zEndPoints.end());
    }

    int AddObject(std::shared_ptr<physics::Object>& objectModel) {
        auto &box = objectModel->boundingBox;


        xEndPoints.emplace_back((objectModel->id << 1), box->min.x);
        xEndPoints.emplace_back((objectModel->id << 1) | 1, box->max.x);
        yEndPoints.emplace_back((objectModel->id << 1), box->min.y);
        yEndPoints.emplace_back((objectModel->id << 1) | 1, box->max.y);
        zEndPoints.emplace_back((objectModel->id<< 1), box->min.z);
        zEndPoints.emplace_back((objectModel->id << 1) | 1, box->max.z);

        sweepEndpoints(xEndPoints, 'x', id, *box);
        sweepEndpoints(yEndPoints, 'y', id, *box);
        sweepEndpoints(zEndPoints, 'z', id, *box);

    }

    float getAxisValue(glm::vec3& point, char axis) {
        switch (axis) {
            case 'x': return point.x;
            case 'y': return point.y;
            case 'z': return point.z;
            default: return 0;
        }
    }

    int findIndex(std::vector<EndPoint>& endPoints, int objectId, bool isMin) {
        for (int i = 0; i < endPoints.size(); ++i) {
            if ((endPoints[i].mData >> 1) == objectId && endPoints[i].isMin() == isMin) {
                return i;
            }
        }
        return -1;
    }

    void addPair(int a, int b, char axis) {
        std::pair<int, int> objPair = std::minmax(a, b);
        collisionMap[objPair].insert(axis);
    }

    void removePair(int a, int b, char axis) {
        std::pair<int, int> objPair = std::minmax(a, b);
        collisionMap[objPair].erase(axis);
    }

    void managePairs(std::vector<EndPoint>& endpoints, int currentIndex, int adjacentIndex, char axis, int direction) {
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

    void look(std::vector<EndPoint>& endpoints, int index, char axis, int direction) {
        while (index + direction >= 0 && index + direction < endpoints.size() &&
               ((direction == 1 && endpoints[index].mValue > endpoints[index + direction].mValue) ||
                (direction == -1 && endpoints[index].mValue < endpoints[index + direction].mValue))) {
            managePairs(endpoints, index, index + direction, axis, direction);
            std::swap(endpoints[index], endpoints[index + direction]);
            index += direction;
        }
    }

    void updateEndpoint(std::vector<EndPoint>& endpoints, int index, bool isMin, BoundingBox box, char axis) {
        EndPoint& current = endpoints[index];
        float newValue = getAxisValue(isMin ? box.min : box.max, axis);
        int direction = (newValue > current.mValue) ? 1 : (newValue < current.mValue) ? -1 : 0;
        current.mValue = newValue;

        if (direction == 0) {
            look(endpoints, index, axis, -1);
            look(endpoints, index, axis, 1);
        } else {
            look(endpoints, index, axis, direction);
        }
    }

    void sweepEndpoints(std::vector<EndPoint>& endPoints, const char axis, int objectId, BoundingBox box) {
        int minIndex = findIndex(endPoints, objectId, true);
        int maxIndex = findIndex(endPoints, objectId, false);

        if (minIndex == -1 || maxIndex == -1) {
            return;
        }

        updateEndpoint(endPoints, minIndex, true, box, axis);
        updateEndpoint(endPoints, maxIndex, false, box, axis);
    }

    void updateObject(std::shared_ptr<physics::Object> object) {
            int objectId = object->id;
            auto &box = *object->boundingBox;

            sweepEndpoints(xEndPoints, 'x', objectId, box);
            sweepEndpoints(yEndPoints, 'y', objectId, box);
            sweepEndpoints(zEndPoints, 'z', objectId, box);
    }

    void printTrueCollisions(std::vector<BroadCollision>& trueCol) const {
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
            if (entry.second.size() == 3 && firstId != secondId)
                trueCollisions.emplace_back(firstId, secondId);
        }

        return trueCollisions;
    }
};
