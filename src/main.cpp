#include "camera.h"
#include "controls/controller_hub.h"
#include "data/uniform_buffer.h"
#include "debug/frame_stats.h"
#include "debug/gl_debug.h"
#include "platform.h"
#include "program.h"
#include "random_transform.h"
#include "scene/demo_scene.h"
#include "scene/scene_renderer.h"
#include "screen.h"
#include "textures/texture_2d.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cassert>
#include <chrono>
#include <exception>
#include <iostream>
#include <vector>

namespace {
    constexpr const char* WINDOW_TITLE = "Wanyan";
    constexpr int WINDOW_WIDTH = 600;
    constexpr int WINDOW_HEIGHT = 600;

    const glm::vec4 CLEAR_COLOR(0.1f, 0.1f, 0.1f, 1.0f);
    const glm::vec3 CAMERA_START(0.0f, 0.0f, -2.0f);

    // The shaders declare the Matrices block with layout(binding = 0).
    constexpr GLuint MATRICES_BINDING = 0;

    // The mirror's picture-in-picture: a third of the size, in the top right.
    constexpr float INSET_SCALE = 0.3f;
    const glm::vec2 INSET_OFFSET(0.5f, 0.5f);

    // buildDemoScene puts the rocks first.
    constexpr std::size_t ROCKS = 0;
}  // namespace

int main() {
    try {
        // First, so it's destroyed last: every GL object below must be
        // deleted while its context still exists.
        Platform platform(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
        debug::enableGlDebugOutput();
        glDebugMessageInsert(
            GL_DEBUG_SOURCE_APPLICATION,
            GL_DEBUG_TYPE_OTHER,
            1,
            GL_DEBUG_SEVERITY_MEDIUM,
            -1,
            "Debug working."
        );

        const DemoShaders shaders = loadDemoShaders();
        UniformBuffer matrices(MATRICES_BINDING);
        matrices.bindBase();  // once: the attachment persists

        Camera camera(CAMERA_START);
        Scene scene = buildDemoScene(camera);
        std::vector<RandomTransform> rockTransforms = buildRockTransforms();

        SceneRenderer renderer(shaders.forRenderer(), matrices);
        Screen screen(platform.width(), platform.height(), CLEAR_COLOR, shaders.screen);
        Screen mirror(platform.width(), platform.height(), CLEAR_COLOR, shaders.screen);

        Program program;
        ControllerHub controls;
        debug::FrameStats stats;

        using Clock = std::chrono::steady_clock;
        Clock::time_point previousFrame = Clock::now();

        while (program.loopActive()) {
            const Clock::time_point now = Clock::now();
            const float frameMilliseconds =
                std::chrono::duration<float, std::milli>(now - previousFrame).count();
            previousFrame = now;

            {
                auto timer = stats.time(debug::FrameStats::UPDATE);
                controls.update(frameMilliseconds,
                                {program, camera, scene.lighting.spotlight, renderer.options(),
                                 screen.parameters(), rockTransforms});
            }

            // The flashlight is held by the camera.
            scene.lighting.spotlight.position = camera.pos();
            scene.lighting.spotlight.direction = camera.dir();

            {
                auto timer = stats.time(debug::FrameStats::INSTANCES);
                SceneObject& rocks = scene.objects[ROCKS];
                assert(rocks.instanceCount() == rockTransforms.size());
                for (std::size_t i = 0; i < rockTransforms.size(); ++i) {
                    rockTransforms[i].rotate(rocks.instance(i));
                    rockTransforms[i].translate(rocks.instance(i));
                }
            }

            {
                auto timer = stats.time(debug::FrameStats::DRAW);
                renderer.renderShadowMaps(scene);  // once: shadows don't depend on the camera
                screen.drawOnFramebuffer(renderer, scene, camera);
                mirror.drawOnFramebuffer(renderer, scene, camera.invert());
                mirror.drawOnAnother(screen, INSET_SCALE, INSET_OFFSET);
                screen.drawOnScreen();
            }

            platform.swapBuffers();
            stats.endFrame(program.showStats());
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
