#include "emulatorscreen.h"

EmulatorScreen::EmulatorScreen(QWidget* parent) : QOpenGLWidget(parent) {}

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
}

void EmulatorScreen::paintGL() { //most of the shit I use has been depreciated/removed since like 3.0 so uh x d
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, PPU::pixels);
    glBindTexture(GL_TEXTURE_2D, tex);
    glLoadIdentity();
    glScalef((256.0f * 3) / 160, (256.0f * 3) / 144, 0.0f);
    glTranslatef((1.0f / 384) * 48 /*(48 - xoffset)*/, (1.0f / 384) * 56/*(56 - yoffset)*/, 0.0f); //each pixel is 0.0078125f and this sets the window to 0,0 - offset
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(-1,-1);
        glTexCoord2f(3,0); glVertex2f(1,-1);
        glTexCoord2f(3,3); glVertex2f(1,1);
        glTexCoord2f(0,3); glVertex2f(-1,1);
    glEnd();
}

void EmulatorScreen::resizeGL(int w, int h) {} //I don't need this ???

QSize EmulatorScreen::minimumSizeHint() const {
    return QSize(160, 144);
}
