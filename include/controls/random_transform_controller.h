#pragma once

#include "controls.h"
#include "random_transform.h"

#include <vector>

// Advances every RandomTransform once per frame. Driven by the frame, not by
// input, so it's a Controller but not a Slot.
class RandomTransformController : public Controller<std::vector<RandomTransform>> {
public:
    void processSignals(std::vector<RandomTransform>& transforms) override;
};
