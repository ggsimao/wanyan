#include "app/launch_options.h"
#include "app/platform.h"
#include "app/program.h"
#include "controls/controller_hub.h"
#include "data/frame_buffer.h"
#include "data/uniform_buffer.h"
#include "debug/frame_stats.h"
#include "random.h"
#include "scene/demo_scene.h"
#include "scene/scene_renderer.h"
#include "view.h"

#include <CLI/CLI.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cassert>
#include <chrono>
#include <exception>
#include <iostream>
#include <random>

namespace {
    constexpr const char* WINDOW_TITLE = "Wanyan";
    constexpr int WINDOW_WIDTH = 600;
    constexpr int WINDOW_HEIGHT = 600;

    const glm::vec4 CLEAR_COLOR(0.1f, 0.1f, 0.1f, 1.0f);

    // The shaders declare the Matrices block with layout(binding = 0).
    constexpr GLuint MATRICES_BINDING = 0;

    // The mirror's picture-in-picture: a third of the size, in the top right.
    constexpr float INSET_SCALE = 0.3f;
    const glm::vec2 INSET_OFFSET(0.5f, 0.5f);
}  // namespace

int main(int argc, char** argv) {
    try {
        CLI::App app{"Wanyan"};

        LaunchOptions options;

        // Register options
        auto seedOption =
            app.add_option("--seed", options.seed, "Seed for RNG");
        app.add_flag("--no-layered-shadows",
                     options.platformOptions.forcedUnlayering,
                     "Set if you don't want point shadow layering");
        app.add_flag("--no-vsync", options.platformOptions.noVSync,
                     "Set if you want VSync to be turned off");

        // Execute the parsing
        CLI11_PARSE(app, argc, argv);

        if (!seedOption->count()) {
            options.seed = std::random_device{}();
        }
        std::cout << "Seed: " << options.seed << '\n';
        random_engine::setSeed(options.seed);

        // First, so it's destroyed last: every GL object below must be
        // deleted while its context still exists.
        Platform platform(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT,
                          options.platformOptions);
        // Intentional startup self-check: confirms GL debug output reaches our
        // callback
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER,
                             1, GL_DEBUG_SEVERITY_MEDIUM, -1, "Debug working.");

        const DemoShaders shaders = loadDemoShaders(platform.gpuFeatures());
        UniformBuffer matrices(MATRICES_BINDING);
        matrices.bindBase();  // once: the attachment persists

        Scene scene({});
        auto entities = buildDemoScene(scene);

        SceneRenderer renderer(shaders.forRenderer(), matrices);
        View screen(platform.width(), platform.height(), CLEAR_COLOR);
        screen.parameters().crosshair = true;
        View mirror(INSET_SCALE * platform.width(),
                    INSET_SCALE * platform.height(), CLEAR_COLOR);

        glm::mat4 mirrorPlacement =
            glm::translate(glm::mat4(1.0f), glm::vec3(INSET_OFFSET, 0.0f)) *
            glm::scale(glm::mat4(1.0f),
                       glm::vec3(INSET_SCALE, INSET_SCALE, 1.0f));
        glm::mat4 regularPlacement = glm::identity<glm::mat4>();

        const PostShaders postShaders = {.resolve = shaders.screen.resolve,
                                         .luminance = shaders.screen.luminance,
                                         .tonemap = shaders.screen.tonemap};

        Program program;
        ControllerHub controls;
        debug::FrameStats stats;

        using Clock = std::chrono::steady_clock;
        Clock::time_point previousFrame = Clock::now();

        FrameContext context{
            .seed = options.seed, .scene = scene, .input = controls.getInput()};

        while (program.loopActive()) {
            const Clock::time_point now = Clock::now();
            const float frameMilliseconds =
                std::chrono::duration<float, std::milli>(now - previousFrame)
                    .count();
            previousFrame = now;
            context.dt = frameMilliseconds;
            context.scene.time.advance(context.dt);

            {
                auto timer = stats.time(debug::FrameStats::UPDATE);
                controls.update({.program = program,
                                 .renderOptions = renderer.options(),
                                 .view = screen.parameters(),
                                 .platform = platform});
                scene.update(context);
            }

            {
                auto timer = stats.time(debug::FrameStats::INSTANCES);
                renderer.updateFrameData(scene);
            }

            {
                auto timer = stats.time(debug::FrameStats::SHADOWS);
                renderer.renderShadowMaps();  // once: shadows don't depend on
                                              // the camera
            }

            {
                auto timer = stats.time(debug::FrameStats::DRAW);
                const auto mainCamera = scene.getActiveCamera();
                screen.drawScene(renderer, scene, mainCamera->getHandle());
                screen.postProcess(postShaders);
                if (screen.parameters().mirror &&
                    mainCamera->getChildrenCount() > 0) {
                    const auto backCamera = mainCamera->getChild(0);
                    if (backCamera) {
                        auto &mirrorParameters = mirror.parameters(),
                             &screenParameters = screen.parameters();
                        mirrorParameters.filterOn = screenParameters.filterOn;
                        mirrorParameters.msaaOn = screenParameters.msaaOn;
                        mirrorParameters.gamma = screenParameters.gamma;

                        mirror.drawScene(renderer, scene,
                                         backCamera->getHandle());
                        mirror.postProcess(postShaders);
                        mirror.present(shaders.screen.present,
                                       DrawFrameBufferBinding(screen.output()),
                                       mirrorPlacement);
                    }
                }
                screen.present(shaders.screen.present,
                               DrawFrameBufferBinding::screen(
                                   platform.width(), platform.height()),
                               regularPlacement);
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
