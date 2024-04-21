
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
    std::vector<Pair> pairs;
    struct collided {
        bool x = false;
        bool y = false;
        bool z = false;
    };

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

    void addModel(std::shared_ptr<Model> objectModel)
    {
        AddObject(objectModel->id, objectModel);
        modelList[objectModel] = objectModel->id;
    }
    int AddObject(int id, std::shared_ptr<Model>& objectModel)
    {
        int objectId = id;
        BoundingBox box = objectModel->boundingbox;
        objects.push_back({ objectModel, objectId });
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

    void sweepEndpoints(std::vector<EndPoint>& xEndPoints, const char axis, int objectId, BoundingBox box)
    {
        int minIndex = -1;
        int maxIndex = -1;

        for (int i = 0; i < xEndPoints.size(); ++i) {
            if ((xEndPoints[i].mData >> 1) == objectId) {
                if (xEndPoints[i].isMin()) {
                    minIndex = i;
                } else {
                    maxIndex = i;
                }
            }
        }

        if (minIndex == -1 || maxIndex == -1) {
            std::cerr << "Error: Indices for object not found correctly.\n";
            return;
        }

        int axisIndex = 0;
        switch (axis) {
        case 'x':
            xEndPoints[minIndex].mValue = box.min.x;
            break;
        case 'y':
            xEndPoints[minIndex].mValue = box.min.y;
            break;
        case 'z':
            xEndPoints[minIndex].mValue = box.min.z;

            break;
        }
        while (minIndex > 0 && xEndPoints[minIndex].mValue < xEndPoints[minIndex - 1].mValue) {
            std::swap(xEndPoints[minIndex], xEndPoints[minIndex - 1]);
            minIndex--;
            if (xEndPoints[minIndex].isMin() && xEndPoints[minIndex + 1].isMax() && (xEndPoints[minIndex].mData >> 1) != (xEndPoints[minIndex + 1].mData >> 1)) {
                pairs.push_back({ xEndPoints[minIndex].mData >> 1, xEndPoints[minIndex + 1].mData >> 1, axis });
            }
        }
        while (minIndex < xEndPoints.size() - 1 && xEndPoints[minIndex].mValue > xEndPoints[minIndex + 1].mValue) {
            std::swap(xEndPoints[minIndex], xEndPoints[minIndex + 1]);
            minIndex++;
            if (xEndPoints[minIndex].isMin() && xEndPoints[minIndex - 1].isMax() && (xEndPoints[minIndex].mData >> 1) != (xEndPoints[minIndex - 1].mData >> 1)) {
                pairs.push_back({ xEndPoints[minIndex].mData >> 1, xEndPoints[minIndex - 1].mData >> 1, axis });
            }
        }

        // Update the max endpoint
        switch (axis) {
        case 'x':
            xEndPoints[maxIndex].mValue = box.max.x;
            break;
        case 'y':
            xEndPoints[maxIndex].mValue = box.max.y;
            break;
        case 'z':
            xEndPoints[maxIndex].mValue = box.max.z;
            break;
        }
        while (maxIndex > 0 && xEndPoints[maxIndex].mValue < xEndPoints[maxIndex - 1].mValue) {
            std::swap(xEndPoints[maxIndex], xEndPoints[maxIndex - 1]);
            maxIndex--;
            if (xEndPoints[maxIndex].isMax() && xEndPoints[maxIndex + 1].isMin() && (xEndPoints[maxIndex].mData >> 1) != (xEndPoints[maxIndex + 1].mData >> 1)) {
                pairs.push_back({ xEndPoints[maxIndex].mData >> 1, xEndPoints[maxIndex + 1].mData >> 1, axis });
            }
        }
        while (maxIndex < xEndPoints.size() - 1 && xEndPoints[maxIndex].mValue > xEndPoints[maxIndex + 1].mValue) {
            std::swap(xEndPoints[maxIndex], xEndPoints[maxIndex + 1]);
            maxIndex++;
            if (xEndPoints[maxIndex].isMax() && xEndPoints[maxIndex - 1].isMin() && (xEndPoints[maxIndex].mData >> 1) != (xEndPoints[maxIndex - 1].mData >> 1)) {
                pairs.push_back({ xEndPoints[maxIndex].mData >> 1, xEndPoints[maxIndex - 1].mData >> 1, axis });
            }
        }
    }
    void UpdateObject(std::shared_ptr<Model> model)
    {
        int objectId = modelList[model];
        BoundingBox box = model->boundingbox;
        sweepEndpoints(xEndPoints, 'x', objectId, box);
        sweepEndpoints(yEndPoints, 'y', objectId, box);
        sweepEndpoints(zEndPoints, 'z', objectId, box);
    }

    void printTrueCollisions()
    {
        std::map<std::pair<int, int>, std::set<char>> collisionMap;
        for (const auto& pair : pairs) {
            std::pair<int, int> objPair = std::minmax(pair.a, pair.b); // Normalize order
            collisionMap[objPair].insert(pair.axis);
        }
        for (const auto& entry : collisionMap) {
            if (entry.second.size() == 3) { // Must have 'x', 'y', 'z'
                std::cout << "Collision between " << entry.first.first << " and " << entry.first.second << "\n";
            }
        }
    }
    std::vector<Pair> getTrueCollisions()
    {
        std::map<std::pair<int, int>, std::set<char>> collisionMap;
        for (const auto& pair : pairs) {
            std::pair<int, int> objPair = std::minmax(pair.a, pair.b); // Normalize order
            collisionMap[objPair].insert(pair.axis);
        }
        std::vector<Pair> trueCollisions;
        for (const auto& entry : collisionMap) {
            if (entry.second.size() == 3) { // Must have 'x', 'y', 'z'
                trueCollisions.push_back({ entry.first.first, entry.first.second, 'x' }); // Axis here is arbitrary
            }
        }

        printTrueCollisions();
        pairs.clear();
        return trueCollisions;
    }
};
