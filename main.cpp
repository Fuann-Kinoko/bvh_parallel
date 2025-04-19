#include "renderengine/engine/RenderEngine.h"
#include "renderengine/utils/IOUtils.h"
#include "visualization/BVHVisualizationRenderLogic.h"
#include "construction/timer.hpp"

int main(int argc, char *argv[]) {
    bool gui = true;
    if (argc > 1) {
        char *first_arg = argv[1];
        if (strcmp(first_arg, "--no_gui") == 0) {
            gui = false;
            std::cout << "[WARNING] program without GUI" << std::endl;
        }
    }

    GLFWwindow *window = RenderEngine::initGL("BVHVisualization", 1920, 1080, gui);
    if (window == nullptr) {
        return -1;
    }
    RenderEngine::initImGui(window);

    IRenderLogic *renderLogic = new BVHVisualizationRenderLogic();

    int initArg = (!gui) ? 2 : 1;
    for (int i = initArg; i < argc; i++) {
        renderLogic->m_startupParameters.emplace_back(argv[i]);
    }
    renderLogic->with_gui = gui;

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
