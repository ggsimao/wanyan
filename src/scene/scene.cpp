#include "scene/scene.h"
#include "scene/entity.h"
#include <memory>

Scene::~Scene() {
    while (!_roots.empty()) {
        releaseSubtree(*_roots.back());
    }
}

Scene::Scene(std::vector<Skybox> skyboxes, glm::vec3 ambient)
    : skyboxes(std::move(skyboxes)), ambient(ambient) {}

void Scene::update(const FrameContext& context) {
    for (auto toStart : _toStart) {
        if (toStart.isNull()) {
            continue;
        }
        if (auto startNode = get(toStart)) {
            startNode->start(context);
        }
    }
    _toStart.clear();
    for (auto root : _roots) {
        root->update(context);
    }
    for (auto root : _roots) {
        root->lateUpdate(context);
    }
    processChanges(context);
}

EntityHandle Scene::create(EntityDescription desc) {
    // Choose a slot without claiming it yet: the last free one, or a new one
    // at the end. Nothing in the scene changes until the entity exists.
    const bool reuse = !_freeSlots.empty();
    const std::uint32_t index =
        reuse ? _freeSlots.back() : static_cast<std::uint32_t>(_slots.size());
    const std::uint32_t generation = reuse ? _slots[index].generation : 0;
    const EntityHandle handle{.index = index, .generation = generation};
    const EntityHandle parent = desc.parent;  // read before desc is moved from

    // Everything that can throw comes first. If any of it does, the scene is
    // exactly as it was: the entity (if built) is freed by its unique_ptr.
    auto entity =
        std::unique_ptr<Entity>(new Entity(*this, handle, std::move(desc)));
    _pendingChanges.reserve(_pendingChanges.size() +
                            1);  // so push_back can't throw below
    if (!reuse) {
        _slots.emplace_back();  // may throw; nothing claimed yet
    }

    // Nothing below can throw: claim the slot, store the entity, queue the
    // link.
    if (reuse) {
        _freeSlots.pop_back();
    }
    EntitySlot& slot = _slots[index];
    assert(!slot.entity && "A free slot should be empty");
    slot.entity = std::move(entity);
    _pendingChanges.push_back(LinkCreated{.entity = handle, .parent = parent});
    return handle;
}

void Scene::destroy(EntityHandle handle) {
    Destroy change{.entity = handle};
    _pendingChanges.push_back(change);
}

void Scene::setParent(EntityHandle entity, EntityHandle parent) {
    Reparent change{.entity = entity, .parent = parent};
    _pendingChanges.push_back(change);
}

Entity* Scene::get(EntityHandle handle) {
    if (handle.isNull() || handle.index >= _slots.size())
        return nullptr;
    EntitySlot& slot = _slots[handle.index];
    return slot.generation == handle.generation ? slot.entity.get() : nullptr;
}

const Entity* Scene::get(EntityHandle handle) const {
    if (handle.isNull() || handle.index >= _slots.size())
        return nullptr;
    const EntitySlot& slot = _slots[handle.index];
    return slot.generation == handle.generation ? slot.entity.get() : nullptr;
}

const std::vector<Entity*> Scene::getRoots() const { return _roots; }

const Entity* Scene::getActiveCamera() const { return _mainCamera; }

void Scene::setActiveCamera(EntityHandle handle) { _mainCamera = get(handle); }

void Scene::processChanges(const FrameContext& context) {
    // Taken out first: processing can queue new changes (an onDestroy that
    // destroys something else, say), and appending to the vector being
    // iterated would invalidate the loop.
    std::vector<StructuralChange> changes = std::move(_pendingChanges);
    _pendingChanges.clear();

    for (const StructuralChange& change : changes) {
        std::visit([&](const auto& c) { processChange(c, context); }, change);
    }
}

void Scene::processChange(const LinkCreated& link,
                          const FrameContext& /*context*/) {
    Entity* entity = get(link.entity);
    if (!entity)
        return;
    applyParent(*entity, link.parent);
    _toStart.push_back(link.entity);
}

void Scene::processChange(const Reparent& reparenting,
                          const FrameContext& /*context*/) {
    Entity* entity = get(reparenting.entity);
    if (!entity)
        return;
    applyParent(*entity, reparenting.parent);
}

void Scene::processChange(const Destroy& destruction,
                          const FrameContext& context) {
    Entity* entity = get(destruction.entity);
    if (!entity) {
        return;  // already gone: destroyed twice, or an ancestor's
                 // subtree took it
    }
    destroySubtree(*entity, context);
}

void Scene::applyParent(Entity& entity, EntityHandle parent) {
    const bool wasRoot = std::ranges::find(_roots, &entity) != _roots.end();
    entity.setParent(parent);
    const bool isRoot = entity.getParent() == nullptr;
    if (wasRoot && !isRoot) {
        std::erase(_roots, &entity);
    } else if (!wasRoot && isRoot) {
        _roots.push_back(&entity);
    }
}

// Destroys an entity and everything below it, leaves first, so each entity
// has no children left by the time it's destroyed (its destructor asserts
// that).
void Scene::destroySubtree(Entity& entity, const FrameContext& context) {
    // Each destruction removes that child from entity's list, so taking from
    // the back until the list is empty never iterates a changing vector.
    while (!entity.getChildren().empty()) {
        destroySubtree(*entity.getChildren().back(), context);
    }
    destroyEntity(entity, context);
}

void Scene::destroyEntity(Entity& entity, const FrameContext& context) {
    entity.onDestroy(context);
    if (!entity.getParent()) {
        std::erase(_roots, &entity);
    }

    const EntityHandle handle =
        entity.getHandle();  // copied: `entity` dies below
    EntitySlot& slot = _slots[handle.index];
    slot.entity
        .reset();  // runs the destructor, which unlinks it from its parent
    ++slot.generation;
    _freeSlots.push_back(handle.index);
    // `entity` is a dangling reference from here on: don't touch it.
}

void Scene::releaseSubtree(Entity& entity) {
    while (!entity.getChildren().empty()) {
        releaseSubtree(*entity.getChildren().back());
    }
    if (!entity.getParent()) {
        std::erase(_roots, &entity);
    }
    const EntityHandle handle = entity.getHandle();
    EntitySlot& slot = _slots[handle.index];
    slot.entity.reset();
}
