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
#include <memory>

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
    (float)width / (float)height, 0.1f, 1000.0f);
const float deltaTime = 1.0 / 60.0; // fixed time step of 1/60th second
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 0;
float lastY = 0;
double accumulator = 0.0;
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
    Shader terShader("../src/terrain.vert.glsl", "../src/terrain.frag.glsl");
    auto leftDoor = std::make_shared<Model>(Model { "../assets/house/leftDoor/leftDoor.obj", glm::mat4(1.0f), glm::vec3(0), 1 });
    auto rightDoor = std::make_shared<Model>(Model { "../assets/house/rightDoor/rightDoor.obj", glm::mat4(1.0f), glm::vec3(0), 2 });
    auto cartDoor = std::make_shared<Model>(Model { "../assets/track/cartDoor.obj", glm::mat4(1.0f), glm::vec3(0.0), 3 });
    auto pipe = std::make_shared<Model>(Model { "../assets/track/doorLock.obj", glm::mat4(1.0f), glm::vec3(0.0, 0.0, 1.0), 4 });
    auto minecart = std::make_shared<Model>(Model { "../assets/track/mineCart.obj", glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0), 5 });
    auto wheelFront = std::make_shared<Model>("../assets/track/wheelBack.obj", glm::mat4(1.0f), glm::vec3(0.0), 6);
    auto wheelBack = std::make_shared<Model>("../assets/track/wheelFront.obj", glm::mat4(1.0f), glm::vec3(0.0), 7);
    auto track = std::make_shared<Model>("../assets/track/spokytrackobj.obj", glm::mat4(1.0f), glm::vec3(0.0), 8);
    auto tree = std::make_shared<Model>("../assets/tree/spookytree.obj", glm::mat4(1.0f), glm::vec3(0.0), 9);
    auto house = std::make_shared<Model>("../assets/house/hh.obj", glm::mat4(1.0f), glm::vec3(0.0), 10);
    auto outhouse = std::make_shared<Model>("../assets/Monster/outhouse.obj", glm::mat4(1.0f), glm::vec3(0.0), 11);
    auto monster = std::make_shared<Model>("../assets/Monster/monster.obj", glm::mat4(1.0f), glm::vec3(0.0), 11);
    auto teeth = std::make_shared<Model>("../assets/Monster/teeth.obj", glm::mat4(1.0f), glm::vec3(0.0), 11);
    auto outhouseDoor = std::make_shared<Model>("../assets/Monster/door.obj", glm::mat4(1.0f), glm::vec3(0.0), 11);
    auto eyes = std::make_shared<Model>("../assets/Monster/eyes.obj", glm::mat4(1.0f), glm::vec3(0.0), 12);
    // auto hallway = std::make_shared<Model>("../assets/hallway/hallway.obj", glm::mat4(1.0f), glm::vec3(0.0), 13);
    Renderer renderer { projection, camera };
    float dim = 0.25;
    std::vector<glm::mat4> translations(amount);

    //  auto newPos = glm::translate(model->translation, glm::vec3(1.7461f, 0.1217f, 0.044948f));

    outhouseDoor->setOrigin(glm::vec3(1.197, -0.00344, 0.4178));

    initInstancedObject(amount, tree, translations);

    renderer.enqueue(shader, { outhouseDoor, outhouse, monster, teeth, eyes, track, house, leftDoor, rightDoor, minecart});
    std::vector<std::shared_ptr<Model>> splineModels = { cartDoor, wheelFront, wheelBack };
    auto world = physics::PhysicsWorld();
    Terrain terrain { 1 };

    camera.position.y = -terrain.getHeight(camera.position.x, camera.position.z);
            Cube cube(pipe->boundingbox);
    auto doorAnimation = initDoorAnimation(cartDoor, pipe, pSpline, minecart, splineModels);
    auto hdAnimation = houseDoorAnimation(rightDoor);
    auto outhouseanimation = initOuthouseDoorAnimation(outhouseDoor);
    auto lastFrameTime = static_cast<float>(glfwGetTime());
    world.addModel(minecart);
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
        renderer.renderAll();
        doorAnimation->update(deltaTime);
        hdAnimation->update(deltaTime);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            doorAnimation->nextState();
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            outhouseDoor->origin.x -= 0.001;
        }

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            outhouseDoor->origin.y -= 0.001;
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            outhouseDoor->origin.z -= 0.001;
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            std::cout << outhouseDoor->origin.x << " " << outhouseDoor->origin.y << " " << outhouseDoor->origin.z << std::endl;
        }

        outhouseanimation->update(deltaTime);
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }

        terrain.render(projection, camera.getCameraView());
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
        processInput(window, terrain);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, Terrain& terrain)
{
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
