#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <limits>

enum Space { Local, World };

class Entity;

class Transform {
  public:
    Transform();
    Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scale);

    glm::vec3 getPos(Space space = Space::World) const;
    glm::quat getRot(Space space = Space::World) const;
    glm::vec3 getScale(Space space = Space::World) const;
    glm::mat4 getModel(Space space = Space::World) const;
    glm::mat3 getCofactorDetMatrix(Space space = Space::World) const;

    void setRotation(const glm::quat& q, Space space = Space::World);
    void rotate(const glm::quat& q, Space space = Space::World);
    void rotate(const glm::vec3& axis, float angle, Space space = Space::World);
    void rotateAround(const glm::vec3& point, const glm::vec3& axis,
                      float angle, Space space = Space::World);

    void setPosition(const glm::vec3& pos, Space space = Space::World);
    void translate(const glm::vec3& offset, Space space = Space::World);

    void setScale(const glm::vec3& scale, Space space = Space::World);
    void scaleBy(const glm::vec3& scale);

  private:
    friend class Entity;

    // A version no real counter reaches, so the first comparison always fails.
    static constexpr std::uint64_t UNSEEN =
        std::numeric_limits<std::uint64_t>::max();

    void setParent(const Transform* parent);
    void updateModel() const;
    void updateWorld() const;
    void markChanged();

    const Transform* _parent = nullptr;

    glm::quat _rot;
    glm::vec3 _pos, _scale;
    mutable glm::mat4 _model;
    mutable bool _localDirty = true;
    std::uint64_t _localVersion = 0;

    mutable glm::mat4 _world;
    mutable glm::quat _worldRot;
    mutable std::uint64_t _worldVersion = 0;
    mutable std::uint64_t _seenLocalVersion = UNSEEN;
    mutable std::uint64_t _seenParentVersion = UNSEEN;
};
