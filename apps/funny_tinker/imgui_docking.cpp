//
// Created by niek on 4/15/2025.
//

#include <camera.h>
#include <shader.h>

#include <iostream>
#include <ostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

void setupImGUIDocking();
void renderImGUIWindows(unsigned int texture_color_buffer);

// settings
constexpr unsigned int SCR_WIDTH = 1920;
constexpr unsigned int SCR_HEIGHT = 1080;
int lastAltState = GLFW_RELEASE;

// Camera
Camera camera{
    glm::vec3(0.0f, 0.0f, 3.0f)
};
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = false;
bool isCursorLocked = false;

float cameraSpeedMultiplier = 3.0f;
float cameraFov = camera.Zoom;
float backgroundColor[3] = {0.2f, 0.2f, 0.2f};

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// Material properties
float shininess = 64.0f;
float tintStrength = 1.0f;
float emissionStrength = 1.0f;
float color[3] = { 1.0f, 1.0f, 1.0f };

float lightColorValues[3] = { 1.0f, 1.0f, 1.0f };
float lightAmbient[3] = { 0.2f, 0.2f, 0.2f };
float lightDiffuse[3] = { 0.5f, 0.5f, 0.5f };

// rendering flags
bool showDemoWindow = false;
bool showCube = true;
bool showLight = true;

