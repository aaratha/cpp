#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// Constants
const float PI = 3.14159265358979323846f;

// Simulation parameters
float k = 10.0f;
float mass = 1.0f;
float damping = 0.1f;
float restLengthX = 0.0f;
float restLengthY = 0.0f;

// Mouse interaction parameters
bool isDragging = false;
float mouseSpringConstant = 10.5f;
double mouseX = 0.0f, mouseY = 0.0f;

// State variables
float positionX = 0.5f;
float positionY = 0.0f;
float velocityX = 0.0f;
float velocityY = 0.0f;

void screenToWorld(GLFWwindow* window, double sx, double sy, double& wx, double& wy) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    wx = sx / width;  // Normalize x coordinate to range [0, 1]
    wy = sy / height; // Normalize y coordinate to range [0, 1]
}

void updatePhysics(float dt) {
    float forceX = -k * (positionX - restLengthX);
    float forceY = -k * (positionY - restLengthY);

    if (isDragging) {
        float mouseForceX = mouseSpringConstant * (mouseX - positionX);
        float mouseForceY = mouseSpringConstant * (mouseY - positionY);
        forceX += mouseForceX;
        forceY += mouseForceY;
    }

    forceX -= damping * velocityX;
    forceY -= damping * velocityY;

    float accelerationX = forceX / mass;
    float accelerationY = forceY / mass;

    velocityX += accelerationX * dt;
    velocityY += accelerationY * dt;
    positionX += velocityX * dt;
    positionY += velocityY * dt;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float degInRad = i * PI / 180;
        glVertex2f(cos(degInRad) * 0.05f + positionX, sin(degInRad) * 0.05f + positionY);
    }
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(restLengthX, restLengthY);
    glVertex2f(positionX, positionY);
    glEnd();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        screenToWorld(window, xpos, ypos, mouseX, mouseY);
        if (action == GLFW_PRESS) {
            isDragging = true;
        } else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging) {
        screenToWorld(window, xpos, ypos, mouseX, mouseY);
    }
}

int main() {
    GLFWwindow* window;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    window = glfwCreateWindow(640, 480, "Spring Mass Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window\n";
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    while (!glfwWindowShouldClose(window)) {
        updatePhysics(0.01f);
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
