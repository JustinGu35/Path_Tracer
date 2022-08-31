#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include "scene/cube.h"
#include "scene/squareplane.h"
#include "texture.h"
#include "camera.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include "framebuffer.h"
#include <array>
#include <QTimer>



class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;
    // Used for rendering each face of the cubemap to a frame buffer
    // Also used to render the cube map as a background
    Cube m_geomCube;
    // Used for loading the HDR environment map from disk to the GPU
    Texture2DHDR m_hdrEnvMap;
    Texture2D m_blueNoiseTexture;

    ShaderProgram m_progPathTracer;
    ShaderProgram m_progDisplay;
    std::array<FrameBuffer2D, 2> m_renderPassOutputFBs;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;
    glm::vec2 m_mousePosPrev;
    int m_iterations;
    QTimer m_timer;


public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void initShaderHandles();

    void resetPathTracer();

protected:
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
    QString getCurrentPath() const;

public slots:
    void tick();
    void loadEnvMap();
};


#endif // MYGL_H
