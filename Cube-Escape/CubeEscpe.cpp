#include <Windows.h>
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

// Cube position and speed variables
float cubeX = 0.0f, speed = 0.05f, initialSpeed = 0.05f, maxSpeed = 0.21f, boundaryX = 1.0f;

// Movement flags
bool moveLeft = false, moveRight = false;

// Scoring variables
int score = 0, updateTime = 1000;

// Game state variables
bool gameOver = false;
int obstacleTime = 2000;

// Pause menu
bool isPaused = false;

// Lighting parameters
GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 5.0f, 5.0f, 1.0f };

// Material properties
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess[] = { 50.0f };

// Obstacle structure
struct Obstacle {
    float x;
    float z;
    float size;
    float rotation; // Add rotation for obstacles
};

// List of obstacles
vector<Obstacle> obstacles;

void update(int value);
void updateScore(int value);
void addObstacle(int value);

// Initialize the game state
void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.18, 0.141, 0.271, 1.0);

    //loadTexture("intro.png", introTexture);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    // Set up lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Set material properties
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

    // Configure color material to work with lighting
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    for (int i = 0; i < 5; ++i) { // Initialize 5 rows of obstacles

        addObstacle(0);

    }
}

// Function to handle window resizing
void reshape(int width, int height) {
    const float ar = (float)width / (float)height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, ar, 0.1, 100.0); // Perspective projection
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    boundaryX = ar * 1.0f;
}

// Function to handle keyboard input (press)
void keyboard(unsigned char key, int x, int y) {
    if (key == 27 || key == 'q' || key == 'Q') { // ESC or 'q' key
        exit(0);
    }

    if (key == 'p' || key == 'P') {
        if (!gameOver) { // Only toggle pause if the game is not over
            isPaused = !isPaused;
            glutPostRedisplay();
            if (!isPaused) {
                glutTimerFunc(16, update, 0);
                glutTimerFunc(updateTime, updateScore, 0);
            }
        }
    }

    if (!isPaused) {
        if (key == 'a') moveLeft = true;
        if (key == 'd') moveRight = true;
    }

    if ((gameOver || isPaused) && (key == 13 || key == 'r' || key == 'R')) { // Restart game
        gameOver = false;
        isPaused = false;
        moveLeft = false;
        moveRight = false;
        cubeX = 0.0f;
        speed = initialSpeed;
        score = 0;
        obstacles.clear();
        init();
        glutTimerFunc(16, update, 0);
        glutTimerFunc(updateTime, updateScore, 0);
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (key == 'a') moveLeft = false;
    if (key == 'd') moveRight = false;
}

void specialKey(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT) moveLeft = true;
    if (key == GLUT_KEY_RIGHT) moveRight = true;
}

void specialKeyUp(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT) moveLeft = false;
    if (key == GLUT_KEY_RIGHT) moveRight = false;
}

// Function to render text on the screen
void renderText(float x, float y, void* font, const string& text) {
    glRasterPos2f(x, y);
    for (const char& c : text) {
        glutBitmapCharacter(font, c);
    }
}

void drawPauseText() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    renderText(-1.8f, 0.8f, GLUT_BITMAP_HELVETICA_18, "Press P to Pause");
}

