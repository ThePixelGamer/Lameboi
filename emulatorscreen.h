#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QTimer>

class EmulatorScreen : public QOpenGLWidget, protected QOpenGLExtraFunctions {
public:
    int radius;
    bool wireframe;
    GLuint* texDat;
    GLuint tex = 0, fbo = 0;

    EmulatorScreen(QWidget* parent = 0);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    QOpenGLContext* context;
    QOpenGLFunctions* openGLFunctions;
};
