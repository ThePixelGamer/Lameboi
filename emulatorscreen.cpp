#include "emulatorscreen.h"
#include <gl/GLU.h>
#include <gl/GL.h>

EmulatorScreen::EmulatorScreen(QWidget* parent) : QOpenGLWidget(parent) {
    setGeometry(0, 0, parent->width(), parent->height());

    texDat = new GLuint[160*144];
    for (int i = 0; i < 23040; ++i) {
        texDat[i] = ((rand() % 256) << 24) + ((rand() % 256) << 16) + ((rand() % 256) << 8) + 0xFF;
    }
}

void EmulatorScreen::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0,0.0,0.0,0.0);
    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);  //Always set the base and max mipmap levels of a texture.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 160, 144, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texDat);
}

void EmulatorScreen::paintGL() {
    for (int i = 0; i < 23040; ++i) {
        texDat[i] = ((rand() % 256) << 24) + ((rand() % 256) << 16) + ((rand() % 256) << 8) + 0xFF;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 160, 144, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texDat);

    glBindTexture(GL_TEXTURE_2D,tex);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(-1,-1);
        glTexCoord2f(1.5,0); glVertex2f(1,-1);
        glTexCoord2f(1.5,1.5); glVertex2f(1,1);
        glTexCoord2f(0,1.5); glVertex2f(-1,1);
    glEnd();
}

void EmulatorScreen::resizeGL(int w, int h) {
    setGeometry(0, 0, w, h);
}
