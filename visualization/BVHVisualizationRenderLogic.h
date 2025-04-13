#pragma once

#include "glad/glad.h"
// #include <GL/glew.h>

#include "../../renderengine/engine/IRenderLogic.h"
#include "../../renderengine/camera/ThirdPersonCamera.h"

#include "BVHVisualizationRenderer.h"

class BVHVisualizationRenderLogic : public IRenderLogic {
public:
    void init() override;

    void update(float time, KeyboardInput *keyboardInput, MouseInput *mouseInput) override;

    void render() override;

    void renderGui() override;

    void cleanUp() override;

    void onWindowResized(int width, int height) override;

    bool generateSceneScreenshot(int number, SceneScreenshotInfo *info) override;

    void blockUntilBuildComplete() override;

private:
    ThirdPersonCamera m_camera;
    BVHVisualizationRenderer m_renderer;
    int m_currentModel;

    glm::ivec2 windowSize = glm::ivec2(1920, 1080);
};