void drawPauseMenu() {
    // Render the game scene (this will keep the current game state visible)
    glEnable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(cubeX, 0.0f, 0.0f);
    glColor3f(1.0, 0.0, 0.0);
    glutSolidCube(0.5);
    glPopMatrix();

    for (const auto& obstacle : obstacles) {
        glPushMatrix();
        glTranslatef(obstacle.x, 0.0f, obstacle.z);
        glRotatef(obstacle.rotation, 0.0f, 1.0f, 0.0f);
        glColor3f(0.0, 1.0, 0.0);
        glutSolidCube(obstacle.size);
        glPopMatrix();
    }

    // Switch to orthographic projection for overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1920, 0, 1080);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING); // Disable lighting for the overlay

    // Render the semi-transparent background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0, 0.0, 0.0, 0.5); // Black with 50% transparency
    glBegin(GL_QUADS);
    glVertex2f(0, 0);          // Bottom-left corner
    glVertex2f(1920, 0);        // Bottom-right corner
    glVertex2f(1920, 1080);      // Top-right corner
    glVertex2f(0, 1080);        // Top-left corner
    glEnd();
    glDisable(GL_BLEND);

    // Render the pause menu text
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glDisable(GL_DEPTH_TEST);
    renderText(900.0f, 800.0f, GLUT_BITMAP_HELVETICA_18, "PAUSED");
    renderText(850.0f, 600.0f, GLUT_BITMAP_HELVETICA_18, "Press P to Resume");
    renderText(850.0f, 500.0f, GLUT_BITMAP_HELVETICA_18, "Press R to Restart");
    renderText(850.0f, 400.0f, GLUT_BITMAP_HELVETICA_18, "Press Q to Quit");
    glEnable(GL_DEPTH_TEST);

    // Restore the previous projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Helper function to render text
void renderText(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0.0, 1.2, 3.0,  // Camera position
        0.0, 0.0, 0.0,        // Look-at point
        0.0, 1.0, 0.0);       // Up vector

    // Update light position in the scene
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    if (isPaused) {
        drawPauseMenu();
    }
    else if (gameOver) {
        // Render game over message
        glDisable(GL_LIGHTING);
        glColor3f(1.0, 0.0, 0.0);
        renderText(-0.5f, 0.0f, GLUT_BITMAP_HELVETICA_18, "Game Over");
        renderText(-0.5f, -0.5f, GLUT_BITMAP_HELVETICA_18, "Score: " + to_string(score));
        renderText(-0.5f, -1.0f, GLUT_BITMAP_HELVETICA_18, "Press 'R' or Enter to Restart");
    }
    else {
        glEnable(GL_LIGHTING);
        // Render the player cube
        glPushMatrix();
        glTranslatef(cubeX, 0.0f, 0.0f);
        glColor3f(1.0, 0.0, 0.0);
        glutSolidCube(0.5);
        glPopMatrix();

        // Render obstacles
        for (const auto& obstacle : obstacles) {
            glPushMatrix();
            glTranslatef(obstacle.x, 0.0f, obstacle.z);
            glRotatef(obstacle.rotation, 0.0f, 1.0f, 0.0f);
            glColor3f(0.0, 1.0, 0.0);
            glutSolidCube(obstacle.size);
            glPopMatrix();
        }

        // Render score
        glDisable(GL_LIGHTING);
        glColor3f(1.0, 1.0, 1.0);
        renderText(-1.8f, 1.0f, GLUT_BITMAP_HELVETICA_18, "Score: " + to_string(score));
        drawPauseText();
    }

    glutSwapBuffers();
}

// Function to update the cube's position and obstacles
void update(int value) {
    if (gameOver || isPaused /*|| isIntro*/) return;
    // Update player position
    if (moveLeft && cubeX - speed > -boundaryX) cubeX -= speed;
    if (moveRight && cubeX + speed < boundaryX) cubeX += speed;

    // Update obstacle positions (move towards the camera along Z-axis)
    for (auto& obstacle : obstacles) {
        obstacle.z += speed;
        obstacle.rotation += 1.0f; // Rotate obstacles

        // Check collision with the cube
        if (fabs(cubeX - obstacle.x) < (0.25f + obstacle.size / 2) && obstacle.z > -0.3f && obstacle.z < 0.3f) {
            gameOver = true;
            break;
        }

        // Reset obstacle if it moves past the camera
        if (obstacle.z > 2.0f) {
            obstacle.z = -20.0f;  // Move the obstacle far away to recycle
            obstacle.x = static_cast<float>(rand() % 200 - 100) / 100.0f * boundaryX;

            obstacle.size = static_cast<float>(rand() % 3 + 1) / 10.0f + 0.2f;
        }
    }

    // Increase difficulty over time
    if (score > 0 && score % 10 == 0) {
        speed = min(maxSpeed, speed + 0.0002f);
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // Call update every 16 ms (~60 FPS)
}

// Function to update the score
void updateScore(int value) {
    if (gameOver || isPaused /*|| isIntro*/) return;
    score++;
    glutTimerFunc(updateTime, updateScore, 0); // Update score every second
}

// Function to add new obstacles

void addObstacle(int value) {

    if (gameOver || isPaused /*|| isIntro*/) return;

    int baseObstacles = 1;
    int numObstacles = max(1, baseObstacles - score / 100);
    float minSpacing = 3.5f + score * 0.1f;
    float initialZ = -30.0f;
    int maxRetries = 10;

    for (int i = 0; i < numObstacles; ++i) {
        float x, size;
        bool positionIsValid = false;
        int retries = 0;

        while (!positionIsValid && retries < maxRetries) {
            x = static_cast<float>(rand() % 200 - 100) / 100.0f * boundaryX;
            size = static_cast<float>(rand() % 3 + 1) / 10.0f + 0.2f;
            positionIsValid = true;

            for (const auto& obstacle : obstacles) {
                if (fabs(x - obstacle.x) < minSpacing && fabs(initialZ - obstacle.z) < minSpacing) {
                    positionIsValid = false;
                    break;
                }
            }
            retries++;
        }

        if (positionIsValid) {
            obstacles.push_back({ x, initialZ, size });
        }
    }
    glutTimerFunc(obstacleTime, addObstacle, 0);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Cube Escape");

    init();
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);
    glutDisplayFunc(display);

    glutTimerFunc(16, update, 0);       // Start the update loop
    glutTimerFunc(updateTime, updateScore, 0); // Start score updates

    glutMainLoop();
    /*glDeleteTextures(1, &introTexture);*/
    return 0;
}
