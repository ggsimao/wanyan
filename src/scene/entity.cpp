#include "scene/entity.h"
#include "scene/scene.h"

Entity::~Entity() {
    // Scene::destroySubtree destroys leaves first, so any children are gone by
    // now. A child left here would keep pointers to this entity and its
    // transform.
    assert(_children.empty() &&
           "Destroy an entity's children before the entity");

    if (_parent) {
        std::vector<Entity*>& siblings = _parent->_children;
        // Usually the last child, since destroySubtree takes them from the
        // back, so check there first: constant time instead of a scan.
        if (!siblings.empty() && siblings.back() == this) {
            siblings.pop_back();
        } else {
            std::erase(siblings, this);
        }
    }
}

Entity* Entity::getParent() { return _parent; }

Transform& Entity::getTransform() { return _transform; }

MeshRenderer* Entity::getMeshRenderer() {
    return _meshRenderer ? &*_meshRenderer : nullptr;
}

Camera* Entity::getCamera() { return _camera ? &*_camera : nullptr; }

Light* Entity::getLight() { return _light ? &*_light : nullptr; }

const Entity* Entity::getParent() const { return _parent; }

EntityHandle Entity::getHandle() const { return _handle; }

const std::vector<Entity*>& Entity::getChildren() const { return _children; }

const Entity* Entity::getChild(std::size_t i) const {
    assert(i < _children.size());
    return _children[i];
}

const Transform& Entity::getTransform() const { return _transform; }

const MeshRenderer* Entity::getMeshRenderer() const {
    return _meshRenderer ? &*_meshRenderer : nullptr;
}

const Camera* Entity::getCamera() const {
    return _camera ? &*_camera : nullptr;
}

const Light* Entity::getLight() const { return _light ? &*_light : nullptr; }

const std::string Entity::getName() const { return _name; }

std::size_t Entity::getChildrenCount() const { return _children.size(); }

bool Entity::isAncestorOf(EntityHandle const node) const {
    assert(_scene);
    Entity* current = _scene->get(node);
    while (current) {
        if (current == this)
            return true;
        current = current->_parent;
    }
    return false;
}

bool Entity::activeSelf() const { return _activeSelf; }

bool Entity::activeInHierarchy() const { return _activeInHierarchy; }

void Entity::setActive(bool active) {
    _activeSelf = active;
    refreshActive();
}

Entity::Entity(Scene& scene, EntityHandle handle, EntityDescription desc)
    : _handle(handle), _scene(&scene), _name(std::move(desc.name)),
      _transform(desc.pos, desc.rot, desc.scale),
      _meshRenderer(std::move(desc.meshRenderer)),
      _camera(std::move(desc.camera)), _light(std::move(desc.light)),
      _behaviours(std::move(desc.behaviours)), _activeSelf(desc.active) {
    setParent(desc.parent);
    refreshActive();
    for (Behaviour& behaviour : _behaviours) {
        std::visit([this](BehaviourBase& base) { base.entity = this; },
                   behaviour);
    }
}

void Entity::setParent(EntityHandle parent) {
    assert(_scene);
    Entity* parentNode = _scene->get(parent);
    assert((parentNode != this && !isAncestorOf(parent)) &&
           "setParent would create a cycle");
    if (parentNode == _parent)
        return;
    if (_parent)
        std::erase(_parent->_children, this);
    _parent = parentNode;
    if (_parent)
        _parent->_children.emplace_back(this);
    _transform.setParent(_parent ? &_parent->_transform : nullptr);
    refreshActive();
}

void Entity::setHandle(EntityHandle handle) { _handle = handle; }

void Entity::start(const FrameContext& context) {
    if (!_started) {
        for (auto& behavior : _behaviours) {
            ::start(behavior, context);
        }
        _started = true;
    }
}

void Entity::update(const FrameContext& context) {
    if (!_activeInHierarchy)
        return;
    for (auto& behavior : _behaviours) {
        ::update(behavior, context);
    }
    for (auto child : _children) {
        child->update(context);
    }
}

void Entity::lateUpdate(const FrameContext& context) {
    if (!_activeInHierarchy)
        return;
    for (auto& behavior : _behaviours) {
        ::lateUpdate(behavior, context);
    }
    for (auto child : _children) {
        child->lateUpdate(context);
    }
}

void Entity::onDestroy(const FrameContext& context) {
    for (auto child : _children) {
        child->onDestroy(context);
    }
    for (auto& behavior : _behaviours) {
        ::onDestroy(behavior, context);
    }
}

void Entity::refreshActive() {
    bool newActive = _activeSelf && (!_parent || _parent->_activeInHierarchy);
    if (_activeInHierarchy != newActive) {
        _activeInHierarchy = newActive;
        for (auto child : _children)
            child->refreshActive();
    }
}
