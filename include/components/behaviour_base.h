#pragma once

class Entity;

struct BehaviourBase {
    Entity* entity = nullptr;  // set when attached; the entity outlives it
    bool active = true;
};
