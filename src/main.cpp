#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
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
#include "utils/Spline.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
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

void glfwSetUpFunctions() {
    if (!glfwInit())
        exit(1);
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Spooky", nullptr, nullptr);
    if (window == nullptr)
        exit(1);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        exit(1);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

glm::quat rotationFromVectors(glm::vec3 start, glm::vec3 dest) {
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

    return {
        s * 1.0f,
        rotationAxis.x * invs,
        rotationAxis.y * invs,
        rotationAxis.z * invs
    };
}

float rotation = 0.0f;
bool opening;
bool cameraChange = false;
bool debug = false;

bool detectCollision(aiAABB boundingbox);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

struct YawPitch {
    float yaw;
    float pitch;
};

YawPitch calculateYawPitch(const glm::vec3 &currentPosition, const glm::vec3 &targetPosition) {
    glm::vec3 direction = glm::normalize(targetPosition - currentPosition);

    float yaw = atan2(direction.x, direction.z);
    float pitch = atan2(-direction.y, glm::length(glm::vec2(direction.x, direction.z)));

    yaw = glm::degrees(yaw);
    pitch = glm::degrees(pitch);

    return {yaw, pitch};
}

void bindShadowFramebuffer(unsigned int framebuffer, unsigned int shadowWidth, unsigned int shadowHeight) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, shadowWidth, shadowHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void unbindFramebuffer(int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

void bindTextures(unsigned int depthMap) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap); // Bind the shadow map texture
}

void setTerrainShaderUniforms(Shader &shader, const glm::mat4 &projection, const glm::mat4 &view,
                              const glm::mat4 &model, const glm::vec3 &lightDir, const glm::vec3 &lightColor,
                              const glm::vec3 &viewPos, float minHeight, float maxHeight,
                              const glm::mat4 &lightSpaceMatrix) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);
    shader.setVec3("lightDir", lightDir);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("viewPos", viewPos);
    shader.setFloat("minHeight", minHeight);
    shader.setFloat("maxHeight", maxHeight);
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.setInt("shadowMap", 4); // Set the sampler uniform to texture unit 4
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);

void processInput(GLFWwindow *window, const std::shared_ptr<Terrain> &terrain);

int height;
CameraHolder cameraHolder;
std::shared_ptr<Camera> camera;
int width;
int amount = 5000;
glm::mat4 projection;
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

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
        player.state = PlayerState::State::TORCH,
                player.changed = true;
        return;
    }

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
        for (auto &physobj: world.getTriggers(*camera->boundingBox)) {
            switch (physobj->model->id) {
                case 1:
                case 2:
                    player.state = PlayerState::State::GUN;
                    player.changed = true;
                    break;
                case 3:
                    insideCart = !insideCart;
                    break;
                case 65:
                    player.state = player.state == PlayerState::State::LADDER
                                       ? PlayerState::State::IDLE
                                       : PlayerState::State::LADDER;
                    player.changed = true;
                    break;
                default:
                    break;
            }
        }
        world.triggers.clear();
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
        if (player.state == PlayerState::State::IDLE)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        if (player.state == PlayerState::State::RUNNING)
            player.state = PlayerState::State::IDLE;
    } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        if (player.state == PlayerState::State::IDLE)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        if (player.state == PlayerState::State::RUNNING)
            player.state = PlayerState::State::IDLE;
    } else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        if (player.state == PlayerState::State::IDLE)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        if (player.state == PlayerState::State::RUNNING)
            player.state = PlayerState::State::IDLE;
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        if (player.state == PlayerState::State::IDLE)
            player.state = PlayerState::State::RUNNING;
    } else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        if (player.state == PlayerState::State::RUNNING)
            player.state = PlayerState::State::IDLE;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        switch (player.state) {
            case PlayerState::State::GUN:
                player.shoot();
                world.fireBullet(camera->position, camera->front, 10.0f);
                break;
            case PlayerState::State::TORCH:
                player.torchOn = !player.torchOn;
                break;
        }
    }
}

