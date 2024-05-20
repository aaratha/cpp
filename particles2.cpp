#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

const float PI = 3.14159f;
const float g = 9.81f;
const float mass = 1.0f;
const float partRadius = 0.04f;
const float partCreationInterval = 0.0f;
const float damping = 0.0f;  // Adjusted damping factor

float lastPartCreationTime = 0.0f;
bool isMousePressed = false;
double mouseX = 0.0f, mouseY = 0.0f;

struct Particle {
    float posX;
    float posY;
    float velX;
    float velY;
    float lastPosX;
    float lastPosY;
};

std::vector<Particle> particles;

void screenToWorld(
    GLFWwindow* window,
    double sx,
    double sy,
    double& wx,
    double& wy
) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    wx = (sx / width) * 2.0 - 1.0;
    wy = 1.0 - (sy / height) * 2.0;
}

void verlet(
    float& position,
    float& lastPosition,
    float acceleration,
    float dt
) {
    float nextPosition = 2.0f * position - lastPosition + acceleration * dt * dt;
    lastPosition = position;
    position = nextPosition;
}

void updateVelocity(Particle& particle, float dt) {
    particle.velX = (particle.posX - particle.lastPosX) / dt;
    particle.velY = (particle.posY - particle.lastPosY) / dt;
}

void resolveCollision(Particle& p1, Particle& p2, float nx, float ny, float overlap) {
    p1.posX -= nx * overlap / 2;
    p1.posY -= ny * overlap / 2;
    p2.posX += nx * overlap / 2;
    p2.posY += ny * overlap / 2;

    float vi_dot_n = p1.velX * nx + p1.velY * ny;
    float vj_dot_n = p2.velX * nx + p2.velY * ny;

    float vi_nx = vi_dot_n * nx;
    float vi_ny = vi_dot_n * ny;
    float vj_nx = vj_dot_n * nx;
    float vj_ny = vj_dot_n * ny;

    p1.velX = p1.velX - vi_nx + vj_nx;
    p1.velY = p1.velY - vi_ny + vj_ny;
    p2.velX = p2.velX - vj_nx + vi_nx;
    p2.velY = p2.velY - vj_ny + vi_ny;
}

void updatePhysics(GLFWwindow* window, float dt) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    const float forceX = 0.0f;
    const float forceY = -g * mass;
    const float accelX = forceX / mass;
    const float accelY = forceY / mass;
    const float radiusSum = 2.0f * partRadius;

    for (auto& particle : particles) {
        verlet(particle.posX, particle.lastPosX, accelX, dt);
        verlet(particle.posY, particle.lastPosY, accelY, dt);

        updateVelocity(particle, dt);

        // Check for border collisions and respond accordingly
        if (particle.posX - partRadius < -1.0f) {
            particle.posX = -1.0f + partRadius;
            particle.velX *= -1.0f;
        }
        if (particle.posX + partRadius > 1.0f) {
            particle.posX = 1.0f - partRadius;
            particle.velX *= -1.0f;
        }
        if (particle.posY - partRadius < -1.0f) {
            particle.posY = -1.0f + partRadius;
            particle.velY *= -1.0f;
        }
        if (particle.posY + partRadius > 1.0f) {
            particle.posY = 1.0f - partRadius;
            particle.velY *= -1.0f;
        }
    }

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            float dx = particles[j].posX - particles[i].posX;
            float dy = particles[j].posY - particles[i].posY;
            float distanceSquared = dx * dx + dy * dy;

            if (distanceSquared < radiusSum * radiusSum) {
                float distance = std::sqrt(distanceSquared);
                float overlap = radiusSum - distance;
                float nx = dx / distance;
                float ny = dy / distance;

                resolveCollision(particles[i], particles[j], nx, ny, overlap);
            }
        }
    }

    static float timeElapsed = 0.0f;
    timeElapsed += dt;
    if (isMousePressed && timeElapsed - lastPartCreationTime >= partCreationInterval) {
        Particle newParticle = { static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f, 0.0f, static_cast<float>(mouseX), static_cast<float>(mouseY) };
        particles.push_back(newParticle);
        lastPartCreationTime = timeElapsed;
    }
}

void render(float time) {
    glClear(GL_COLOR_BUFFER_BIT);
    for (const auto& particle : particles) {
        float r = 0.5f + 0.5f * sin(time);
        float g = 0.5f + 0.5f * sin(time + 2.0f * PI / 3.0f); // Phase shift for green
        float b = 0.5f + 0.5f * sin(time + 4.0f * PI / 3.0f); // Phase shift for blue
        glColor3f(r, g, b);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            float degInRad = i * PI / 180;
            glVertex2f(
                cos(degInRad) * partRadius + particle.posX,
                sin(degInRad) * partRadius + particle.posY
            );
        }
        glEnd();
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        screenToWorld(window, xpos, ypos, mouseX, mouseY);
        if (action == GLFW_PRESS) {
            isMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            isMousePressed = false;
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

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        updatePhysics(window, dt);
        render(currentTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
