
#include "Model.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

struct EndPoint {
    int mData;
    float mValue;

    EndPoint(int data, float value)
            : mData(data), mValue(value) {
    }

    EndPoint(const EndPoint &end) {
        this->mData = end.mData;
        this->mValue = end.mValue;
    }

    [[nodiscard]] bool isMin() const { return (mData & 1) == 0; }

    [[nodiscard]] bool isMax() const { return !isMin(); }
};

struct sweepObject {
    std::shared_ptr<Model> objectModel;
    int id;
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
    std::vector<sweepObject> objects;
    int id = 0;
    std::unordered_map<std::shared_ptr<Model>, int> modelList;
    std::map<std::pair<int, int>, std::set<char>> collisionMap;
    bool checked = false;

    void SortEndPoints() {
        std::sort(xEndPoints.begin(), xEndPoints.end(), [](const EndPoint &a, const EndPoint &b) {
            return a.mValue < b.mValue;
        });

        std::sort(yEndPoints.begin(), yEndPoints.end(), [](const EndPoint &a, const EndPoint &b) {
            return a.mValue < b.mValue;
        });

        std::sort(zEndPoints.begin(), zEndPoints.end(), [](const EndPoint &a, const EndPoint &b) {
            return a.mValue < b.mValue;
        });
    }

    void addModel(std::shared_ptr<Model> objectModel) {
        AddObject(objectModel->id, objectModel);
        modelList[objectModel] = objectModel->id;
    }

    int AddObject(int id, std::shared_ptr<Model> &objectModel) {
        int objectId = id;
        BoundingBox box = objectModel->boundingbox;
        objects.push_back({objectModel, objectId});
        xEndPoints.push_back(EndPoint((objectId << 1), box.min.x));
        xEndPoints.push_back(EndPoint((objectId << 1) | 1, box.max.x));
        yEndPoints.push_back(EndPoint((objectId << 1), box.min.y));
        yEndPoints.push_back(EndPoint((objectId << 1) | 1, box.max.y));
        zEndPoints.push_back(EndPoint((objectId << 1), box.min.z));
        zEndPoints.push_back(EndPoint((objectId << 1) | 1, box.max.z));

        sweepEndpoints(xEndPoints, 'x', objectId, box);
        sweepEndpoints(yEndPoints, 'y', objectId, box);
        sweepEndpoints(zEndPoints, 'z', objectId, box);
        return objectId;
    }

    float getAxisValue(glm::vec3 &point, char axis) {
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

    int findIndex(std::vector<EndPoint> &endPoints, int objectId, bool isMin) {
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

    void managePairs(std::vector<EndPoint> &endpoints, int currentIndex, int adjacentIndex, char axis, int direction) {
        EndPoint &current = endpoints[currentIndex];
        EndPoint &adjacent = endpoints[adjacentIndex];

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

    void look(std::vector<EndPoint> &endpoints, int index, char axis, int direction) {
        while (index + direction >= 0 && index + direction < endpoints.size() &&
               ((direction == 1 && endpoints[index].mValue > endpoints[index + direction].mValue) ||
                (direction == -1 && endpoints[index].mValue < endpoints[index + direction].mValue))) {
            managePairs(endpoints, index, index + direction, axis, direction);
            std::swap(endpoints[index], endpoints[index + direction]);
            index += direction;
        }
    }

    void updateEndpoint(std::vector<EndPoint> &endpoints, int index, bool isMin, BoundingBox box, char axis) {

        EndPoint &current = endpoints[index];
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


    void sweepEndpoints(std::vector<EndPoint> &endPoints, const char axis, int objectId, BoundingBox box) {
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

    void UpdateObject(std::shared_ptr<Model> model) {
        int objectId = modelList[model];
        BoundingBox box = model->boundingbox;
        sweepEndpoints(xEndPoints, 'x', objectId, box);
        sweepEndpoints(yEndPoints, 'y', objectId, box);
        sweepEndpoints(zEndPoints, 'z', objectId, box);
    }


    std::vector<Pair> getTrueCollisions() {
        std::vector<Pair> trueCollisions;
        for (const auto &entry: collisionMap) {
            if (entry.second.size() == 3) { // Must have 'x', 'y', 'z'
                trueCollisions.push_back({entry.first.first, entry.first.second, 'x'}); // Axis here is arbitrary
            }
        }
        return trueCollisions;
    }
};
