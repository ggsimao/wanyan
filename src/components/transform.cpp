#include "components/transform.h"
#include "math.h"

#include <glm/ext/quaternion_common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cassert>

Transform::Transform()
    : _rot(glm::identity<glm::quat>()), _pos(glm::vec3(0.0)),
      _scale(glm::vec3(1.0)) {}

Transform::Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scale)
    : _rot(rot), _pos(pos), _scale(scale) {}

glm::vec3 Transform::getPos(Space space) const {
    if (space == Space::World) {
        updateWorld();
        return glm::vec3(_world[3]);
    }
    return _pos;
}

glm::quat Transform::getRot(Space space) const {
    if (space == Space::World) {
        updateWorld();
        return _worldRot;
    }
    return _rot;
}

glm::vec3 Transform::getScale(Space space) const {
    if (space == Space::World) {
        updateWorld();
        return {glm::length(glm::vec3(_world[0])),
                glm::length(glm::vec3(_world[1])),
                glm::length(glm::vec3(_world[2]))};
    }
    return _scale;
}

glm::mat4 Transform::getModel(Space space) const {
    if (space == Space::World) {
        updateWorld();
        return _world;
    }
    updateModel();
    return _model;
}

glm::mat3 Transform::getCofactorDetMatrix(Space space) const {
    const glm::mat4& model = (space == Space::World)
                                 ? this->getModel(Space::World)
                                 : this->getModel(Space::Local);
    return cofactorDetMatrix(model);
}

// Space::Local: q is the rotation relative to the parent (the local rotation).
// Space::World: q is the rotation in world space.
void Transform::setRotation(const glm::quat& q, Space space) {
    _rot = space == Space::Local || !_parent
               ? q
               : glm::inverse(_parent->getRot(Space::World)) * q;
    _rot = glm::normalize(_rot);
    markChanged();
}

// Space::Local: turns around the object's own axes.
// Space::World: turns around the world's axes. The object's world rotation is
// P * _rot; applying q in world space gives q * P * _rot, and dividing out
// the parent's rotation brings it back relative to the parent.
void Transform::rotate(const glm::quat& q, Space space) {
    if (space == Space::Local) {
        _rot = _rot * q;
    } else if (!_parent) {
        _rot = q * _rot;
    } else {
        const glm::quat p = _parent->getRot(Space::World);
        _rot = glm::inverse(p) * q * p * _rot;
    }
    _rot = glm::normalize(_rot);
    markChanged();
}

void Transform::rotate(const glm::vec3& axis, float angle, Space space) {
    rotate(glm::angleAxis(angle, glm::normalize(axis)), space);
}

// Orbits around `point`, turning the object as it goes. Space::World: point and
// axis are in world space. Space::Local: they're in the object's own space,
// relative to its origin and axes.
void Transform::rotateAround(const glm::vec3& point, const glm::vec3& axis,
                             float angle, Space space) {
    glm::vec3 parentPoint;
    glm::vec3 parentAxis;
    if (space == Space::Local || !_parent) {
        parentPoint =
            glm::vec3(getModel(Space::Local) * glm::vec4(point, 1.0f));
        parentAxis = _rot * axis;
    } else {
        parentPoint = glm::vec3(glm::inverse(_parent->getModel(Space::World)) *
                                glm::vec4(point, 1.0f));
        parentAxis = glm::inverse(_parent->getRot(Space::World)) * axis;
    }

    const glm::quat q = glm::angleAxis(angle, glm::normalize(parentAxis));
    _pos = parentPoint + q * (_pos - parentPoint);
    _rot = glm::normalize(q * _rot);
    markChanged();
}

// Space::Local: pos is relative to the parent (the local position).
// Space::World: pos is in world space.
void Transform::setPosition(const glm::vec3& pos, Space space) {
    _pos = space == Space::Local || !_parent
               ? pos
               : glm::vec3(glm::inverse(_parent->getModel(Space::World)) *
                           glm::vec4(pos, 1.0f));
    markChanged();
}

// Space::Local: moves along the object's own axes.
// Space::World: moves along the world's axes. The offset is converted into the
// parent's space as a direction (w = 0), so the parent's rotation and scale
// apply but not its translation.
void Transform::translate(const glm::vec3& offset, Space space) {
    if (space == Space::Local) {
        _pos += _rot * offset;
    } else if (!_parent) {
        _pos += offset;
    } else {
        _pos += glm::vec3(glm::inverse(_parent->getModel(Space::World)) *
                          glm::vec4(offset, 0.0f));
    }
    markChanged();
}

// Space::Local: the local scale.
// Space::World: sets the local scale so the world scale comes out as `scale`,
// by dividing by the parent's lossy scale. Exact when the parent's scale is
// uniform, or the object's axes line up with the parent's; approximate
// otherwise, like Unity's lossyScale.
void Transform::setScale(const glm::vec3& scale, Space space) {
    if (space == Space::Local) {
        _scale = scale;
    } else {
        const glm::vec3 parentScale =
            _parent ? _parent->getScale(Space::World) : glm::vec3(1.0f);
        assert(glm::all(glm::greaterThan(parentScale, glm::vec3(0.0f))) &&
               "Can't set a world scale under a parent with zero scale");
        _scale = scale / parentScale;
    }
    markChanged();
}

void Transform::scaleBy(const glm::vec3& scale) {
    _scale *= scale;
    markChanged();
}

void Transform::setParent(const Transform* parent) {
    _parent = parent;
    _seenParentVersion = UNSEEN;
}

// Rebuilds the local matrix if position, rotation or scale changed since.
void Transform::updateModel() const {
    if (!_localDirty) {
        return;
    }
    _model = glm::translate(glm::mat4(1.0f), _pos) * glm::mat4_cast(_rot) *
             glm::scale(glm::mat4(1.0f), _scale);
    _localDirty = false;
}

// Rebuilds the world matrix and rotation if this transform or any ancestor
// changed since they were last computed. Ancestors are brought up to date
// first, so the comparison sees their current versions.
void Transform::updateWorld() const {
    std::uint64_t parentVersion = 0;  // roots: nothing above can change
    if (_parent) {
        _parent->updateWorld();
        parentVersion = _parent->_worldVersion;
    }

    if (_localVersion == _seenLocalVersion &&
        parentVersion == _seenParentVersion) {
        return;  // built from exactly these values already
    }

    updateModel();
    if (_parent) {
        _world = _parent->_world * _model;
        _worldRot = _parent->_worldRot * _rot;
    } else {
        _world = _model;
        _worldRot = _rot;
    }

    _seenLocalVersion = _localVersion;
    _seenParentVersion = parentVersion;
    ++_worldVersion;  // tells children their world caches are stale
}

// Called by every setter after changing the position, rotation or scale.
void Transform::markChanged() {
    _localDirty = true;  // the local matrix must be rebuilt on next read
    ++_localVersion;     // world caches built from the old values are now stale
}
