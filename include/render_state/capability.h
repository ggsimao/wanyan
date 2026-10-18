#pragma once

#include <GL/glew.h>

namespace render_state {
    // Enables or disables one capability (GL_DEPTH_TEST, GL_CULL_FACE, GL_BLEND,
    // ...) for a scope, then restores whatever it was before.
    //
    // Like every guard in render_state, it reads the current state back from GL
    // on construction. That's cheap at a handful of guards per pass; avoid
    // creating them per draw call in hot loops.
    class Capability {
    public:
        Capability(GLenum capability, bool enabled);
        ~Capability();
        Capability(const Capability&) = delete;
        Capability& operator=(const Capability&) = delete;

    private:
        void apply(bool enabled) const;

        GLenum _capability;
        bool _wasEnabled;
    };
}  // namespace render_state
