#include "Animator.h"
#include "Camera.h"
#include "Cube.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Instance.h"
#include "Model.h"
#include "Physics.h"
#include "Renderer.h"
#include "Terrain.h"
#include "lib/miniaudo.h"
#include "utils/Spline.h"
#include <__errc>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> // Include for quaternion operations
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <memory>

glm::quat rotationFromVectors(glm::vec3 start, glm::vec3 dest)
{
    start = glm::normalize(start);
    dest = glm::normalize(dest);

    float cosTheta = glm::dot(start, dest);
    glm::vec3 rotationAxis;

    if (cosTheta < -1 + 0.001f) {
        // special case when vectors in opposite directions:
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
        if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
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
glm::vec3 lightPos(0.0f, -1.0f, -0.3f);
float rotation = 0.0f;
bool opening;
bool detectCollision(aiAABB boundingbox);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

glm::vec3 p1(0.0, 0.009, 0.0);
glm::vec3 p2(0.0, -0.0001, -0.005);
glm::vec3 p3(0.0, 0.5, -0.8);
glm::vec3 p4(0.0, 0.8, -2.4);
auto spline = Spline(p1, p2, p3, p4, 200, 0.25, 0.5);
auto pSpline = std::make_shared<Spline>(spline);

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

void processInput(GLFWwindow* window, Terrain& terrain);
auto height = 2000;
Camera camera = {};
auto width = 3000;
int amount = 100000;

auto projection = glm::perspective(glm::radians(45.0f),
    (float)width / (float)height, 0.01f, 800.0f);
const float deltaTime = 1.0 / 60.0; // fixed time step of 1/60th second
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 0;
float lastY = 0;
double accumulator = 0.0;
bool pressed = false;
bool insideCart = false;
int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit()) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("../src/modelLoading.vert.glsl", "../src/modelLoading.frag.glsl");
    Shader treeShader("../src/tree.vert.glsl", "../src/tree.frag.glsl");
    auto pipe = std::make_shared<Model>("../assets/track/doorLock.obj", glm::mat4(1.0f), glm::vec3(1.0, 0.0, 0.0), 4);
    auto minecart = std::make_shared<Model>("../assets/track/mineCart.obj", glm::mat4(1.0f), glm::vec3(0.0), 19);
    auto cart = std::make_shared<Model>("../assets/track/cart.obj", glm::mat4(1.0f), glm::vec3(-18.212, 5.8958, -0.55272), 7);
    auto track = std::make_shared<Model>("../assets/track/track.obj", glm::mat4(1.0f), glm::vec3(0.0), 8);
    auto tree = std::make_shared<Model>("../assets/tree/spookytree.obj", glm::mat4(1.0f), glm::vec3(0.0), 9);
    auto house = std::make_shared<Model>("../assets/house/hh.obj", glm::mat4(1.0f), glm::vec3(0.0), 10);
    auto monster = std::make_shared<Model>("../assets/monster/monster.obj", glm::mat4(1.0f), glm::vec3(1.1470, 0.0, 0.8994), 11);
    auto teeth = std::make_shared<Model>("../assets/Monster/teeth.obj", glm::mat4(1.0f), glm::vec3(0.0), 11);
    auto outhouseDoor = std::make_shared<Model>("../assets/Monster/door.obj", glm::mat4(1.0f), glm::vec3(0.0), 11);
    auto eyes = std::make_shared<Model>("../assets/Monster/eyes.obj", glm::mat4(1.0f), glm::vec3(0.0), 12);
    // auto hallway = std::make_shared<Model>("../assets/hallway/hallway.obj", glm::mat4(1.0f), glm::vec3(0.0), 13);
    float dim = 0.25;
    std::vector<glm::mat4> translations(amount);

    //  auto newPos = glm::translate(model->translation, glm::vec3(1.7461f, 0.1217f, 0.044948f));

    outhouseDoor->setOrigin(glm::vec3(1.197, -0.00344, 0.4178));

    initInstancedObject(amount, tree, translations);

    auto world = physics::PhysicsWorld();
    Terrain terrain { 1, { "../assets/Water texture.png", "../assets/rock 01.jpg", "../assets/rock02 texture.jpg", "../assets/tilable img 0044 verydark.png" }, 5.0f };
    monster->position = glm::vec3(201.1470, 0, 300.8994);
    camera.position.x = 250;
    camera.position.z = 350;
    camera.position.y = -terrain.GetHeightInterpolated(camera.position.x, camera.position.z);
    camera.position.y += 20.0f;

    glm::vec3 normal = terrain.normalMap(240, 330);
    house->setOrigin(glm::vec3(1.682, -0.027, -0.51));
    float yaw = -glm::degrees(glm::atan(normal.x, glm::sqrt(normal.y * normal.y + normal.z * normal.z))); // Rotate in xy-plane
    house->position = glm::vec3(240, -terrain.getHeight(240, 330), 330.0);

    Renderer renderer { projection, camera, terrain };
    renderer.enqueue(shader, { outhouseDoor, monster, teeth, eyes, track, house, cart, pipe, minecart });

    auto outhouseanimation = initOuthouseDoorAnimation(outhouseDoor);
    auto lastFrameTime = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.4f, 0.1f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float currentTime = static_cast<float>(glfwGetTime());
        double frameTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        accumulator += frameTime;
        while (accumulator >= deltaTime) {
            accumulator -= deltaTime;
            world.tick(deltaTime, terrain);
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            world.applyForce(4, glm::vec3(0.0f, 0.0f, -0.1f));
        }
        renderer.renderAll();

        if (insideCart) {
            camera.position = cart->position;
            camera.position.y += 10.0f;
            camera.position.z -= 2.0f;
            camera.update();
        }

        outhouseanimation->update(deltaTime);
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }

        terrain.terrainShader.use();
        terrain.terrainShader.setMat4("projection", projection);
        terrain.terrainShader.setMat4("view", camera.getCameraView());
        terrain.terrainShader.setMat4("model", glm::translate(glm::mat4(1.0), terrain.terposition));
        terrain.terrainShader.setVec3("lightDir", lightPos);
        terrain.terrainShader.setVec3("lightColor", glm::vec3(1.0f));
        terrain.terrainShader.setVec3("viewPos", camera.position);
        terrain.terrainShader.setMat4("model", glm::translate(glm::mat4(1.0), terrain.terposition));
        terrain.terrainShader.setFloat("minHeight", terrain.minHeight);
        terrain.terrainShader.setFloat("maxHeight", terrain.maxHeight);

        terrain.render();
        treeShader.use();
        renderer.lightingShader(treeShader);
        treeShader.setInt("texture_diffuse1", 0);
        treeShader.setFloat("shininess", 3);
        treeShader.setMat4("projection", projection);
        treeShader.setMat4("view", camera.getCameraView());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tree->textures_loaded[0].id);
        for (unsigned int i = 0; i < tree->meshes.size(); i++) {
            glBindVertexArray(tree->meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(tree->meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }

        glfwPollEvents();
        processInput(window, terrain);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, Terrain& terrain)
{
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        pressed = true;
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && pressed) {
        pressed = false;
        if (insideCart) {
            camera.position = glm::vec3(250, 20, 350);
            camera.update();
        }
        insideCart = !insideCart;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(Camera::Movement::FORWARD, deltaTime, true, terrain);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(Camera::Movement::BACKWARD, deltaTime, true, terrain);
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(Camera::Movement::LEFT, deltaTime, true, terrain);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(Camera::Movement::RIGHT, deltaTime, true, terrain);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
        camera.processKeyboard(Camera::Movement::FORWARD, deltaTime, false, terrain);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
        camera.processKeyboard(Camera::Movement::BACKWARD, deltaTime, false, terrain);
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
        camera.processKeyboard(Camera::Movement::LEFT, deltaTime, false, terrain);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
        camera.processKeyboard(Camera::Movement::RIGHT, deltaTime, false, terrain);
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
    camera.processMouseMovement(xoffset, yoffset);
}

bool detectCollision(aiAABB boundingbox)
{
    return camera.position.x >= boundingbox.mMin.x && camera.position.x <= boundingbox.mMax.x && camera.position.y >= boundingbox.mMin.y && camera.position.y <= boundingbox.mMax.y && camera.position.z >= boundingbox.mMin.z && camera.position.z <= boundingbox.mMax.z;
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
