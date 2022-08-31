#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDir>
#include "texture.h"
#include <QFileDialog>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this), m_geomCube(this),
      m_hdrEnvMap(this), m_blueNoiseTexture(this),
      m_progPathTracer(this), m_progDisplay(this),
      m_renderPassOutputFBs{FrameBuffer2D(this, this->width(), this->height(), this->devicePixelRatio()),
                            FrameBuffer2D(this, this->width(), this->height(), this->devicePixelRatio())},
      m_glCamera(), m_mousePosPrev(), m_iterations(0), m_timer()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Set a few settings/modes in OpenGL rendering
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    m_geomSquare.create();
    m_geomCube.create();

    // Create and set up the diffuse shader
    m_progPathTracer.create(":/glsl/passthrough.vert.glsl",
                            {":/glsl/pathtracer.defines.glsl",
                             ":/glsl/pathtracer.scenes.glsl",
                             ":/glsl/pathtracer.sampleWarping.glsl",
                             ":/glsl/pathtracer.bsdf.glsl",
                             ":/glsl/pathtracer.intersection.glsl",
                             ":/glsl/pathtracer.frag.glsl"});
    m_progDisplay.create(":/glsl/passthrough.vert.glsl", ":/glsl/noOp.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    initShaderHandles();

    QString path = getCurrentPath();
    path.append("/assignment_package/environment_maps/Frozen_Waterfall_Ref.hdr");
    m_hdrEnvMap.create(path.toStdString().c_str(), false);

    path = getCurrentPath();
    path.append("/assignment_package/textures/BlueNoise_RGBA256.png");
    m_blueNoiseTexture.create(path.toStdString().c_str(), false);

    m_timer.start(16);
}

void MyGL::resizeGL(int w, int h)
{
    m_glCamera = Camera(w, h);
    m_glCamera.RecomputeAttributes();
    m_progPathTracer.setUnifVec3("u_Eye", m_glCamera.eye);
    m_progPathTracer.setUnifVec3("u_Forward", m_glCamera.look);
    m_progPathTracer.setUnifVec3("u_Right", m_glCamera.right);
    m_progPathTracer.setUnifVec3("u_Up", m_glCamera.up);

    m_renderPassOutputFBs[0].resize(width(), height(), devicePixelRatio());
    m_renderPassOutputFBs[1].resize(width(), height(), devicePixelRatio());

    m_renderPassOutputFBs[0].destroy();
    m_renderPassOutputFBs[1].destroy();

    m_renderPassOutputFBs[0].create();
    m_renderPassOutputFBs[1].create();

    m_progPathTracer.setUnifVec2("u_ScreenDims", glm::vec2(w, h));
    m_progDisplay.setUnifVec2("u_ScreenDims", glm::vec2(w, h));

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL() {
    int currBufferID = m_iterations % 2;
    int prevBufferID = (m_iterations + 1) % 2;

    // Render the next iteration of the path tracer to texture
    m_renderPassOutputFBs[currBufferID].bindFrameBuffer();
    glViewport(0,0,
               m_renderPassOutputFBs[currBufferID].width(),
               m_renderPassOutputFBs[currBufferID].height());
    glClearColor(1, 0.5, 1, 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderPassOutputFBs[prevBufferID].bindToTextureSlot(PATH_TRACER_INPUT_TEX_SLOT);
    m_progPathTracer.setUnifInt("u_AccumImg", PATH_TRACER_INPUT_TEX_SLOT);

    // Allow the path tracer shader to read the loaded environment
    // cube map
    m_hdrEnvMap.bind(ENV_MAP_FLAT_TEX_SLOT);
    m_progPathTracer.setUnifInt("u_EnvironmentMap", ENV_MAP_FLAT_TEX_SLOT);
    m_progPathTracer.setUnifInt("u_Iterations", ++m_iterations);
    m_blueNoiseTexture.bind(BLUE_NOISE_TEX_SLOT);
    m_progPathTracer.setUnifInt("u_BlueNoiseTex", BLUE_NOISE_TEX_SLOT);

    m_progPathTracer.draw(m_geomSquare);

    // Display the just-rendered iteration
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClearColor(0.5, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderPassOutputFBs[currBufferID].bindToTextureSlot(PATH_TRACER_OUTPUT_TEX_SLOT);
    m_progDisplay.setUnifInt("u_Texture", PATH_TRACER_OUTPUT_TEX_SLOT);
    m_progDisplay.setUnifInt("u_Iterations", m_iterations);

    m_progDisplay.draw(m_geomSquare);
}

void MyGL::initShaderHandles() {
    // Shader for path tracing
    m_progPathTracer.addAttrib("vs_Pos");
    m_progPathTracer.addAttrib("vs_UV");

    m_progPathTracer.addUniform("u_Eye");
    m_progPathTracer.addUniform("u_Forward");
    m_progPathTracer.addUniform("u_Right");
    m_progPathTracer.addUniform("u_Up");
    m_progPathTracer.addUniform("u_AccumImg");
    m_progPathTracer.addUniform("u_ScreenDims");
    m_progPathTracer.addUniform("u_Iterations");
    m_progPathTracer.addUniform("u_EnvironmentMap");
    m_progPathTracer.addUniform("u_BlueNoiseTex");

    // Shader for displaying the sum of all PT iterations.
    // Also applies color correction & filtering.
    m_progDisplay.addAttrib("vs_Pos");
    m_progDisplay.addAttrib("vs_UV");

    m_progDisplay.addUniform("u_Texture");
    m_progDisplay.addUniform("u_Iterations");
    m_progDisplay.addUniform("u_ScreenDims");
}

void MyGL::resetPathTracer() {
//    for(int i = 0; i < 2; ++i) {
//        m_renderPassOutputFBs[i].destroy();
//        m_renderPassOutputFBs[i].create();
//    }
    m_iterations = 0;
    m_progPathTracer.setUnifInt("u_Iterations", m_iterations);
    m_progDisplay.setUnifInt("u_Iterations", m_iterations);
}

void MyGL::tick() {
    update();
}

void MyGL::loadEnvMap() {
    QString path = getCurrentPath();
    path.append("/assignment_package/environment_maps/");
    QString filepath = QFileDialog::getOpenFileName(
                        0, QString("Load Environment Map"),
                        path, tr("*.hdr"));
    Texture2DHDR tex(this);
    try {
        tex.create(filepath.toStdString().c_str(), false);
    }
    catch(std::exception &e) {
        std::cout << "Error: Failed to load HDR image" << std::endl;
        return;
    }
    this->m_hdrEnvMap.destroy();
    this->m_hdrEnvMap = tex;
    resetPathTracer();
    update();
}

QString MyGL::getCurrentPath() const {
    QString path = QDir::currentPath();
    path = path.left(path.lastIndexOf("/"));
#ifdef __APPLE__
    path = path.left(path.lastIndexOf("/"));
    path = path.left(path.lastIndexOf("/"));
    path = path.left(path.lastIndexOf("/"));
#endif
    return path;
}