int main() {

    // glfw initialise
    // ---------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw init window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ImGUI Docking Test", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(0);  // disable vsync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // tell glfw to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        glfwDestroyWindow(window);
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // set up ImGui style
    ImGui::StyleColorsDark();

    // when viewports are enabled, we need to tweak window rounding/-bg
    // ---------------------------------------------------------------------
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // configure global open gl state
    // ------------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile the shader program
    const Shader lightingShader("resources/shaders/material.vert", "resources/shaders/material.frag");
    const Shader lightCubeShader("resources/shaders/lighting/lighting_cube.vert", "resources/shaders/lighting/lighting_cube.frag");

    // set up cube vertices
    constexpr float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // configure cube VAO and VBO
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // tex coords
    // diffuse
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // specular
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // emission
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(6 * sizeof(float)));
    glEnableVertexAttribArray(4);

    // configure light VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    // load textures
    // ---------------
    const unsigned int diffuseMap = loadTexture("resources/textures/container2.png");
    const unsigned int specularMap = loadTexture("resources/textures/container2_specular.png");
    const unsigned int emissionMap = loadTexture("resources/textures/matrix.jpg");

    // shader config
    // ---------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setInt("material.emission", 2);

    // custom framebuffer for scene rendering
    // ----------------------------------------
    unsigned int frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    // color attachment texture
    // ----------------------------------------
    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureColorBuffer, 0);

    // renderbuffer for depth and stencil
    // ---------------------------------
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER::FRAMEBUFFER is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render loop
    // -----------------
    while (!glfwWindowShouldClose(window)) {

        // per-frame time logic
        // --------------------
        const auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (showCube) {
            // be sure to activate shader when setting uniforms/drawing objects
            lightingShader.use();
            lightingShader.setVec3("light.position", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);

            // light properties
            lightingShader.setVec3("light.ambient", lightAmbient[0], lightAmbient[1], lightAmbient[2]);
            lightingShader.setVec3("light.diffuse", lightDiffuse[0], lightDiffuse[1], lightDiffuse[2]);
            lightingShader.setVec3("light.specular", lightColorValues[0], lightColorValues[1], lightColorValues[2]);

            // material properties
            lightingShader.setFloat("material.shininess", shininess);
            lightingShader.setVec3("material.color", glm::vec3(color[0], color[1], color[2]));
            lightingShader.setFloat("material.tintStrength", tintStrength);
            lightingShader.setFloat("material.emissionStrength", emissionStrength);

            // view/projection transformations
            glm::mat4 projection = glm::perspective(
                glm::radians(camera.Zoom),
                static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
                0.1f,
                100.0f
            );
            glm::mat4 view = camera.GetViewMatrix();

            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);

            // world transformation
            auto model = glm::mat4(1.0f);
            lightingShader.setMat4("model", model);

            // bind diffuse map
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularMap);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, emissionMap);

            // render the cube
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        if (showLight) {
            glm::mat4 projection = glm::perspective(
                glm::radians(camera.Zoom),
                static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
                0.1f,
                100.0f
            );
            glm::mat4 view = camera.GetViewMatrix();

            // also draw the lamp object
            lightCubeShader.use();
            lightCubeShader.setMat4("projection", projection);
            lightCubeShader.setMat4("view", view);
            lightCubeShader.setVec3("lightColor", glm::vec3(lightColorValues[0], lightColorValues[1], lightColorValues[2]));

            auto model = glm::mat4(1.0f);
            model = translate(model, lightPos);
            model = scale(model, glm::vec3(0.2f)); // a smaller cube
            lightCubeShader.setMat4("model", model);

            glBindVertexArray(lightCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // imgui: initialise
        // ----------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // imgui: setup docking environment
        // --------------------------------
        setupImGUIDocking();
        renderImGUIWindows(textureColorBuffer);

        // show demo window
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        // imgui: render
        // -------------------
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // imgui: update and render
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(1, &textureColorBuffer);
    glDeleteRenderbuffers(1, &rbo);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow *window) {
    float speedMultiplier{ 1.0f };

    // Get current Alt key state
    const int currentAltState = glfwGetKey(window, GLFW_KEY_LEFT_ALT);

    // Check for single press (key was released before and is now pressed)
    if (currentAltState == GLFW_PRESS && lastAltState == GLFW_RELEASE) {
        // Toggle cursor lock
        isCursorLocked = !isCursorLocked;

        // Update cursor mode based on lock state
        if (isCursorLocked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // Reset first mouse
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // Store current state for next frame
    lastAltState = currentAltState;

    // early return if cursor isn't locked
    if (!isCursorLocked) return;

    // move faster while shift is being held
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speedMultiplier = cameraSpeedMultiplier;

    // stop app
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime * speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime * speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime * speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime * speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime * speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime * speedMultiplier);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow*, const int width, const int height) {
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow*, const double xPosIn, const double yPosIn) {
    if (!isCursorLocked) return;

    const auto x_pos = static_cast<float>(xPosIn);
    const auto y_pos = static_cast<float>(yPosIn);

    if (firstMouse) {
        lastX = x_pos;
        lastY = y_pos;
        firstMouse = false;
    }

    const float xOffset = x_pos - lastX;
    const float yOffset = lastY - y_pos; // reversed since y-coordinates go from bottom to top

    lastX = x_pos;
    lastY = y_pos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    if (unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0))
    {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// imgui: setting up all the necessary docking properties
// -------------------------------------------------------
void setupImGUIDocking() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.2f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.8f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Set up the menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "Esc")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Demo Window", nullptr, &showDemoWindow);
            ImGui::MenuItem("Show Cube", nullptr, &showCube);
            ImGui::MenuItem("Show Light", nullptr, &showLight);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Camera")) {
            if (ImGui::MenuItem("Lock/Unlock Camera", "Alt")) {
                isCursorLocked = !isCursorLocked;
                glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR,
                                isCursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                firstMouse = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // DockSpace
    const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    ImGui::End();
}

// imgui: render all the windows ImGUI has created
// -----------------------------------------------------------------
void renderImGUIWindows(const unsigned int texture_color_buffer) {
    // Scene Viewport
    ImGui::Begin("Viewport");
    // Calculate the size to maintain aspect ratio
    const float availableWidth = ImGui::GetContentRegionAvail().x;
    const float availableHeight = ImGui::GetContentRegionAvail().y;

    // Desired aspect ratio: 16:9
    constexpr float aspectRatio = 16.0f / 9.0f;

    // Calculate target size based on aspect ratio
    float viewportWidth = availableWidth;
    float viewportHeight = viewportWidth / aspectRatio;

    // Clamp to available height if needed
    if (viewportHeight > availableHeight) {
        viewportHeight = availableHeight;
        viewportWidth = viewportHeight * aspectRatio;
    }

    // Center the image in the available space
    const ImVec2 cursorPos = ImGui::GetCursorPos();
    const ImVec2 offset{
        (availableWidth - viewportWidth) * 0.5f,
        (availableHeight - viewportHeight) * 0.5f
    };
    ImGui::SetCursorPos(ImVec2(cursorPos.x + offset.x, cursorPos.y + offset.y));

    // Display the image with the correct size
    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(texture_color_buffer)),
                 ImVec2(viewportWidth, viewportHeight),
                 ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    // Material Editor
    ImGui::Begin("Material Properties");
    ImGui::ColorEdit3("Object Color", color);
    ImGui::SliderFloat("Shininess", &shininess, 1.0f, 256.0f);
    ImGui::SliderFloat("Tint Strength", &tintStrength, 0.0f, 2.0f);
    ImGui::SliderFloat("Emission Strength", &emissionStrength, 0.0f, 10.0f);
    ImGui::End();

    // Light Editor
    ImGui::Begin("Light Properties");
    ImGui::ColorEdit3("Light Color", lightColorValues);
    ImGui::ColorEdit3("Ambient", lightAmbient);
    ImGui::ColorEdit3("Diffuse", lightDiffuse);

    ImGui::Text("Light Position");
    ImGui::SliderFloat("X", &lightPos.x, -5.0f, 5.0f);
    ImGui::SliderFloat("Y", &lightPos.y, -5.0f, 5.0f);
    ImGui::SliderFloat("Z", &lightPos.z, -5.0f, 5.0f);
    ImGui::End();

    ImGui::Begin("Camera Settings");
    ImGui::SliderFloat("Speed Multiplier", &cameraSpeedMultiplier, 0.5f, 10.0f);
    if (ImGui::SliderFloat("FOV", &cameraFov, 0.0f, 120.0f))
        camera.Zoom = cameraFov;
    ImGui::ColorEdit3("Background Color", backgroundColor);
    ImGui::End();

    // Statistics Window
    ImGui::Begin("Statistics");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
    ImGui::End();

    // Controls Window
    ImGui::Begin("Controls");
    ImGui::Text("Navigation:");
    ImGui::BulletText("WASD - Move camera");
    ImGui::BulletText("Q/E - Move up/down");
    ImGui::BulletText("Mouse - Look around (when locked)");
    ImGui::BulletText("Alt - Toggle mouse lock");
    ImGui::BulletText("Shift - Move faster");
    ImGui::BulletText("Esc - Exit application");

    ImGui::Separator();
    ImGui::End();
}
