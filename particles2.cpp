#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

const float PI = 3.14159f;
const float g = 9.81f;
const float mass = 1.0f;
const float partRadius = 0.05f; // Fixed typo
const float partCreationInterval = 0.1f;

float lastPartCreationTime = 0.0f;
bool isSpacePressed = false;
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
    glfwGetWindowSize(window, &width, &height); // Corrected syntax with semicolon

    // Convert screen coordinates (sx, sy) to world coordinates (wx, wy)
    wx = (sx / width * 2.0) - 1.0;
    wy = 1.0 - (sy / height * 2.0);
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

void updatePhysics(float dt) {
    const float forceX = 0.0f;
    const float forceY = -g * mass;
    const float accelX = forceX / mass;
    const float accelY = forceY / mass;
    const float radiusSum = 2.0f * partRadius;

    for (auto& particle : particles) {
        verlet(particle.posX, particle.lastPosX, accelX, dt);
        verlet(particle.posY, particle.lastPosY, accelY, dt);
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
}
