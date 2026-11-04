#include "app/launch_options.h"
#include "camera.h"
#include "controls/controller_hub.h"
#include "data/uniform_buffer.h"
#include "debug/frame_stats.h"
#include "app/platform.h"
#include "app/program.h"
#include "random_transform.h"
#include "scene/demo_scene.h"
#include "scene/scene_renderer.h"
#include "screen.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <CLI/CLI.hpp>

#include <cassert>
#include <chrono>
#include <exception>
#include <iostream>
#include <random>
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

int main(int argc, char** argv) {
    try {
        CLI::App app{"Wanyan"};

        LaunchOptions options;

        // Register options
        auto seedOption = app.add_option(
            "--seed",
            options.seed,
            "Seed for RNG"
        );
        app.add_flag(
            "--no-layered-shadows",
            options.platformOptions.forcedUnlayering,
            "Set if you don't want point shadow layering"
        );
        app.add_flag(
            "--no-vsync",
            options.platformOptions.noVSync,
            "Set if you want VSync to be turned off"
        );

        // Execute the parsing
        CLI11_PARSE(app, argc, argv);

        if (!seedOption->count()) {
            options.seed = std::random_device{}();
        }
        std::cout << "Seed: " << options.seed << '\n';

        // First, so it's destroyed last: every GL object below must be
        // deleted while its context still exists.
        Platform platform(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, options.platformOptions);
        // Intentional startup self-check: confirms GL debug output reaches our callback
        glDebugMessageInsert(
            GL_DEBUG_SOURCE_APPLICATION,
            GL_DEBUG_TYPE_OTHER,
            1,
            GL_DEBUG_SEVERITY_MEDIUM,
            -1,
            "Debug working."
        );

        const DemoShaders shaders = loadDemoShaders(platform.gpuFeatures());
        UniformBuffer matrices(MATRICES_BINDING);
        matrices.bindBase();  // once: the attachment persists

        Camera camera(CAMERA_START);
        RandomTransform::setSeed(options.seed);
        Scene scene = buildDemoScene(camera);
        std::vector<RandomTransform> rockTransforms = buildRockTransforms(options.seed);

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
                                {.program=program, .camera=camera, .flashlight=scene.lighting.spotlight,
                                    .renderOptions=renderer.options(), .screen=screen.parameters(),
                                    .rockTransforms=rockTransforms});
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
                auto timer = stats.time(debug::FrameStats::SHADOWS);
                renderer.renderShadowMaps(scene);  // once: shadows don't depend on the camera
            }

            {
                auto timer = stats.time(debug::FrameStats::DRAW);
                screen.drawOnFramebuffer(renderer, scene, camera);
                if (screen.parameters().mirror) {
                    mirror.drawOnFramebuffer(renderer, scene, camera.invert());
                    mirror.drawOnAnother(screen, INSET_SCALE, INSET_OFFSET);
                }
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
