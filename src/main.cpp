#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "Animator.h"
#include "Camera.h"
#include "CameraHolder.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Instance.h"
#include "Model.h"
#include "Physics.h"
#include "PlayerState.h"
#include "Renderer.h"
#include "Terrain.h"
#include "imgui.h"
#include "lib/miniaudo.h"
#include "utils/Spline.h"
#include <__errc>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> // Include for quaternion operations
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <memory>

#define TERRAINX 100.0f
#define TERRAINZ 700.0f
float down = 0;
bool updown = false;
std::shared_ptr<Terrain> terrain;

bool enabled = false;

void glfwSetUpFunctions()
{

    if (!glfwInit())
        exit(1);

    const char* glsl_version = "#version 330";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        exit(1);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        exit(1);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

glm::quat rotationFromVectors(glm::vec3 start, glm::vec3 dest)
{
    start = glm::normalize(start);
    dest = glm::normalize(dest);

    float cosTheta = glm::dot(start, dest);
    glm::vec3 rotationAxis;

    if (cosTheta < -1 + 0.001f) {
        rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
        if (glm::length2(rotationAxis) < 0.01)
            rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
        rotationAxis = glm::normalize(rotationAxis);
        return glm::angleAxis(glm::radians(180.0f), rotationAxis);
    }

    rotationAxis = glm::cross(start, dest);
    float s = sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;

    return glm::quat(
        s * 1.0f,
        rotationAxis.x * invs,
        rotationAxis.y * invs,
        rotationAxis.z * invs);
}

float rotation = 0.0f;
bool opening;
bool cameraChange = false;
bool debug = false;
bool detectCollision(aiAABB boundingbox);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

struct YawPitch {
    float yaw;
    float pitch;
};

YawPitch calculateYawPitch(const glm::vec3& currentPosition, const glm::vec3& targetPosition)
{
    glm::vec3 direction = glm::normalize(targetPosition - currentPosition);

    float yaw = atan2(direction.x, direction.z);
    float pitch = atan2(-direction.y, glm::length(glm::vec2(direction.x, direction.z)));

    yaw = glm::degrees(yaw);
    pitch = glm::degrees(pitch);

    return { yaw, pitch };
}

void bindLightCube(unsigned int& VAO, unsigned int& VBO)
{
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window, std::shared_ptr<Terrain> terrain);
auto height = 2000;
CameraHolder cameraHolder;
std::shared_ptr<Camera> camera;
auto width = 3000;
int amount = 3000;
auto projection = glm::perspective(glm::radians(45.0f),
    (float)width / (float)height, 0.01f, 800.0f);
float deltaTime = 0;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 0;
float lastY = 0;
float accumulator = 0.0;
bool pressed = false;
bool insideCart = false;
physics::PhysicsWorld world;
PlayerState player;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
        cameraHolder.incrementCurrentCamera();
        switch (player.state) {
        case PlayerState::State::IDLE:
            player.state = PlayerState::State::FLYING;
            player.changed = true;
            break;
        case PlayerState::State::RUNNING:
            player.state = PlayerState::State::FLYING;
            player.changed = true;
            break;
        case PlayerState::State::FLYING:
            player.state = PlayerState::State::IDLE;
            player.changed = true;
            break;
        }
    }
    if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
        for (auto physobj : world.triggers) {
            switch (physobj->model->id) {
            case 10:
                insideCart = !insideCart;
                return;
            }
        }
    }

    if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
        for (auto triggers : world.triggers) {
            switch (triggers->model->id) {
            case 3:
                std::cout << "trigger";
                insideCart = !insideCart;
                world.triggers.clear();
                return;
            }
        }
    }
    if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
        if (debug) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        debug = !debug;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::IDLE;
    } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::IDLE;
    } else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::IDLE;
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        if (player.state != PlayerState::State::FLYING)
            player.state = PlayerState::State::IDLE;
    }
}

