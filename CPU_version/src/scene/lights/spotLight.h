#pragma once

#include "light.h"
#include "scene/scene.h"

class SpotLight : public Light
{
public:
    // SpotLight Public Methods
    SpotLight(const Transform &t, const Color3f &I, Scene* s,
              Float totalWidth, Float falloffStart);

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf) const;
    Float Pdf_Li(const Intersection &, const Vector3f &) const;


    Float Falloff(const Vector3f &w) const;
    Color3f Power() const;

private:
    // SpotLight Private Data
    const Point3f pLight;
    const Color3f I;
    Scene const * const scene;
    const Float cosTotalWidth, cosFalloffStart;
};
