#pragma once

#include "camera.h"
#include <samplers/sampler.h>

//A perspective projection camera
//Receives its eye position and reference point from the scene XML file
class thinLensCamera : public Camera
{
public:
    thinLensCamera();
    thinLensCamera(const Camera &c);
    ~thinLensCamera(){delete sampler;}

    void setLens(float lr);
    Ray Raycast(const Point2f &pt) const;

    Sampler* sampler;
    float focalDistance, lensRadius;

};
