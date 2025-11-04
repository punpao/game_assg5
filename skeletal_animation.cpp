// File: src/8.guest/2020/skeletal_animation/skeletal_controller.cpp
// Playable skeletal character with orbit camera + full animation controls.

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/animator.h>

#include <iostream>
#include <fstream>
#include <unordered_map>

static const unsigned WIDTH = 1280;
static const unsigned HEIGHT = 720;

// ---- orbit camera (ไม่ใช้ camera class หมุนเองเพื่อให้ไอเดียต่าง) ----
struct OrbitCam {
    glm::vec3 target{ 0, 1.0f, 0 };
    float distance = 4.0f;
    float yaw = glm::radians(180.0f);
    float pitch = glm::radians(10.0f);
    float fov = 45.0f;

    glm::mat4 view() const {
        glm::vec3 eye;
        eye.x = target.x + distance * cosf(pitch) * cosf(yaw);
        eye.y = target.y + distance * sinf(pitch);
        eye.z = target.z + distance * cosf(pitch) * sinf(yaw);
        return glm::lookAt(eye, target, glm::vec3(0, 1, 0));
    }
} gCam;

float gDelta = 0, gLast = 0;
bool  gLMB = false; double gMx = 0, gMy = 0;

// ---- player transform & anim control ----
struct Player {
    glm::vec3 pos{ 0,0,0 };
    float yawDeg = 180.0f;        // หันหน้าเข้ากล้องตอนเริ่ม
    float speed = 2.2f;          // m/s
} gPlayer;

struct AnimCtrl {
    bool paused = false;
    bool reverse = false;
    float rate = 1.0f;              // 0.5/1/2
    bool slowmo = false;
} gAnim;

static void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }
static void scroll_callback(GLFWwindow*, double, double yoff) {
    gCam.distance = glm::clamp(gCam.distance - (float)yoff * 0.4f, 1.5f, 12.0f);
}
static void mouse_button_callback(GLFWwindow*, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) gLMB = (action == GLFW_PRESS);
}
static void cursor_pos_callback(GLFWwindow*, double x, double y) {
    if (gLMB) {
        float dx = (float)(x - gMx) * 0.005f;
        float dy = (float)(y - gMy) * 0.005f;
        gCam.yaw -= dx;
        gCam.pitch = glm::clamp(gCam.pitch - dy, glm::radians(-80.0f), glm::radians(80.0f));
    }
    gMx = x; gMy = y;
}
static bool pressed(GLFWwindow* w, int key) {
    static std::unordered_map<int, bool> last;
    bool now = glfwGetKey(w, key) == GLFW_PRESS;
    bool rise = now && !last[key];
    last[key] = now; return rise;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* win = glfwCreateWindow(WIDTH, HEIGHT, "Playable Skeletal Character", nullptr, nullptr);
    if (!win) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetScrollCallback(win, scroll_callback);
    glfwSetCursorPosCallback(win, cursor_pos_callback);
    glfwSetMouseButtonCallback(win, mouse_button_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // orbit camera = เมาส์ปกติ

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Failed to init GLAD\n"; return -1; }
    glViewport(0, 0, WIDTH, HEIGHT);
    stbi_set_flip_vertically_on_load(true);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    Shader sh("anim_model.vs", "anim_model.fs");

    // asset path
    const std::string rel = "resources/objects/vampire/dancing_vampire.dae";
    std::string asset = FileSystem::getPath(rel);
    std::ifstream f(asset);
    if (!f.good()) {
        std::cout << "[ERROR] Model not found at: " << asset << "\n";
    }
    else {
        std::cout << "Resolved path: " << asset << "\n";
    }

    Model     model(asset);
    Animation clip(asset, &model);
    Animator  animator(&clip);
    animator.UpdateAnimation(0.0f);

    std::cout << "meshes: " << model.meshes.size() << "\n";
    std::cout << "bone_mats: " << animator.GetFinalBoneMatrices().size() << "\n";

    const float scale = 0.02f;

    while (!glfwWindowShouldClose(win)) {
        float now = (float)glfwGetTime(); gDelta = now - gLast; gLast = now;
        glfwPollEvents();

        // ---- input: movement & anim control ----
        // หมุนลำตัว A/D
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) gPlayer.yawDeg += 90.0f * gDelta;
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) gPlayer.yawDeg -= 90.0f * gDelta;
        // W/S เดินหน้า–ถอยหลัง
        float forward = 0.0f;
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) forward += 1.0f;
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) forward -= 1.0f;
        if (forward != 0.0f) {
            float r = glm::radians(gPlayer.yawDeg);
            glm::vec3 dir(-sinf(r), 0.0f, -cosf(r));
            gPlayer.pos += dir * (gPlayer.speed * forward * gDelta);
        }

        // anim keys
        if (pressed(win, GLFW_KEY_SPACE)) gAnim.paused = !gAnim.paused;
        if (pressed(win, GLFW_KEY_R))     gAnim.reverse = !gAnim.reverse;
        if (pressed(win, GLFW_KEY_1))     gAnim.rate = 0.5f;
        if (pressed(win, GLFW_KEY_2))     gAnim.rate = 1.0f;
        if (pressed(win, GLFW_KEY_3))     gAnim.rate = 2.0f;
        if (pressed(win, GLFW_KEY_B))     gAnim.slowmo = !gAnim.slowmo;

        // step-by-step frame
        if (pressed(win, GLFW_KEY_LEFT_BRACKET))  animator.UpdateAnimation(-0.02f);
        if (pressed(win, GLFW_KEY_RIGHT_BRACKET)) animator.UpdateAnimation(+0.02f);

        // update animation
        if (!gAnim.paused) {
            float rate = gAnim.rate * (gAnim.slowmo ? 0.2f : 1.0f);
            float signedRate = gAnim.reverse ? -rate : rate;
            animator.UpdateAnimation(gDelta * signedRate);
        }

        // orbit camera follow target (ศีรษะ)
        gCam.target = gPlayer.pos + glm::vec3(0, 1.0f, 0);

        // ---- render ----
        glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sh.use();
        glm::mat4 P = glm::perspective(glm::radians(gCam.fov), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 V = gCam.view();
        sh.setMat4("projection", P);
        sh.setMat4("view", V);

        // bones
        auto mats = animator.GetFinalBoneMatrices();
        if (mats.empty()) mats.resize(200, glm::mat4(1.0f));
        for (size_t i = 0; i < mats.size(); ++i)
            sh.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", mats[i]);

        // model matrix (translate + rotate ตาม yaw)
        glm::mat4 M(1.0f);
        /*M = glm::translate(M, gPlayer.pos);
        M = glm::rotate(M, glm::radians(gPlayer.yawDeg), glm::vec3(0, 1, 0));
        M = glm::scale(M, glm::vec3(scale));*/
        const float scale = 0.5f;                      
        M = glm::translate(M, gPlayer.pos + glm::vec3(0.0f, -0.4f, 0.0f));  
        M = glm::rotate(M, glm::radians(gPlayer.yawDeg), glm::vec3(0, 1, 0));
        M = glm::scale(M, glm::vec3(scale));
        sh.setMat4("model", M);

        model.Draw(sh);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glfwSwapBuffers(win);
    }

    glfwTerminate();
    return 0;
}
