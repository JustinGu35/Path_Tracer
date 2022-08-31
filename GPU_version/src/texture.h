#pragma once

#include "openglcontext.h"
#include "glm_includes.h"
#include "smartpointerhelp.h"
#include "stb_image.h"

// Texture slot for the 2D HDR environment map
#define ENV_MAP_FLAT_TEX_SLOT 0
// Texture slot for the 3D HDR environment cube map
#define BLUE_NOISE_TEX_SLOT 1
// Texture slot for the input of the path tracer
#define PATH_TRACER_INPUT_TEX_SLOT 2
// Texture slot for the output of the path tracer
#define PATH_TRACER_OUTPUT_TEX_SLOT 3

class Texture {
public:
    Texture(OpenGLContext* context);
    virtual ~Texture();

    virtual void create(const char *texturePath, bool wrap) = 0;
    void destroy();
    void bind(GLuint texSlot);

    bool m_isCreated;

protected:
    OpenGLContext* context;
    GLuint m_textureHandle;
};

class Texture2D : public Texture {
public:
    Texture2D(OpenGLContext* context);
    ~Texture2D();

    void create(const char *texturePath, bool wrap) override;
};

class Texture2DHDR : public Texture {
public:
    Texture2DHDR(OpenGLContext* context);
    ~Texture2DHDR();

    void create(const char *texturePath, bool wrap) override;
};
