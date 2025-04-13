#include "BVHVisualizationRenderLogic.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include "imgui.h"

#include "../renderengine/utils/IOUtils.h"

void BVHVisualizationRenderLogic::init() {
    if (m_startupParameters.size() != 1) {
        std::cout << "[WARNING] No input path to the obj file as program argument provided. Using the example file instead. Otherwise use: ./BVHVisualization <pathToOBJ.obj>" << std::endl;
    }

    m_currentModel = 1; // 0 for dragon, 1 for cow, 2 for ...
    m_startupParameters.emplace_back("./resources/models/spot/spot_triangulated_good.obj");
    // m_startupParameters.emplace_back("./resources/models/dragon/xyzrgb_dragon.obj");

    m_camera.move(2.f, 0.f, 5.f);

    m_renderer.init(m_startupParameters[0]);
}

void BVHVisualizationRenderLogic::update(float time, KeyboardInput *keyboardInput, MouseInput *mouseInput) {
    static const float CAMERA_POS_STEP = 3.f;
    static const float MOUSE_SENSITIVITY = 0.25f;

    float factor = 1.0f;
    if (keyboardInput->isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
        factor = 25.f;
    }

    glm::vec3 cameraInc(0.f);
    if (keyboardInput->isKeyPressed(GLFW_KEY_W)) {
        cameraInc.z -= factor;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_S)) {
        cameraInc.z += factor;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_A)) {
        cameraInc.x -= factor;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_D)) {
        cameraInc.x += factor;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_LEFT_SHIFT) || keyboardInput->isKeyPressed(GLFW_KEY_Q)) {
        cameraInc.y -= factor;
    }
    if (keyboardInput->isKeyPressed(GLFW_KEY_SPACE) || keyboardInput->isKeyPressed(GLFW_KEY_E)) {
        cameraInc.y += factor;
    }

    m_camera.moveCenter(cameraInc.x * CAMERA_POS_STEP * time, cameraInc.y * CAMERA_POS_STEP * time,
                        cameraInc.z * CAMERA_POS_STEP * time);

    if (mouseInput->isLeftButtonPressed()) {
        glm::vec2 motion = mouseInput->getMotion();
        m_camera.move(0.0f, motion.y * MOUSE_SENSITIVITY * time, -motion.x * MOUSE_SENSITIVITY * time);
    }
    if (mouseInput->getScroll(false).y != 0) {
        m_camera.move(-mouseInput->getScroll(true).y, 0.f, 0.f);
    }
}

void BVHVisualizationRenderLogic::render() {
    m_renderer.render(&m_camera, windowSize.x, windowSize.y);
}

void BVHVisualizationRenderLogic::renderGui() {
    ImGui::Begin("BVHVisualization");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    ImGui::Spacing();

    ImGui::Text("Cell (X,Y,Z) = (%i,%i,%i)", static_cast<int>(glm::floor(m_camera.getPosition().x)),
                static_cast<int>(glm::floor(m_camera.getPosition().y)),
                static_cast<int>(glm::floor(m_camera.getPosition().z)));
    ImGui::Text("Position (X,Y,Z) = (%.3f,%.3f,%.3f)", m_camera.getPosition().x, m_camera.getPosition().y,
                m_camera.getPosition().z);
    ImGui::Text("Rotation (R,Phi,Theta) = (%.3f,%.3f,%.3f)", m_camera.getR(), m_camera.getPhi(),
                m_camera.getTheta());

    ImGui::Spacing();

    const char* models[] = { "Dragon", "Cow", "Homer", "Face", "Car" };
    if (ImGui::Combo("Model", &m_currentModel, models, IM_ARRAYSIZE(models))) {
        // This block executes when the selection changes
        m_startupParameters.clear();
        switch (m_currentModel) {
            case 0:
                m_startupParameters.emplace_back("./resources/models/dragon/xyzrgb_dragon.obj");
                break;
            case 1:
                m_startupParameters.emplace_back("./resources/models/spot/spot_triangulated_good.obj");
                break;
            case 2:
                m_startupParameters.emplace_back("./resources/models/homer/homer.obj");
                break;
            case 3:
                m_startupParameters.emplace_back("./resources/models/face/max-planck.obj");
                break;
            case 4:
                m_startupParameters.emplace_back("./resources/models/car/beetle-alt.obj");
                break;
            default:
                m_startupParameters.emplace_back("./resources/models/spot/spot_triangulated_good.obj");
                break;
        }
    }

    if (ImGui::Button("Reconstruction")) {
        m_renderer.init(m_startupParameters[0]);
    }

    ImGui::Spacing();

    if (ImGui::Button("Screenshot")) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << "screenshot_" << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".png";
        IOUtils::writeFramebufferToFile(oss.str(), windowSize.x, windowSize.y);
    }


    ImGui::End();
}

void BVHVisualizationRenderLogic::cleanUp() {
    m_renderer.cleanUp(true);
}

void BVHVisualizationRenderLogic::onWindowResized(int width, int height) {
    windowSize = glm::vec2(width, height);
}

bool BVHVisualizationRenderLogic::generateSceneScreenshot(int number, IRenderLogic::SceneScreenshotInfo *info) {
    return false;
}