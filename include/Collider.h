
#ifndef INCLUDE_COLLIDER_H_
#define INCLUDE_COLLIDER_H_
#include "SweepPrune.h"
#include <initializer_list>

class Collider {
public:

    SweepAndPrune sweepandprune = SweepAndPrune();
    std::vector<Pair> broadphasePairs = {};
    void addModel(std::shared_ptr<Model>& model) {
        sweepandprune.addModel(model);
    }

    void collide(std::shared_ptr<Model>& model)

    {
        sweepandprune.UpdateObject(model);
        broadphasePairs = sweepandprune.getTrueCollisions();
    }
};

#endif
