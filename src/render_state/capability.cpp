#include "render_state/capability.h"

namespace render_state {
    Capability::Capability(GLenum capability, bool enabled)
        : _capability(capability), _wasEnabled(glIsEnabled(capability) == GL_TRUE) {
        apply(enabled);
    }

    Capability::~Capability() {
        apply(_wasEnabled);
    }

    void Capability::apply(bool enabled) const {
        if (enabled) {
            glEnable(_capability);
        } else {
            glDisable(_capability);
        }
    }
}  // namespace render_state
