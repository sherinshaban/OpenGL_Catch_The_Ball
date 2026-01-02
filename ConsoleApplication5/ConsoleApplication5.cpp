#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include <GL/glut.h>

// --- إعدادات اللعبة ---
int windowWidth = 600;
int windowHeight = 600;

// متغيرات الحالة
int score = 0;
bool isGameOver = false;
GLuint textureID;// تخزين id الصورة

// متغيرات الكورة
float ballX = 300.0f;
float ballY = 500.0f;
float ballRadius = 10.0f;
float ballDirX = 0.0f;// حركه الكرة يمين وشمال
float ballDirY = -6.0f;
float ballSpeed = 6.0f;

// متغيرات المضرب
float paddleX = 350.0f;
float paddleY = 50.0f;
float paddleWidth = 300.0f;
float paddleHeight = 20.0f;
float paddleSpeedKey = 30.0f;

// --- دالة تحميل الصور ---
GLuint loadBMP(const char* imagepath) {
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int width, height;
    unsigned int imageSize;
    unsigned char* data;

    FILE* file = fopen(imagepath, "rb");
    if (!file) { std::cout << "Image not found!\n"; return 0; }
    if (fread(header, 1, 54, file) != 54 || header[0] != 'B' || header[1] != 'M') {
        fclose(file); return 0;
    }
    dataPos = *(int*)&(header[0x0A]);
    imageSize = *(int*)&(header[0x22]);
    width = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);
    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0) dataPos = 54;
    data = new unsigned char[imageSize];
    fread(data, 1, imageSize, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    delete[] data;
    return textureID;
}

// --- دالة الخلفية ---
void drawBackground() {
    if (isGameOver) {
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f, 0.0f, 0.0f); // خلفية حمراء عند الخسارة
    }
    else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(windowWidth, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(windowWidth, windowHeight);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, windowHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// دالة الكتابة بخط كبير
void drawText(float x, float y, std::string text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
}

void drawRect(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS); glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h); glEnd();
}

void drawCircle(float cx, float cy, float r, int num_segments) {
    glColor3f(1.0f, 0.0f, 0.0f); // الكورة حمراء
    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
        float x = r * cosf(theta); float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void resetGame() {
    isGameOver = false;
    score = 0;
    ballX = windowWidth / 2; ballY = windowHeight - 50;
    ballSpeed = 6.0f; ballDirX = 0; ballDirY = -ballSpeed;
    paddleX = windowWidth / 2 - paddleWidth / 2;
}

// --- دالة العرض ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBackground();

    if (isGameOver) {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(windowWidth / 2 - 70, windowHeight / 2, "GAME OVER");
        drawText(windowWidth / 2 - 80, windowHeight / 2 - 40, "Final Score: " + std::to_string(score));
        drawText(windowWidth / 2 - 140, windowHeight / 2 - 80, "Press 'R' or Click to Restart");
    }
    else {
        // رسم المضرب (تم التغيير للأسود: 0.0, 0.0, 0.0)
        drawRect(paddleX, paddleY, paddleWidth, paddleHeight, 0.0f, 0.0f, 0.0f);

        drawCircle(ballX, ballY, ballRadius, 30);

        // السكور أسود وبخط كبير
        glColor3f(0.0f, 0.0f, 0.0f);
        drawText(20, windowHeight - 40, "Score: " + std::to_string(score));
    }
    glutSwapBuffers();
}

void timer(int v) {
    if (!isGameOver) {
        ballX += ballDirX; ballY += ballDirY;
        if (ballX + ballRadius > windowWidth || ballX - ballRadius < 0) ballDirX = -ballDirX;
        if (ballY + ballRadius > windowHeight) ballDirY = -ballDirY;

        if (ballY - ballRadius < 0) isGameOver = true;

        if (ballY - ballRadius <= paddleY + paddleHeight && ballY + ballRadius >= paddleY &&
            ballX >= paddleX && ballX <= paddleX + paddleWidth) {
            ballY = paddleY + paddleHeight + ballRadius + 1.0f;
            ballDirY = fabs(ballDirY);
            score++;
            if (ballSpeed < 15.0f) ballSpeed += 0.2f;
            float hitPoint = (ballX - (paddleX + paddleWidth / 2)) / (paddleWidth / 2);
            ballDirX = hitPoint * ballSpeed;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void mouseMotion(int x, int y) {
    if (!isGameOver) {
        paddleX = x - (paddleWidth / 2);
        if (paddleX < 0) paddleX = 0;
        if (paddleX + paddleWidth > windowWidth) paddleX = windowWidth - paddleWidth;
    }
}
void specialInput(int key, int x, int y) {
    if (!isGameOver) {
        if (key == GLUT_KEY_LEFT) paddleX -= paddleSpeedKey;
        if (key == GLUT_KEY_RIGHT) paddleX += paddleSpeedKey;
        if (paddleX < 0) paddleX = 0;
        if (paddleX + paddleWidth > windowWidth) paddleX = windowWidth - paddleWidth;
    }
    glutPostRedisplay();
}
void keyboard(unsigned char key, int x, int y) {
    if (isGameOver && (key == 'r' || key == 'R' || key == 32)) resetGame();
    if (key == 27) exit(0);
}
void mouseClick(int button, int state, int x, int y) {
    if (isGameOver && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) resetGame();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    textureID = loadBMP("background.bmp");
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Final Game Project");

    init();

    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);
    glutPassiveMotionFunc(mouseMotion);
    glutSpecialFunc(specialInput);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseClick);

    glutMainLoop();
    return 0;
}