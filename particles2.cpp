#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

// Constants
const float PI = 3.14159265358979323846f;
const float PARTICLE_CREATION_INTERVAL = 0.1f; // Time interval in seconds

// Simulation parameters
const float g = 9.8f;
const float k = 60.0f;
const float mass = 1.0f;
const float damping = 0.2f;
const float ballRadius = 0.05f;
const float bowlRadius = 1.0f; // Define the radius of the circular bowl

// Mouse interaction parameters
bool isDragging = false;
bool isSpacePressed = false;
double mouseX = 0.0f, mouseY = 0.0f;

// Particle structure
struct Particle {
    float positionX;
    float positionY;
    float velocityX;
    float velocityY;
    float lastPositionX;
    float lastPositionY;
};

// Particle system
std::vector<Particle> particles;

float lastParticleCreationTime = 0.0f;

void screenToWorld(GLFWwindow* window, double sx, double sy, double& wx, double& wy) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    wx = (sx / width) * 2.0f - 1.0f;  // Normalize x coordinate to range [-1, 1]
    wy = (sy / height) * 2.0f - 1.0f; // Normalize y coordinate to range [-1, 1]
    wy = -wy; // Invert Y if necessary based on your coordinate system
}

void verletIntegration(float& position, float& lastPosition, float velocity, float acceleration, float dt) {
    float nextPosition = 2.0f * position - lastPosition + acceleration * dt * dt;
    lastPosition = position;
    position = nextPosition;
}

void updatePhysics(float dt) {

    for (auto& particle : particles) {
        float forceX = -damping * particle.velocityX;
        float forceY = -g * mass - damping * particle.velocityY;

        float accelerationX = forceX / mass;
        float accelerationY = forceY / mass;

        verletIntegration(particle.positionX, particle.lastPositionX, particle.velocityX, accelerationX, dt);
        verletIntegration(particle.positionY, particle.lastPositionY, particle.velocityY, accelerationY, dt);

        float distanceFromOrigin = sqrt(particle.positionX * particle.positionX + particle.positionY * particle.positionY);
        if (distanceFromOrigin + ballRadius > bowlRadius) {
            float overlap = (distanceFromOrigin + ballRadius) - bowlRadius;
            float normalizationFactor = distanceFromOrigin / (distanceFromOrigin + overlap);
            particle.positionX *= normalizationFactor;
            particle.positionY *= normalizationFactor;

            float normalX = particle.positionX / distanceFromOrigin;
            float normalY = particle.positionY / distanceFromOrigin;
            float dotProduct = particle.velocityX * normalX + particle.velocityY * normalY;
            particle.velocityX = particle.velocityX - 2 * dotProduct * normalX;
            particle.velocityY = particle.velocityY - 2 * dotProduct * normalY;
            particle.velocityX *= 0.8f;
            particle.velocityY *= 0.8f;
        }
    }

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            float dx = particles[j].positionX - particles[i].positionX;
            float dy = particles[j].positionY - particles[i].positionY;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance < 2 * ballRadius) {
                float overlap = 2 * ballRadius - distance;
                float nx = dx / distance;
                float ny = dy / distance;
                particles[i].positionX -= nx * overlap / 2;
                particles[i].positionY -= ny * overlap / 2;
                particles[j].positionX += nx * overlap / 2;
                particles[j].positionY += ny * overlap / 2;

                float vi_dot_n = particles[i].velocityX * nx + particles[i].velocityY * ny;
                float vj_dot_n = particles[j].velocityX * nx + particles[j].velocityY * ny;
                float vi_nx = vi_dot_n * nx;
                float vi_ny = vi_dot_n * ny;
                float vj_nx = vj_dot_n * nx;
                float vj_ny = vj_dot_n * ny;

                particles[i].velocityX = particles[i].velocityX - vi_nx + vj_nx;
                particles[i].velocityY = particles[i].velocityY - vi_ny + vj_ny;
                particles[j].velocityX = particles[j].velocityX - vj_nx + vi_nx;
                particles[j].velocityY = particles[j].velocityY - vj_ny + vi_ny;
            }
        }
    }
    // Add new particles if the spacebar is held down and enough time has passed
    static float timeElapsed = 0.0f;
    timeElapsed += dt;
    if (isSpacePressed && timeElapsed - lastParticleCreationTime >= PARTICLE_CREATION_INTERVAL) {
        Particle newParticle = { static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f, 0.0f, static_cast<float>(mouseX), static_cast<float>(mouseY) };
        particles.push_back(newParticle);
        lastParticleCreationTime = timeElapsed;
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& particle : particles) {
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            float degInRad = i * PI / 180;
            glVertex2f(cos(degInRad) * ballRadius + particle.positionX, sin(degInRad) * ballRadius + particle.positionY);
        }
        glEnd();
    }

    glBegin(GL_LINE_LOOP);
    for (int i = 0;  i < 360; i++) {
        float degInRad = i * PI / 180;
        glVertex2f(cos(degInRad) * bowlRadius, sin(degInRad) * bowlRadius);
    }
    glEnd();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        screenToWorld(window, xpos, ypos, mouseX, mouseY);
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    mouseX = (xpos / width) * 2.0f - 1.0f;
    mouseY = (ypos / height) * 2.0f - 1.0f;
    mouseY = -mouseY;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE) {
        if (action == GLFW_PRESS) {
            isSpacePressed = true;
        } else if (action == GLFW_RELEASE) {
            isSpacePressed = false;
        }
    }
}

int main() {
    GLFWwindow* window;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    window = glfwCreateWindow(640, 640, "Particle Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window\n";
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        updatePhysics(dt);
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