int main() {
    if (!glfwInit())
        return 1;
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
    width = mode->width;
    height = mode->height;
    GLFWwindow *window = glfwCreateWindow(width, height, "Spooky House", nullptr, nullptr);

    projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.01f,
                                  1000.0f);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return 1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    glEnable(GL_DEPTH_TEST);
    Camera fly = Camera{606};
    Camera fps = Camera{101, true};
    cameraHolder.addCamera({std::make_shared<Camera>(fps), std::make_shared<Camera>(fly)});
    camera = cameraHolder.getCam();
    CubicSpline cSpline = CubicSpline();

    Shader shader("../src/modelLoading.vert.glsl", "../src/modelLoading.frag.glsl");
    Shader depth("../src/depthShader.vert.glsl", "../src/depthShader.frag.glsl");
    Shader basic("../src/basic.vert.glsl", "../src/basic.frag.glsl");
    auto treeOne = std::make_shared<Model>("../assets/tree/treeOne.obj", glm::mat4(1.0f), glm::vec3(0.0), 202, 0.0,
                                           9166, 0.0, 0.0);
    auto treeTwo = std::make_shared<Model>("../assets/tree/treeTwo.obj", glm::mat4(1.0f), glm::vec3(0.0), 203, 0.0,
                                           9166, 0.0, 0.0);
    auto treeThree = std::make_shared<Model>("../assets/tree/treeThree.obj", glm::mat4(1.0f), glm::vec3(0.0), 204, 0.0,
                                             9166, 0.0, 0.0);
    auto treeFour = std::make_shared<Model>("../assets/tree/treeFour.obj", glm::mat4(1.0f), glm::vec3(0.0), 205, 0.0,
                                            9166, 0.0, 0.0);
    auto treeFive = std::make_shared<Model>("../assets/tree/treeFive.obj", glm::mat4(1.0f), glm::vec3(0.0), 206, 0.0,
                                            9166, 0.0, 0.0);
    auto treeSix = std::make_shared<Model>("../assets/tree/treeSix.obj", glm::mat4(1.0f), glm::vec3(0.0), 207, 0.0,
                                           9166, 0.0, 0.0);
    auto left = std::make_shared<Model>("../assets/player/nleft.obj", glm::mat4(1.0f), glm::vec3(0.0), 5, 0.0, 10.0,
                                        0.0);
    auto right = std::make_shared<Model>("../assets/player/nright.obj", glm::mat4(1.0f), glm::vec3(0.0), 6, 0.0, 10.0,
                                         0.0);

    auto torch = std::make_shared<Model>("../assets/player/torch.obj", glm::mat4(1.0f), glm::vec3(0.0), 5, 0.0, 0.0,
                                         0.0);
    std::vector<std::vector<glm::mat4> > translations{5};
    world = physics::PhysicsWorld();
    Terrain ter{
        1,
        {
            "../assets/Water texture.png", "../assets/rock 01.jpg", "../assets/rock02 texture.jpg",
            "../assets/tilable img 0044 verydark.png"
        },
        5.0f
    };
    terrain = std::make_shared<Terrain>(ter);

    auto gun = std::make_shared<Model>("../assets/player/pistolbut.obj", glm::mat4(1.0f),
                                       glm::vec3(10, -terrain->GetHeightInterpolated(10, 10) + 1.0f, 10), 1, 0.0,
                                       0.0, 0.0);
    auto scope = std::make_shared<Model>("../assets/player/pistolscope.obj", glm::mat4(1.0f),
                                         glm::vec3(10, -terrain->GetHeightInterpolated(375, 109) + 1.0f, 109), 10, 0.0,
                                         0.0, 0.0);
    auto platform = std::make_shared<Model>("../assets/house/platform.obj", glm::mat4(1.0f),
                                            glm::vec3(250.0, (-terrain->GetHeightInterpolated(250.0, 200.0) + 50.0f),
                                                      200),
                                            64, 0, 0, 0.0);

    auto house = std::make_shared<Model>("../assets/house/hh.obj", glm::mat4(1.0f),
                                         glm::vec3(300.0, platform->position.y + 20.0f, 234),
                                         60, 90, 90, 0.0);
    auto lampOne = std::make_shared<Model>("../assets/lamps/lampOne.obj", glm::mat4(1.0f),
                                           glm::vec3(416.0f, platform->position.y - 7.0f, 100.0f), 301, 0.0, 0.0,
                                           0.0);
    auto lampTwo = std::make_shared<Model>("../assets/lamps/lampTwo.obj", glm::mat4(1.0f),
                                           glm::vec3(174.0f, platform->position.y - 7.0f, 324.0f), 302, 0.0, 0.0,
                                           0.0);
    auto lampThree = std::make_shared<Model>("../assets/lamps/lampThree.obj", glm::mat4(1.0f),
                                             glm::vec3(217.0f, platform->position.y - 7.0f, 88.0f), 303, 0.0,
                                             0.0, 0.0);

    auto ladder = std::make_shared<Model>("../assets/house/ladder.obj", glm::mat4(1.0f),
                                          glm::vec3(130, terrain->GetHeightInterpolated(130, 199) - 20.0f, 199), 65,
                                          0.0, 90, 0.0f);

    initMultiTree(amount, 6, 250, 560.0, translations, house->position, *terrain);
    auto track = std::make_shared<Model>("../assets/track/track.obj", glm::mat4(1.0f),
                                         glm::vec3(305.2f, house->position.y - 13.0, 177.5f), 111, 3.0, 82.5, -3.0);
    auto cart = std::make_shared<Model>("../assets/cart/cart.obj", glm::mat4(1.0f),
                                        glm::vec3(206.98, track->position.y + 10.0f, 127), 3, 0.0, 80.0, 0.0);

    treeOne->initInstanced(translations[0].size(), translations[0]);
    treeTwo->initInstanced(translations[1].size(), translations[1]);
    treeThree->initInstanced(translations[2].size(), translations[2]);
    treeFour->initInstanced(translations[3].size(), translations[3]);
    treeFive->initInstanced(translations[4].size(), translations[4]);
    treeSix->initInstanced(translations[5].size(), translations[5]);
    world.addCamera(camera, true, false, false);
    cameraHolder.incrementCurrentCamera();
    camera = cameraHolder.getCam();
    world.addCamera(camera, true, false, false);
    cameraHolder.incrementCurrentCamera();
    world.addModel(house, false, false, true);
    world.addModel(cart, false, true, true);
    world.addModel(gun, false, true, true);
    world.addModel(ladder, false, true, true);
    // world.addModel(house, false, false, true);
    glm::vec3 s1(207.5f, track->position.y + 2, 133.0f);
    glm::vec3 s2(243.39f, track->position.y + 5.7f, 130.45f); // Added in the gradual rise between s1 and s2
    glm::vec3 s3(269.50f, track->position.y + 10.5f, 130.56f);
    glm::vec3 s4(286.3f, track->position.y + 12.9f, 131.205f); // Enhances the gentle rise between s2 and s3
    glm::vec3 s5(300.36f, track->position.y + 14.4f, 132.09f);
    glm::vec3 s6(314.73f, track->position.y + 14.6f, 134.285f); // Smooths the transition before reaching s4
    glm::vec3 s7(322.30f, track->position.y + 13.8f, 135.42f);
    glm::vec3 s8(339.20f, track->position.y + 11.0f, 137.61f); // Midway through the drop from s4 to s5
    glm::vec3 s9(351.31f, track->position.y + 8.5f, 138.90f);
    glm::vec3 s10(383.05f, track->position.y - 0.9f, 145.75f); // Deepening the curve before leveling at s6
    glm::vec3 s11(403.6f, track->position.y - 3.8f, 153.0f);
    glm::vec3 s12(414.02f, track->position.y - 3.9f, 159.2f); // Approach the peak near s7
    glm::vec3 s13(420.60f, track->position.y - 0.5, 163.4f);
    glm::vec3 s14(428.02f, track->position.y + 3.5f, 177.8f); // Just before the steep drop at s8
    glm::vec3 s15(427.44f, track->position.y + 5.0f, 196.20f);
    glm::vec3 s16(422.15f, track->position.y + 2.0f, 214.0f);
    glm::vec3 s17(403.02f, track->position.y, 225.95f); // On the descent towards s9
    glm::vec3 s18(370.64f, track->position.y - 1.0f, 231.70f);
    glm::vec3 s19(350.82f, track->position.y - 1.0f, 230.85f); // Transition between s9 and s10
    glm::vec3 s20(331.0f, track->position.y - 1.0f, 230.0f);
    glm::vec3 s21(299.55f, track->position.y + 1.0, 228.15f);
    glm::vec3 s22(256.10f, track->position.y + 1.0, 222.3f);
    glm::vec3 s23(226.205f, track->position.y + 1.0, 215.575f);
    glm::vec3 s24(181.02f, track->position.y + 8.2f, 198.14f);
    glm::vec3 s25(174.75f, track->position.y + 19.0f, 168.785f);
    glm::vec3 s26(177.0f, track->position.y + 17.0f, 158.72f);
    glm::vec3 s27(183.0f, track->position.y + 9.2f, 146.0f);
    glm::vec3 s28(190.0f, track->position.y + 5.0f, 140.0f);
    glm::vec3 s29(199.5f, track->position.y + 1.7f, 134.86f);
    glm::vec3 s30(207.5f, track->position.y + 2.0, 133.0f);
    std::vector<glm::vec3> splinearray{
        s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, s21, s22,
        s23, s24, s25, s26, s27, s28, s29, s30
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) nullptr);
    Renderer renderer{projection, camera, *terrain};
    renderer.enqueue(shader, {
                         track, house, cart, left, right, gun, scope, platform, treeOne, treeTwo, treeFour, treeFive,
                         treeSix, lampOne, lampTwo, lampThree, ladder
                     });
    renderer.addLampPointLight(lampOne->position);
    renderer.addLampPointLight(lampTwo->position);
    renderer.addLampPointLight(lampThree->position);
    player = {left, right, gun, scope, torch};
    glm::vec3 &lightPos = renderer.lightPos;
    auto lastFrameTime = static_cast<float>(glfwGetTime());
    bool xChange = false;
    bool yChange = false;
    bool zChange = false;
    bool position = false;
    bool py = false;
    int controlMode = 0;

    // Shadows setup
    // -----------------------
    const unsigned int SHADOW_WIDTH = 3000, SHADOW_HEIGHT = 3000;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int lightningCounter = 1;
    int lightning = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (player.isShooting) {
            camera->update();
            player.isShooting = false;
        }
        camera = cameraHolder.getCam();
        renderer.cam = camera;
        renderer.torchPos = player.torch->position;
        renderer.torch = player.torchOn;
        /*
        int randAmount = 80;
        if (renderer.lightning) {
            if (randAmount < 2 || rand() % (randAmount / lightningCounter) == 0) {
                renderer.lightningSwitch();
                lightningCounter = 1;
            } else {
                lightningCounter++;
            }
        }
        if (rand() % randAmount < 1) {
            renderer.lightningSwitch();
        }
        */
        glm::mat4 lightView = glm::lookAt(glm::vec3(-10.0f, 100.0f, -10), glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));

        float near_plane = 1.0f, far_plane = 100.0f;
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, -20.0f, near_plane, far_plane);
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        renderer.lightSpaceMatrix = lightSpaceMatrix;
        if (player.state == PlayerState::State::FLYING && player.changed) {
            renderer.removeModel(shader.ID, left);
            renderer.removeModel(shader.ID, right);
            renderer.removeModel(shader.ID, gun);
            renderer.removeModel(shader.ID, scope);
            player.changed = false;
        } else if ((player.state == PlayerState::State::IDLE || player.state == PlayerState::State::RUNNING) && player.
                   changed) {
            renderer.addModel(shader.ID, left);
            renderer.addModel(shader.ID, right);
            player.changed = false;
        } else if (player.state == PlayerState::State::GUN && player.changed) {
            renderer.removeModel(shader.ID, left);
            renderer.removeModel(shader.ID, right);
            player.changed = false;
        } else if (player.state == PlayerState::State::TORCH && player.changed) {
            renderer.removeModel(shader.ID, left);
            renderer.removeModel(shader.ID, right);
            renderer.addModel(shader.ID, torch);
            player.changed = false;
        } else if (player.state == PlayerState::State::LADDER && player.changed) {
            renderer.removeModel(shader.ID, left);
            renderer.removeModel(shader.ID, right);
            renderer.removeModel(shader.ID, torch);
            player.changed = false;
        }
        auto currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        if (debug) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Checkbox("Position or Pitch + Yaw", &position);
            ImGui::Text("FPS %f", 60 / deltaTime);
            ImGui::Text("Camera position, X: %f Y:otherDungeon %f Z: %f", camera->position.x, camera->position.y,
                        camera->position.z);
            ImGui::Text("Camera bb min, X: %f Y: %f Z: %f", camera->boundingBox->min.x, camera->boundingBox->min.y,
                        camera->boundingBox->min.z);
            ImGui::Text("Camera bb max, X: %f Y: %f Z: %f", camera->boundingBox->max.x, camera->boundingBox->max.y,
                        camera->boundingBox->max.z);
            ImGui::Text("house bb min, X: %f Y: %f Z: %f", house->boundingbox->min.x, house->boundingbox->min.y,
                        house->boundingbox->min.z);
            ImGui::Text("house bb max, X: %f Y: %f Z: %f", house->boundingbox->max.x, house->boundingbox->max.y,
                        house->boundingbox->max.z);


            if (position) {
                ImGui::Begin("Model Position Controls");
                ImGui::Text("Adjust the position of models:");
                ImGui::Checkbox("X Change", &xChange);
                ImGui::Checkbox("Y Change", &yChange);
                ImGui::Checkbox("Z Change", &zChange);
                auto positionControl = [&](const char *label, Model &model) {
                    if (xChange) {
                        ImGui::DragFloat((std::string(label) + " X").c_str(), &model.position.x, 0.1f, 0.0f, 0.0f,
                                         "%.3f");
                    }
                    if (yChange) {
                        ImGui::DragFloat((std::string(label) + " Y").c_str(), &model.position.y, 0.1f, 0.0f, 0.0f,
                                         "%.3f");
                    }
                    if (zChange) {
                        ImGui::DragFloat((std::string(label) + " Z").c_str(), &model.position.z, 0.1f, 0.0f, 0.0f,
                                         "%.3f");
                    }
                };

                auto lightControl = [&](const char *label, glm::vec3 &pos) {
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
                positionControl("Gun", *gun);
                positionControl("Cart", *scope);
                positionControl("Track", *track);
                positionControl("House", *house);
                positionControl("Left", *left);
                positionControl("Right", *right);
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

                auto pitchControl = [&](const char *label, Model &model) {
                    switch (controlMode) {
                        case 0:
                            ImGui::DragFloat((std::string(label) + " Pitch").c_str(), &model.pitch, 0.1f, -360.0f,
                                             360.0f,
                                             "%.3f");
                            break;
                        case 1:
                            ImGui::DragFloat((std::string(label) + " Yaw").c_str(), &model.yaw, 0.1f, -360.0f, 360.0f,
                                             "%.3f");
                        case 2:
                            ImGui::DragFloat((std::string(label) + " Roll").c_str(), &model.roll, 0.1f, -360.0f, 360.0f,
                                             "%.3f");
                            break;
                    }
                };

                pitchControl("Cart", *cart);
                pitchControl("Track", *track);
                pitchControl("House", *house);
                pitchControl("Left", *left);
                pitchControl("Right", *right);
                pitchControl("Gun", *gun);
                pitchControl("scope", *scope);

                ImGui::End();
                ImGui::Render();
            }
        }
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------

        depth.use();
        depth.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE2);

        glCullFace(GL_FRONT);

        renderer.renderShadowMap(depth);
        depth.setMat4("model", glm::mat4(1.0f));

        if (renderer.torch) {
            lightView = glm::lookAt(camera->position, glm::vec3(0.0f, 0.0f, 0.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
            renderer.renderShadowMap(depth);
        }
        glCullFace(GL_BACK); //
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.renderAll();

        world.tick(deltaTime, *terrain);


        if (insideCart) {
            auto newPos = cSpline.ConstVelocitySplineAtTime(currentFrameTime * 60);
            auto pitch = calculateYawPitch(cart->position, newPos);
            auto pitchDif = cart->pitch - pitch.pitch;
            auto yawDif = cart->yaw - pitch.yaw;
            cart->position = newPos;
            cart->pitch = pitch.pitch;
            cart->yaw = pitch.yaw;
            world.updateAll(3, cart->position, cart->pitch, cart->yaw, 0.0f);
            camera->position = cart->position;
            camera->position.y += 5.0f;
            world.updatePosition(camera->id, camera->position);
        }
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
        if (down > 3.0) {
            updown = true;
        }
        if (down < 0) {
            updown = false;
        }
        terrain->terrainShader.use();
        terrain->terrainShader.setMat4("projection", projection);
        terrain->terrainShader.setMat4("view", camera->getCameraView());
        terrain->terrainShader.setMat4("model", glm::translate(glm::mat4(1.0), terrain->terposition));
        terrain->terrainShader.setVec3("lightDir", lightPos);
        terrain->terrainShader.setVec3("lightColor", glm::vec3(1.0f));
        terrain->terrainShader.setVec3("viewPos", camera->position);
        terrain->terrainShader.setFloat("minHeight", terrain->minHeight);
        terrain->terrainShader.setFloat("maxHeight", terrain->maxHeight);
        terrain->render();
        basic.use();
        basic.setMat4("projection", projection);
        basic.setMat4("view", camera->getCameraView());
        basic.setMat4("model", glm::mat4(1.0));
        basic.setVec4("color", glm::vec4(1.0, 1.0, 1.0, 1.0));
        glBindVertexArray(splineVAO);
        glDrawArrays(GL_LINE_STRIP, 0, sizeof(splinearray));
        glBindVertexArray(0);

        processInput(window, terrain);
        if (debug) {
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        glfwSwapBuffers(window);
        player.tick(deltaTime, camera);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, const std::shared_ptr<Terrain> &terrain) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    glm::vec3 forward = camera->front;
    switch (player.state) {
        case PlayerState::State::IDLE:
        case PlayerState::State::RUNNING:
            forward = glm::vec3(forward.x, 0.0, forward.z);
            break;
        case PlayerState::State::FLYING:
            break;
        case PlayerState::State::LADDER:
            forward = glm::vec3(0.0, 1.0, 0.0);
            break;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        float force = player.state == PlayerState::State::LADDER ? 30.0f : 10.0f;
        world.applyForce(camera->id, forward * force);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {

        float force = player.state == PlayerState::State::LADDER ? 30.0f : 10.0f;
        world.applyForce(camera->id, -forward * force);
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (player.state == PlayerState::State::LADDER)
            return;
        world.applyForce(camera->id, -camera->right * 10.0f);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (player.state == PlayerState::State::LADDER)
            return;
        world.applyForce(camera->id, camera->right * 10.0f);
    }
}

// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int nwidth, int nheight) {
    height = nheight;
    width = nwidth;
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.01f,
                                  1000.0f);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
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
    world.updatePyr(camera->id, camera->options.pitch, camera->options.yaw, 0.0f);
}


/*  if (cam.hold[cam.currentCameraIndex].position.x >= model.boundingbox.mMin.x && cam.hold[cam.currentCameraIndex].position.x <= model.boundingbox.mMax.x && cam.hold[cam.currentCameraIndex].position.y >= model.boundingbox.mMin.y && cam.hold[cam.currentCameraIndex].position.y <= model.boundingbox.mMax.y && cam.hold[cam.currentCameraIndex].position.z >= model.boundingbox.mMin.z && cam.hold[cam.currentCameraIndex].position.z <= model.boundingbox.mMax.z) {

      std::vector<float> arr = {
         colPacketA->obj->camera->position.y // xmin
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
