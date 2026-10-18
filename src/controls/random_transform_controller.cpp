#include "controls/random_transform_controller.h"

void RandomTransformController::processSignals(std::vector<RandomTransform>& transforms) {
    for (RandomTransform& transform : transforms) {
        transform.tick();
    }
}
