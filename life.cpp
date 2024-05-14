#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <thread>
#include <chrono>

const int GAME_WIDTH = 64;
const int GAME_HEIGHT = 48;
const int CELL_SIZE = 10;

bool isAlive(
    const std::array<std::array<int, GAME_HEIGHT>, GAME_WIDTH>& game,
    const int x,
    const int y
) {
    int neighbors = 0;
    const int state = game[x][y];

    if (x > 0 && game[x-1][y] == 1) neighbors++;
    if (x < GAME_WIDTH-1 && game[x+1][y] == 1) neighbors++;
    if (y > 0 && game[x][y-1] == 1) neighbors++;
    if (y < GAME_HEIGHT-1 && game[x][y+1] == 1) neighbors++;

    if (x > 0 && y > 0 && game[x-1][y-1] == 1) neighbors++;
    if (x > 0 && y < GAME_HEIGHT-1 && game[x-1][y+1] == 1) neighbors++;
    if (x < GAME_WIDTH-1 && y > 0 && game[x+1][y-1] == 1) neighbors++;
    if (x < GAME_WIDTH-1 && y < GAME_HEIGHT-1 && game[x+1][y+1] == 1) neighbors++;

    if (state == 1 && neighbors < 2) return false;
    if (state == 1 && (neighbors == 2 || neighbors == 3)) return true;
    if (state == 1 && neighbors > 3) return false;
    if (state == 0 && neighbors == 3) return true;

    return false;
}

void drawGrid(const std::array<std::array<int, GAME_HEIGHT>, GAME_WIDTH>& game) {
    glBegin(GL_QUADS);
    for (int x = 0; x < GAME_WIDTH; ++x) {
        for (int y = 0; y < GAME_HEIGHT; ++y) {
            if (game[x][y] == 1) {
                glColor3f(1.0f, 1.0f, 1.0f);
            } else {
                glColor3f(0.0f, 0.0f, 0.0f);
            }
            glVertex2f(x * CELL_SIZE, y * CELL_SIZE);
            glVertex2f((x + 1) * CELL_SIZE, y * CELL_SIZE);
            glVertex2f((x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE);
            glVertex2f(x * CELL_SIZE, (y + 1) * CELL_SIZE);
        }
    }
    glEnd();
}

void setupOpenGL(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Function to place a pattern (e.g., a glider or a 3-cell line) centered at (cellX, cellY)
void placePattern(std::array<std::array<int, GAME_HEIGHT>, GAME_WIDTH>& game, int cellX, int cellY) {
    // Define a 3-cell horizontal line as the pattern
    if (cellX >= 0 && cellX < GAME_WIDTH && cellY >= 0 && cellY < GAME_HEIGHT) {
        game[cellX][cellY] = 1; // Center cell
        if (cellX > 0) game[cellX - 1][cellY] = 1; // Left cell
        if (cellX < GAME_WIDTH - 1) game[cellX + 1][cellY] = 1; // Right cell
    }
}

// Mouse button callback to change the state of a cell
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << "Mouse click at (" << xpos << ", " << ypos << ")" << std::endl;

        int cellX = static_cast<int>(xpos) / CELL_SIZE;
        int cellY = static_cast<int>(ypos) / CELL_SIZE;

        std::cout << "Grid cell (" << cellX << ", " << cellY << ")" << std::endl;

        if (cellX >= 0 && cellX < GAME_WIDTH && cellY >= 0 && cellY < GAME_HEIGHT) {
            std::array<std::array<int, GAME_HEIGHT>, GAME_WIDTH>* displayPtr = static_cast<std::array<std::array<int, GAME_HEIGHT>, GAME_WIDTH>*>(glfwGetWindowUserPointer(window));
            placePattern(*displayPtr, cellX, cellY);
            std::cout << "Pattern placed at (" << cellX << ", " << cellY << ")" << std::endl;
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(GAME_WIDTH * CELL_SIZE, GAME_HEIGHT * CELL_SIZE, "Game of Life", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    std::srand(std::time(0));

    setupOpenGL(GAME_WIDTH * CELL_SIZE, GAME_HEIGHT * CELL_SIZE);

    std::array<std::array<int, GAME_HEIGHT>, GAME_WIDTH> display {};
    std::array<std::array<int, GAME_HEIGHT>, GAME_WIDTH> swap {};

    for (auto& row : display) {
        std::generate(row.begin(), row.end(), []() { return rand() % 10 == 0 ? 1 : 0; });
    }

    // Set the user pointer to the display array for the mouse callback
    glfwSetWindowUserPointer(window, &display);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    while (!glfwWindowShouldClose(window)) {
        for (int x = 0; x < GAME_WIDTH; ++x) {
            for (int y = 0; y < GAME_HEIGHT; ++y) {
                swap[x][y] = isAlive(display, x, y) ? 1 : 0;
            }
        }
        std::swap(display, swap);
        glClear(GL_COLOR_BUFFER_BIT);
        drawGrid(display);
        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    glfwTerminate();
    return 0;
}
