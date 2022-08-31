#pragma once

#include "light.h"
#include "scene/scene.h"

class PointLight : public Light
{
public:
    PointLight(const Transform &t, const Color3f &I, Scene* s);

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf) const;
    Float Pdf_Li(const Intersection &, const Vector3f &) const;
    Color3f Power() const;

private:
    const Point3f pLight;
    const Color3f I;
    Scene const * const scene;
};
