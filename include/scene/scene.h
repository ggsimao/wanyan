#pragma once

#include "drawables/skybox.h"
#include "scene/entity.h"

#include <vector>

class Scene {
  public:
    std::vector<Skybox> skyboxes;
    glm::vec3 ambient;
    Scene(std::vector<Skybox> skyboxes, glm::vec3 ambient = glm::vec3(0.0f));
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&& other) = delete;
    Scene& operator=(Scene&& other) = delete;
    ~Scene();

    // A reparenting requested during update takes effect at the end of the
    // frame. Until then, the entity’s world transform still follows its old
    // parent. Within the frame, everything sees a consistent tree; the change
    // appears all at once, before rendering.
    void update(const FrameContext& context);

    EntityHandle create(EntityDescription desc);
    void destroy(EntityHandle handle);
    void setParent(EntityHandle entity, EntityHandle parent,
                   bool keepWorldTransform = false);

    Entity* get(EntityHandle handle);
    const Entity* get(EntityHandle handle) const;

    const std::vector<Entity*> getRoots() const;
    const Entity* getActiveCamera() const;

    void setActiveCamera(EntityHandle handle);

    // Calls process(entity) for every active entity,
    // depth-first. Inactive entities are skipped with their whole subtree.
    template <typename F> void forEachActive(F& process) {
        for (Entity* root : _roots) {
            forEachInSubtree(*root, process);
        }
    }
    template <typename F> void forEachInSubtree(Entity& entity, F& process) {
        if (!entity.activeSelf()) {
            return;
        }
        process(entity);
        for (Entity* child : entity.getChildren()) {
            forEachInSubtree(*child, process);
        }
    }
    template <typename F> void forEachActive(F& process) const {
        for (const Entity* root : _roots) {
            forEachInSubtree(*root, process);
        }
    }
    template <typename F>
    void forEachInSubtree(const Entity& entity, F& process) const {
        if (!entity.activeSelf()) {
            return;
        }
        process(entity);
        for (const Entity* child : entity.getChildren()) {
            forEachInSubtree(*child, process);
        }
    }

  private:
    struct EntitySlot {
        std::unique_ptr<Entity> entity;
        std::uint32_t generation = 0;
    };
    struct LinkCreated {
        EntityHandle entity;
        EntityHandle parent;
    };
    struct Reparent {
        EntityHandle entity;
        EntityHandle parent;
        bool keepWorldTransform = false;
    };
    struct Destroy {
        EntityHandle entity;
    };
    using StructuralChange = std::variant<LinkCreated, Reparent, Destroy>;

    void processChanges(const FrameContext& context);
    void processChange(const LinkCreated& link, const FrameContext& context);
    void processChange(const Reparent& reparenting,
                       const FrameContext& context);
    void processChange(const Destroy& destruction, const FrameContext& context);

    void applyParent(Entity& entity, EntityHandle parent);
    void destroySubtree(Entity& entity, const FrameContext& context);
    void destroyEntity(Entity& entity, const FrameContext& context);
    void releaseSubtree(Entity& entity);

    std::vector<EntitySlot> _slots;
    std::vector<std::uint32_t> _freeSlots;
    std::vector<EntityHandle> _toStart;
    std::vector<Entity*> _roots;
    std::vector<StructuralChange> _pendingChanges;
    Entity* _mainCamera;
};
