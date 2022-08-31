#include "thinLensCamera.h"
#include <warpfunctions.h>

thinLensCamera::thinLensCamera():
    Camera(400, 400)
{
    look = Vector3f(0,0,-1);
    up = Vector3f(0,1,0);
    right = Vector3f(1,0,0);

    focalDistance = glm::length(ref - eye);
    lensRadius = 1.5f;
    sampler = new Sampler(10,0);
}
thinLensCamera::thinLensCamera(const Camera &c):
    Camera(c)
{
    focalDistance = glm::length(ref - eye);
    lensRadius = 1.5f;
    sampler = new Sampler(10,0);
}


void  thinLensCamera::setLens(float lr){

    lensRadius = lr;
    focalDistance = glm::length(ref - eye);
}

Ray thinLensCamera::Raycast(const Point2f &pt) const
{
    float ndc_x = (2.f*pt.x/width - 1);
    float ndc_y = (1 - 2.f*pt.y/height);

    glm::vec3 P = ref + ndc_x*H + ndc_y*V;
    Ray result(eye, glm::normalize(P - eye));

    if(lensRadius > 0)
    {
        Point2f pLens = Point2f(lensRadius * WarpFunctions::squareToDiskConcentric(sampler->Get2D()));
        float ft = focalDistance / fabs(result.direction.z);
        Point3f pFocus = ft * result.direction + result.origin;

        result.origin = result.origin + (up * pLens.y) + (right * pLens.x);
        result.direction = glm::normalize(pFocus - result.origin);
    }
    return result;
}