int main()
{
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(3000, 2000, "Spooky House", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return 1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    glEnable(GL_DEPTH_TEST);
    Camera fly = Camera { 606 };
    Camera fps = Camera { 101, true };
    cameraHolder.addCamera({ std::make_shared<Camera>(fly), std::make_shared<Camera>(fps) });
    camera = cameraHolder.getCam();
    CubicSpline cSpline = CubicSpline();

    Shader shader("../src/modelLoading.vert.glsl", "../src/modelLoading.frag.glsl");
    Shader treeShader("../src/tree.vert.glsl", "../src/tree.frag.glsl");
    Shader basic("../src/basic.vert.glsl", "../src/basic.frag.glsl");

    auto gun = std::make_shared<Model>("../assets/player/pistolbut.obj", glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0), 1, 0.0, 0.0, 0.0);
    auto scope = std::make_shared<Model>("../assets/player/pistolscope.obj", glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0), 2, 0.0, 0.0, 0.0);
    auto cart = std::make_shared<Model>("../assets/cart/cart.obj", glm::mat4(1.0f), glm::vec3(309, 3.0, 131.35), 3, 0.0, 80.0, 0.0);
    auto tree = std::make_shared<Model>("../assets/tree/spookyTree.obj", glm::mat4(1.0f), glm::vec3(0.0), 4, 0.0, 9166, 0.0, 0.0);
    auto left = std::make_shared<Model>("../assets/player/nleft.obj", glm::mat4(1.0f), glm::vec3(0.0), 5, 0.0, 10.0, 0.0);
    auto right = std::make_shared<Model>("../assets/player/nright.obj", glm::mat4(1.0f), glm::vec3(0.0), 6, 0.0, 10.0, 0.0);

    std::vector<glm::mat4> translations(amount);
    world = physics::PhysicsWorld();
    Terrain ter { 1, { "../assets/Water texture.png", "../assets/rock 01.jpg", "../assets/rock02 texture.jpg", "../assets/tilable img 0044 verydark.png" }, 5.0f };
    terrain = std::make_shared<Terrain>(ter);
    auto house = std::make_shared<Model>("../assets/house/hh.obj", glm::mat4(1.0f),
        glm::vec3(300.0, (-terrain->GetHeightInterpolated(300.0, 234.0) + 28.0f), 234),
        60, 92, 90.0, 0.0);
    initInstancedObject(amount, tree, translations, house->position, 560.0f, *terrain);
    auto track = std::make_shared<Model>("../assets/track/track.obj", glm::mat4(1.0f),
        glm::vec3(305.2f, house->position.y - 13.0, 177.5f), 111, 3.0, 82.5, -3.0);

    world.addCamera(camera, false, false);
    world.addModel(cart, false, true);

    glm::vec3 s1(206.98, track->position.y, 127.12);
    glm::vec3 s2 = glm::vec3(230.74, track->position.y + 4.0, 125.84); // Added in the gradual rise between s1 and s2
    glm::vec3 s3(249.50, track->position.y + 6.0, 124.56);
    glm::vec3 s4 = glm::vec3(270.93, track->position.y + 9.5, 126.205); // Enhances the gentle rise between s2 and s3
    glm::vec3 s5(292.36, track->position.y + 12.2, 127.85);
    glm::vec3 s6 = glm::vec3(302.73, track->position.y + 12.6, 129.285); // Smooths the transition before reaching s4
    glm::vec3 s7(313.10, track->position.y + 15.1, 130.72);
    glm::vec3 s8 = glm::vec3(337.80, track->position.y + 5, 134.61); // Midway through the drop from s4 to s5
    glm::vec3 s9(362.50, track->position.y + 2, 138.50);
    glm::vec3 s10 = glm::vec3(383.05, track->position.y - 2, 145.75); // Deepening the curve before leveling at s6
    glm::vec3 s11(403.6, track->position.y - 1.5, 153.0);
    glm::vec3 s12 = glm::vec3(414.02, track->position.y - 2.5, 159.2); // Approach the peak near s7
    glm::vec3 s13(420.60, track->position.y - 1.5, 163.4);
    glm::vec3 s14 = glm::vec3(428.02, track->position.y + 2, 177.8); // Just before the steep drop at s8
    glm::vec3 s15(427.44, track->position.y + 3, 196.20);
    glm::vec3 s101 = glm::vec3(422.15, track->position.y + 2, 214.0);
    glm::vec3 s16 = glm::vec3(406.02, track->position.y, 233.95); // On the descent towards s9
    glm::vec3 s17(370.64, track->position.y, 235.70);
    glm::vec3 s18 = glm::vec3(350.82, track->position.y, 234.85); // Transition between s9 and s10
    glm::vec3 s19(331.0, track->position.y, 234.0);
    glm::vec3 s20 = glm::vec3(293.55, track->position.y, 229.15);
    glm::vec3 s21(256.10, track->position.y, 224.3);
    glm::vec3 s22 = glm::vec3(226.205, track->position.y, 215.575);
    glm::vec3 s23(196.31, track->position.y, 206.85);
    glm::vec3 s24 = glm::vec3(175.75, track->position.y + 8, 185.785);
    glm::vec3 s25(176.19, track->position.y + 13.5, 152.72);
    glm::vec3 s26(185.66, track->position.y + 3.5, 136.12);
    glm::vec3 s27(206.98, track->position.y, 127.12);

    std::vector<glm::vec3> splinearray {
        s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s101, s16, s17, s18, s19, s20, s21, s22,
        s23, s24, s25, s26, s27
    };

    cSpline.m_points = splinearray;
    cSpline.InitializeSpline();

    unsigned int splineVAO;
    unsigned int splineVBO;
    glGenVertexArrays(1, &splineVAO);
    glGenBuffers(1, &splineVBO);
    glBindVertexArray(splineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
    glBufferData(GL_ARRAY_BUFFER, splinearray.size() * sizeof(glm::vec3), splinearray.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    Renderer renderer { projection, camera, *terrain };
    renderer.enqueue(shader, { track, house, cart });
    player = { left, right, gun };
    glm::vec3& lightPos = renderer.lightPos;

    //    for (int i = 0; i < cSpline.m_points.size(); i++) {
    //        auto pipe = std::make_shared<Model>("../assets/player/nleft.obj", glm::mat4(1.0f), splinearray[i], 4, 0.0, 0.0, 0.0);
    //        renderer.enqueue(shader, { pipe });
    //    }
    auto lastFrameTime = static_cast<float>(glfwGetTime());
    auto gunanim = initGunHouseAnimation(gun);
    bool xChange = false;
    bool yChange = false;
    bool zChange = false;
    bool position = false;
    bool py = false;
    int controlMode = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        camera = cameraHolder.getCam();
        renderer.cam = camera;
        if (player.state == PlayerState::State::FLYING && player.changed) {
            renderer.removeModel(shader.ID, left);
            renderer.removeModel(shader.ID, right);
            renderer.removeModel(shader.ID, gun);
            renderer.removeModel(shader.ID, scope);
            player.changed = false;
        } else if ((player.state == PlayerState::State::IDLE || player.state == PlayerState::State::RUNNING) && player.changed) {
            renderer.addModel(shader.ID, left);
            renderer.addModel(shader.ID, right);
            player.changed = false;
        }
        float currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        if (debug) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Checkbox("Position or Pitch + Yaw", &position);
            ImGui::Text("FPS %f", 60 / deltaTime);
            ImGui::Text("Camera position, X: %f Y: %f Z: %f", camera->position.x, camera->position.y, camera->position.z);
            ImGui::Text("Camera bb min, X: %f Y: %f Z: %f", camera->boundingBox.min.x, camera->boundingBox.min.y,
                camera->boundingBox.min.z);
            ImGui::Text("Camera bb max, X: %f Y: %f Z: %f", camera->boundingBox.max.x, camera->boundingBox.max.y,
                camera->boundingBox.max.z);

            ImGui::Text("house bb min, X: %f Y: %f Z: %f", cart->boundingbox.min.x, cart->boundingbox.min.y,
                house->boundingbox.min.z);
            ImGui::Text("house bb max, X: %f Y: %f Z: %f", cart->boundingbox.max.x, cart->boundingbox.max.y,
                house->boundingbox.max.z);
            if (position) {
                ImGui::Begin("Model Position Controls");
                ImGui::Text("Adjust the position of models:");
                ImGui::Checkbox("X Change", &xChange);
                ImGui::Checkbox("Y Change", &yChange);
                ImGui::Checkbox("Z Change", &zChange);
                auto positionControl = [&](const char* label, Model& model) {
                    if (xChange) {
                        ImGui::DragFloat((std::string(label) + " X").c_str(), &model.position.x, 0.1f, 0.0f, 0.0f, "%.3f");
                    }
                    if (yChange) {
                        ImGui::DragFloat((std::string(label) + " Y").c_str(), &model.position.y, 0.1f, 0.0f, 0.0f, "%.3f");
                    }
                    if (zChange) {
                        ImGui::DragFloat((std::string(label) + " Z").c_str(), &model.position.z, 0.1f, 0.0f, 0.0f, "%.3f");
                    }
                };

                auto lightControl = [&](const char* label, glm::vec3& pos) {
                    if (xChange) {
                        ImGui::DragFloat((std::string(label) + " X").c_str(), &pos.x, 0.1f, 0.0f, 0.0f, "%.3f");
                    }
                    if (yChange) {
                        ImGui::DragFloat((std::string(label) + " Y").c_str(), &pos.y, 0.1f, 0.0f, 0.0f, "%.3f");
                    }
                    if (zChange) {
                        ImGui::DragFloat((std::string(label) + " Z").c_str(), &pos.z, 0.1f, 0.0f, 0.0f, "%.3f");
                    }
                };
                positionControl("Cart", *cart);
                positionControl("Track", *track);
                positionControl("Tree", *tree);
                positionControl("House", *house);
                positionControl("Left", *left);
                positionControl("Right", *right);
                lightControl("Light", lightPos);
                ImGui::End();
                ImGui::Render();
            } else {
                ImGui::Begin("Control Model Pitch and Yaw");
                ImGui::Text("Adjust the orientation of models:");
                ImGui::RadioButton("Pitch", &controlMode, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Yaw", &controlMode, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Roll", &controlMode, 2);

                auto pitchControl = [&](const char* label, Model& model) {
                    switch (controlMode) {
                    case 0:
                        ImGui::DragFloat((std::string(label) + " Pitch").c_str(), &model.pitch, 0.1f, -360.0f, 360.0f,
                            "%.3f");
                        break;
                    case 1:
                        ImGui::DragFloat((std::string(label) + " Yaw").c_str(), &model.yaw, 0.1f, -360.0f, 360.0f,
                            "%.3f");
                        break;
                    case 2:
                        ImGui::DragFloat((std::string(label) + " Roll").c_str(), &model.roll, 0.1f, -360.0f, 360.0f,
                            "%.3f");
                        break;
                    }
                };

                pitchControl("Cart", *cart);
                pitchControl("Track", *track);
                pitchControl("Tree", *tree);
                pitchControl("House", *house);
                pitchControl("Left", *left);
                pitchControl("Right", *right);
                pitchControl("Gun", *gun);
                pitchControl("scope", *scope);
                ImGui::End();
                ImGui::Render();
            }
        }
        /* ----------- OPENGL ----------------- */
        glClearColor(0.4f, 0.1f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.renderAll();
        world.tick(deltaTime, *terrain);
        float totalDuration = 10.0f;
        if (insideCart) {
            float currentTime = fmod(currentFrameTime, totalDuration);
            float normalizedTime = (currentTime / totalDuration) * (cSpline.m_points.size() - 1);
            auto newPos = cSpline.ConstVelocitySplineAtTime(currentTime * 60);
            auto pitch = calculateYawPitch(cart->position, newPos);
            auto pitchDif = pitch.pitch - cart->pitch;
            auto yawDif = cart->yaw - pitch.yaw;
            cart->position = newPos;
            cart->pitch = pitch.pitch;
            cart->yaw = pitch.yaw;
            world.updatePosition(10, newPos);
            camera->position = cart->position;
            camera->options.yaw += yawDif;
            camera->options.pitch += pitchDif;
            camera->position.y += 5.0f;
            camera->update();
        }
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
        terrain->terrainShader.use();
        terrain->terrainShader.setMat4("projection", projection);
        terrain->terrainShader.setMat4("view", camera->getCameraView());
        terrain->terrainShader.setMat4("model", glm::translate(glm::mat4(1.0), terrain->terposition));
        terrain->terrainShader.setVec3("lightDir", lightPos);
        terrain->terrainShader.setVec3("lightColor", glm::vec3(1.0f));
        terrain->terrainShader.setVec3("viewPos", camera->position);
        terrain->terrainShader.setMat4("model", glm::translate(glm::mat4(1.0), terrain->terposition));
        terrain->terrainShader.setFloat("minHeight", terrain->minHeight);
        terrain->terrainShader.setFloat("maxHeight", terrain->maxHeight);
        if (down > 3.0) {
            updown = true;
        }
        if (down < 0) {
            updown = false;
        }

        terrain->render();
        treeShader.use();
        renderer.lightingShader(treeShader);
        treeShader.setInt("texture_diffuse1", 0);
        treeShader.setFloat("shininess", 3);
        treeShader.setMat4("projection", projection);
        treeShader.setMat4("view", camera->getCameraView());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tree->textures_loaded[0].id);
        for (unsigned int i = 0; i < tree->meshes.size(); i++) {
            glBindVertexArray(tree->meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(tree->meshes[i].indices.size()),
                GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }

        basic.use();
        basic.setMat4("projection", projection);
        basic.setMat4("view", camera->getCameraView());
        basic.setMat4("model", glm::mat4(1.0f));
        basic.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(splineVAO);
        glDrawArrays(GL_LINE_STRIP, 0, sizeof(splinearray));
        glBindVertexArray(0);
        processInput(window, terrain);
        if (debug) {
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        glfwSwapBuffers(window);
        player.tick(deltaTime, camera);
        gun->position.x = camera->position.x + camera->front.x * 2.0f + -camera->right.x * 1.0f;
        gun->position.z = camera->position.z + camera->front.z * 2.0f + -camera->right.x * 1.0f;
        gun->position.y += camera->position.y - gun->position.y - 0.8f;
        gunanim->update(deltaTime);

        scope->position.x = camera->position.x + camera->front.x * 2.0f + -camera->right.x * 1.0f;
        scope->position.z = camera->position.z + camera->front.z * 2.0f + -camera->right.x * 1.0f;
        scope->position.y += camera->position.y - scope->position.y - 0.8f;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, std::shared_ptr<Terrain> terrain)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->processKeyboard(Camera::Movement::FORWARD, deltaTime, true, terrain);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->processKeyboard(Camera::Movement::BACKWARD, deltaTime, true, terrain);
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->processKeyboard(Camera::Movement::LEFT, deltaTime, true, terrain);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->processKeyboard(Camera::Movement::RIGHT, deltaTime, true, terrain);
    } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
        camera->processKeyboard(Camera::Movement::FORWARD, deltaTime, false, terrain);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
        camera->processKeyboard(Camera::Movement::BACKWARD, deltaTime, false, terrain);
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
        if (player.state != PlayerState::State::FLYING)
            camera->processKeyboard(Camera::Movement::LEFT, deltaTime, false, terrain);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
        if (player.state != PlayerState::State::FLYING)
            camera->processKeyboard(Camera::Movement::RIGHT, deltaTime, false, terrain);
    }
}

// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f),
        (float)width / (float)height, 0.001f, 200.0f);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (enabled)
        return;
    auto xpos = static_cast<float>(xposIn);
    auto ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    camera->processMouseMovement(xoffset, yoffset);
}

/*  if (cam.hold[cam.currentCameraIndex].position.x >= model.boundingbox.mMin.x && cam.hold[cam.currentCameraIndex].position.x <= model.boundingbox.mMax.x && cam.hold[cam.currentCameraIndex].position.y >= model.boundingbox.mMin.y && cam.hold[cam.currentCameraIndex].position.y <= model.boundingbox.mMax.y && cam.hold[cam.currentCameraIndex].position.z >= model.boundingbox.mMin.z && cam.hold[cam.currentCameraIndex].position.z <= model.boundingbox.mMax.z) {

      std::vector<float> arr = {
          // xmin
          std::abs(model.boundingbox.mMin.x - cam.hold[cam.currentCameraIndex].position.x),
          // xmax
          std::abs(model.boundingbox.mMax.x - cam.hold[cam.currentCameraIndex].position.x),
          // ymin
          std::abs(model.boundingbox.mMin.y - cam.hold[cam.currentCameraIndex].position.y),
          // ymax
          std::abs(model.boundingbox.mMax.y - cam.hold[cam.currentCameraIndex].position.y),
          // zmix
          std::abs(model.boundingbox.mMin.z - cam.hold[cam.currentCameraIndex].position.z),
          // zmax
          std::abs(model.boundingbox.mMax.z - cam.hold[cam.currentCameraIndex].position.z)
      };
int irr = 0;
      float max = arr[0];
      for (int i = 1; i < arr.size(); i++) {
          if (arr[i] < max) {
              max = arr[i];
              irr = i;
          }
      }

      switch (irr) {
      case 0:
          cam.hold[cam.currentCameraIndex].position.x = model.boundingbox.mMin.x - 0.0000001;
          break;
      case 1:
          cam.hold[cam.currentCameraIndex].position.x = model.boundingbox.mMax.x + 0.0000001;
          break;
      case 2:
          cam.hold[cam.currentCameraIndex].position.y = model.boundingbox.mMin.y - 0.0000001;
          break;
      case 3:
          cam.hold[cam.currentCameraIndex].position.y = model.boundingbox.mMax.y + 0.0000001;
          break;
      case 4:
          cam.hold[cam.currentCameraIndex].position.z = model.boundingbox.mMin.z - 0.0000001;
          break;
      case 5:
          cam.hold[cam.currentCameraIndex].position.z = model.boundingbox.mMax.z + 0.0000001;
          break;
      }

      cam.hold[cam.currentCameraIndex].update();
      return true; // Collision detected
  }

  return false; // No collision detected
}

* */
