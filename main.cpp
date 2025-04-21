#include "renderengine/engine/RenderEngine.h"
#include "renderengine/utils/IOUtils.h"
#include "visualization/BVHVisualizationRenderLogic.h"
#include "construction/timer.hpp"
#include <omp.h>

int main(int argc, char *argv[]) {

    // FIXME: 演示：openmp并行，记得include <omp.h>
    // #pragma omp parallel for
    // for (int i = 0; i < 100; i++) printf("%d\n", i);

    bool gui = true;
    bool render = true;
    std::vector<char*> filtered_args;
    for(int i = 1; i < argc; ++i) {
        char *arg = argv[i];
        if (strcmp(arg, "--no_gui") == 0) {
            gui = false;
            std::cout << "[WARNING] program without GUI" << std::endl;
            continue;
        }
        if (strcmp(arg, "--no_render") == 0) {
            render = false;
            std::cout << "[WARNING] program without rendering" << std::endl;
            continue;
        }
        filtered_args.push_back(arg);
    }

    GLFWwindow *window = RenderEngine::initGL("BVHVisualization", 1920, 1080, gui);
    if (window == nullptr) {
        return -1;
    }
    RenderEngine::initImGui(window);

    IRenderLogic *renderLogic = new BVHVisualizationRenderLogic();

    for (char *arg_i: filtered_args) {
        renderLogic->m_startupParameters.emplace_back(arg_i);
    }
    renderLogic->with_gui = gui;
    renderLogic->with_render = render;

    auto *renderEngine = new RenderEngine(window, renderLogic);
    try {
        renderEngine->run();
    } catch (const std::exception &e) {
        delete renderEngine;
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    delete renderEngine;

    if(!gui) {
        timer::time_prefix_sum();
    }

    return EXIT_SUCCESS;
}
