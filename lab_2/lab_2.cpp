#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <cmath>

// Parameters for sphere and camera
float sphereRadius = 1.0f;
float cameraDistance = 5.0f;
float cameraAngle = 0.0f;

// Function to draw a sphere
void drawSphere(float radius, int slices, int stacks) {
    for (int i = 0; i < slices; ++i) {
        float theta1 = i * M_PI * 2.0f / slices;
        float theta2 = (i + 1) * M_PI * 2.0f / slices;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= stacks; ++j) {
            float phi = j * M_PI / stacks;

            float x1 = radius * sin(phi) * cos(theta1);
            float y1 = radius * cos(phi);
            float z1 = radius * sin(phi) * sin(theta1);

            float x2 = radius * sin(phi) * cos(theta2);
            float y2 = radius * cos(phi);
            float z2 = radius * sin(phi) * sin(theta2);

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}

int main() {
    // Create an SFML window
    sf::Window window(sf::VideoMode(800, 600), "3D Sphere with SFML and OpenGL", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    // Enable OpenGL features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                // Adjust sphere radius
                if (event.key.code == sf::Keyboard::Up)
                    sphereRadius += 0.1f;
                if (event.key.code == sf::Keyboard::Down)
                    sphereRadius = std::max(0.1f, sphereRadius - 0.1f);

                // Adjust camera distance
                if (event.key.code == sf::Keyboard::W)
                    cameraDistance = std::max(1.0f, cameraDistance - 0.1f);
                if (event.key.code == sf::Keyboard::S)
                    cameraDistance += 0.1f;

                // Rotate camera
                if (event.key.code == sf::Keyboard::Left)
                    cameraAngle -= 5.0f;
                if (event.key.code == sf::Keyboard::Right)
                    cameraAngle += 5.0f;
            }
        }

        // Clear the window
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, 800.0 / 600.0, 1.0, 100.0);

        // Set camera position
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            cameraDistance * sin(cameraAngle * M_PI / 180.0),
            0.0,
            cameraDistance * cos(cameraAngle * M_PI / 180.0),
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);

        // Add lighting
        GLfloat lightPos[] = { 2.0f, 2.0f, 2.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        // Draw the sphere
        glColor3f(0.5f, 0.7f, 0.9f);
        drawSphere(sphereRadius, 30, 30);

        // Display the frame
        window.display();
    }

    return 0;
}
