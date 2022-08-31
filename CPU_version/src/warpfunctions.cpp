#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>
#include "globals.h"

glm::vec3 WarpFunctions::squareToDiskUniform(const glm::vec2 &sample)
{
    //TODO
    float r = std::sqrt(sample[0]);
    float theta = 2 * M_PI * sample[1];
    return glm::vec3(r*std::cos(theta),r*std::sin(theta),0);
}

glm::vec3 WarpFunctions::squareToDiskConcentric(const glm::vec2 &sample)
{
    //TODO
    glm::vec2 offset = 2.f * sample - glm::vec2(1,1);
    if(fequal(offset[0],0.0f) && fequal(offset[1],0.0f) )
    {
        return glm::vec3(0,0,0);
    }

    float theta, r;
    if(std::abs(offset.x) > std::abs(offset.y))
    {
        r = offset.x;
        theta = M_PI/4.f * (offset.y/offset.x);
    }
    else
    {
        r = offset.y;
        theta = M_PI/2.0f - M_PI/4.f * (offset.x/offset.y);
    }
    return r * glm::vec3(std::cos(theta),std::sin(theta),0);
}

float WarpFunctions::squareToDiskPDF(const glm::vec3 &sample)
{
    //TODO
    return InvPi;
}

glm::vec3 WarpFunctions::squareToSphereUniform(const glm::vec2 &sample)
{
    //TODO
    float z = 1-2*sample[0];
    float r = std::sqrt(std::max((float)0, (float)1. - z * z));
    float phi = 2 * M_PI * sample[1];
    return glm::vec3(r * std::cos(phi), r * std::sin(phi), z);
}

float WarpFunctions::squareToSphereUniformPDF(const glm::vec3 &sample)
{
    //TODO
    return Inv4Pi;
}

glm::vec3 WarpFunctions::squareToSphereCapUniform(const glm::vec2 &sample, float thetaMin)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented sphere cap warping!");
}

float WarpFunctions::squareToSphereCapUniformPDF(const glm::vec3 &sample, float thetaMin)
{
    //TODO
    return 0;
}

glm::vec3 WarpFunctions::squareToHemisphereUniform(const glm::vec2 &sample)
{
    //TODO
    float z = sample[0];
    float r = std::sqrt(std::max((float)0, (float)1. - z * z));
    float phi = 2 * M_PI * sample[1];
    return glm::vec3(r * std::cos(phi), r * std::sin(phi), z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const glm::vec3 &sample)
{
    //TODO
    return Inv2Pi;
}

glm::vec3 WarpFunctions::squareToHemisphereCosine(const glm::vec2 &sample)
{
    //TODO
    glm::vec3 d = squareToDiskConcentric(sample);
    float z = std::sqrt(std::max((float)0, 1 - d.x * d.x - d.y * d.y));
    return glm::vec3(d.x, d.y, z);
}

float WarpFunctions::squareToHemisphereCosinePDF(const glm::vec3 &sample)
{
    //TODO
    return sample[2] * InvPi;
}
