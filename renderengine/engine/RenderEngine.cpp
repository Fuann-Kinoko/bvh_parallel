#include "RenderEngine.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

RenderEngine::RenderEngine(GLFWwindow *window, IRenderLogic *renderLogic)
        : m_window(window), m_renderLogic(renderLogic), m_keyboardInput(new KeyboardInput(m_window)),
          m_mouseInput(new MouseInput()) {

}

RenderEngine::~RenderEngine() {
    delete m_renderLogic;
    delete m_keyboardInput;
    delete m_mouseInput;
}

void RenderEngine::run() {
    init();
    renderLoop();
    cleanUp();
}

void RenderEngine::init() {
    installCallbacks();
    m_renderLogic->init();
}

void RenderEngine::renderLoop() {
    bool no_gui = !m_renderLogic->with_gui;
    int no_gui_tick = 0;
    while (!glfwWindowShouldClose(m_window) && no_gui_tick < 100) {
        update();
        render();
        if(no_gui) {
            ++no_gui_tick;
            m_renderLogic->blockUntilBuildComplete();
        }
    }
    if(no_gui) {
        update();
        render();
        m_renderLogic->generateSceneScreenshot(1, NULL);
    }
}

void RenderEngine::cleanUp() {
    m_renderLogic->cleanUp();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void RenderEngine::update() {
    double time = glfwGetTime();
    double timeSinceLastFrame = glfwGetTime() - m_lastTime;
    m_lastTime = time;
    m_mouseInput->update();
    m_renderLogic->update(static_cast<float>(timeSinceLastFrame), m_keyboardInput, m_mouseInput);
}

void RenderEngine::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_renderLogic->render();
    if(m_renderLogic->with_gui)
        renderGui();
    /* Swap front and back buffers */
    glfwSwapBuffers(m_window);
    /* Poll for and process events */
    glfwPollEvents();
}

void RenderEngine::renderGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_renderLogic->renderGui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "ERROR::GLFW::%d: %s\n", error, description);
}

GLFWwindow *RenderEngine::initGL(const std::string &title, int width, int height, bool gui) {
    GLFWwindow *window;

    glfwSetErrorCallback(glfw_error_callback);
    /* Initialize the library */
    if (!glfwInit()) {
        return nullptr;
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if(!gui) glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });


    if(gui) {
        const GLFWvidmode *vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if (!vidMode) {
            glfwTerminate();
            return nullptr;
        }
        glfwSetWindowPos(window,
                        static_cast<int>(static_cast<float>(vidMode->width - width) / 2.f),
                        static_cast<int>(static_cast<float>(vidMode->height - height) / 2.f));
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
    }

    /* Enable v-sync */
    glfwSwapInterval(1);

    /* Make the window visible */
    if(gui) {
        glfwShowWindow(window);
    }

    /* Set the clear color */
    glClearColor(RED, GREEN, BLUE, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    return window;
}

void RenderEngine::initImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

void RenderEngine::installCallbacks() {
    glfwSetWindowUserPointer(m_window, this);

    // framebuffer size
    {
        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *window, int width, int height) {
            glViewport(0, 0, width, height);
            auto *renderEngine = static_cast<RenderEngine *>(glfwGetWindowUserPointer(window));
            renderEngine->m_renderLogic->onWindowResized(width, height);
        });
    }

    // mouse
    {
        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y) {
            auto *renderEngine = static_cast<RenderEngine *>(glfwGetWindowUserPointer(window));
            renderEngine->m_mouseInput->onCursorPos(x, y);
            ImGui_ImplGlfw_CursorPosCallback(window, x, y);
        });
        glfwSetCursorEnterCallback(m_window, [](GLFWwindow *window, int entered) {
            auto *renderEngine = static_cast<RenderEngine *>(glfwGetWindowUserPointer(window));
            renderEngine->m_mouseInput->onCursorEnter(entered);
            ImGui_ImplGlfw_CursorEnterCallback(window, entered);
        });
        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mode) {
            auto *renderEngine = static_cast<RenderEngine *>(glfwGetWindowUserPointer(window));
            renderEngine->m_mouseInput->onMouseButton(button, action);
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mode);
        });
        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double deltaX, double deltaY) {
            auto *renderEngine = static_cast<RenderEngine *>(glfwGetWindowUserPointer(window));
            renderEngine->m_mouseInput->onScroll(deltaX, deltaY);
            ImGui_ImplGlfw_ScrollCallback(window, deltaX, deltaY);
        });
    }
}
