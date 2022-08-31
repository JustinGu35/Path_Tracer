#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //TODO
    if(twoSided)
    {
        return emittedLight;
    }
    else
    {
        if(glm::dot(isect.normalGeometric, w) >0)
        {
            return emittedLight;
        }
    }
    return Color3f(0.f);
}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
   // TODO
    *pdf =0;

    Intersection pShape = shape->Sample(ref,xi, pdf);

    if (*pdf == 0.f || glm::length(pShape.point - ref.point) <  ShadowEpsilon)
    {
        return Color3f(0.f);
    }

    *wi = glm::normalize(pShape.point - ref.point);
    //*pdf = shape->Pdf(ref, *wi);
    return L(pShape, -*wi);
}

float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    // TODO
    return shape->Pdf(ref, wi);
}
