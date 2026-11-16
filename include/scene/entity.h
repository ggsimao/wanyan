#pragma once

#include "components/behaviour.h"
#include "components/camera.h"
#include "components/lighting.h"
#include "components/mesh_renderer.h"
#include "components/transform.h"

#include <cstddef>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// True if T is one of the variant's alternatives.
template <typename T, typename Variant> constexpr bool isAlternative = false;

template <typename T, typename... Ts>
constexpr bool isAlternative<T, std::variant<Ts...>> = (std::is_same_v<T, Ts> ||
                                                        ...);

template <typename T>
concept BehaviourType =
    std::derived_from<T, BehaviourBase> && isAlternative<T, Behaviour>;

struct EntityHandle {
    static constexpr std::uint32_t NO_INDEX =
        std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index = NO_INDEX;
    std::uint32_t generation = 0;

    bool isNull() const { return index == NO_INDEX; }
    bool operator==(const EntityHandle&) const = default;
    auto operator<=>(const EntityHandle&) const = default;
};

struct EntityDescription {
    std::string name = "";
    EntityHandle parent = {};  // default: invalid, meaning a root
    bool active = true;
    glm::vec3 pos{0.0f};
    glm::quat rot = glm::identity<glm::quat>();
    glm::vec3 scale{1.0f};
    std::optional<MeshRenderer> meshRenderer = {};
    std::optional<Camera> camera = {};
    std::optional<Light> light = {};
    std::vector<Behaviour> behaviours = {};
    // the light and the camera join when they exist
};

class Entity {
  public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&& other) = delete;
    Entity& operator=(Entity&& other) = delete;
    ~Entity();

    Entity* getParent();
    Transform& getTransform();
    MeshRenderer* getMeshRenderer();
    Camera* getCamera();
    Light* getLight();

    const Entity* getParent() const;
    EntityHandle getHandle() const;
    const std::vector<Entity*>& getChildren() const;
    const Entity* getChild(std::size_t i) const;
    const Transform& getTransform() const;
    const MeshRenderer* getMeshRenderer() const;
    const Camera* getCamera() const;
    const Light* getLight() const;
    const std::string getName() const;

    std::size_t getChildrenCount() const;

    void setActive(bool active);

    // Adds a behaviour of type T, constructed from args, and attaches it to
    // this
    // entity. The returned reference is valid until the next addBehaviour,
    // which may grow the vector and move the behaviours.
    template <BehaviourType T, typename... Args>
    T& addBehaviour(Args&&... args) {
        Behaviour& added = _behaviours.emplace_back(
            std::in_place_type<T>, std::forward<Args>(args)...);
        T& behaviour = std::get<T>(added);
        behaviour.entity = this;
        return behaviour;
    }

    // The first behaviour of type T, or nullptr if the entity has none.
    template <BehaviourType T> T* getBehaviour() {
        for (Behaviour& behaviour : _behaviours) {
            if (T* found = std::get_if<T>(&behaviour)) {
                return found;
            }
        }
        return nullptr;
    }

    template <BehaviourType T> const T* getBehaviour() const {
        for (const Behaviour& behaviour : _behaviours) {
            if (const T* found = std::get_if<T>(&behaviour)) {
                return found;
            }
        }
        return nullptr;
    }

    bool isAncestorOf(EntityHandle const node) const;
    bool activeSelf() const;
    bool activeInHierarchy() const;

  private:
    friend class Scene;
    Entity(Scene& scene, EntityHandle handle, EntityDescription desc);

    void setParent(EntityHandle parent);
    void setHandle(EntityHandle handle);

    void start(const FrameContext& context);
    void update(const FrameContext& context);
    void lateUpdate(const FrameContext& context);
    void onDestroy(const FrameContext& context);

    void refreshActive();

    EntityHandle _handle;
    Scene* _scene = nullptr;
    Entity* _parent = nullptr;
    std::vector<Entity*> _children;

    std::string _name;
    Transform _transform;
    std::optional<MeshRenderer> _meshRenderer;
    std::optional<Camera> _camera;
    std::optional<Light> _light;
    std::vector<Behaviour> _behaviours;
    bool _activeSelf = true, _activeInHierarchy = true, _started = false;
};
