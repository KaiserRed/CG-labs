#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <cmath>

float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f; // Углы вращения
float rotationSpeed = 0.2f; // Скорость вращения

void initOpenGL() {
    glEnable(GL_DEPTH_TEST); // Включаем тест глубины для правильной отрисовки
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Цвет фона
    glMatrixMode(GL_PROJECTION); // Ожидаем проекцию
    glLoadIdentity();
    gluPerspective(45.0f, 1.0f, 0.1f, 100.0f); // Устанавливаем перспективную проекцию
    glMatrixMode(GL_MODELVIEW); // Переходим к моделированию
}

void drawCube() {
    glBegin(GL_QUADS);

    // Передняя грань
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);

    // Задняя грань
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);

    // Левая грань
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);

    // Правая грань
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f,  0.5f, -0.5f);
    glVertex3f(0.5f,  0.5f,  0.5f);
    glVertex3f(0.5f, -0.5f,  0.5f);

    // Верхняя грань
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);

    // Нижняя грань
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);

    glEnd();
}

void updateRotation() {
    // Обрабатываем ввод с клавиатуры для изменения углов вращения
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        angleY -= rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        angleY += rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        angleX -= rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        angleX += rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        angleZ -= rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        angleZ += rotationSpeed;
    }
}

int main() {
    sf::Window window(sf::VideoMode(800, 600), "SFML/OpenGL Cube Rotation", sf::Style::Default, sf::ContextSettings(24, 8, 4, 3, 3));

    // Инициализируем OpenGL
    initOpenGL();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Обновляем вращение
        updateRotation();

        // Очищаем экран
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Применяем матрицы для вращения
        glLoadIdentity(); // Сброс матрицы модели
        glTranslatef(0.0f, 0.0f, -5.0f); // Перемещаем куб

        // Вращаем куб вокруг осей X, Y, Z
        glRotatef(angleX, 1.0f, 0.0f, 0.0f);
        glRotatef(angleY, 0.0f, 1.0f, 0.0f);
        glRotatef(angleZ, 0.0f, 0.0f, 1.0f);

        // Отрисовываем куб
        drawCube();

        // Отображаем кадр
        window.display();
    }

    return 0;
}
