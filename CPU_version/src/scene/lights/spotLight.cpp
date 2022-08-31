#include "spotLight.h"

// SpotLight Method Definitions
SpotLight::SpotLight(const Transform &t,
                     const Color3f &I,
                     Scene* s,
                     Float totalWidth, Float falloffStart)
    : Light(t),
      pLight(Point3f(t.T()*glm::vec4(0, 0, 0, 1.f))),
      I(I),
      scene(s),
      cosTotalWidth(std::cos(Pi *totalWidth / 180.f)),
      cosFalloffStart(std::cos(Pi * falloffStart / 180.f))
    {}

Color3f SpotLight::Sample_Li(const Intersection &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf) const {
    Vector3f oriPos = ref.point;
    *wi = glm::normalize(pLight - oriPos);
    *pdf = 1.f;

//    Intersection isect;
//    Ray testRay = ref.SpawnRay(*wi);
//    if(!scene->Intersect(testRay,&isect))
//    {
//        return Color3f(0.f);
//    }
//    Vector3f hitPos = isect.point;
//    if(glm::length(hitPos-oriPos) < glm::length(pLight-oriPos))
//    {
//        return Color3f(0.f);
//    }
    return I * Falloff(-*wi) / glm::length2(pLight - ref.point);
}

Float SpotLight::Falloff(const Vector3f &w) const {
    Vector3f wl = glm::normalize(transform.rotateT() * w);
    Float cosTheta = -wl.z;
    if (cosTheta < cosTotalWidth) return 0;
    if (cosTheta >= cosFalloffStart) return 1;
    // Compute falloff inside spotlight cone
    Float delta =
        (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);
    return (delta * delta) * (delta * delta);
}

Color3f SpotLight::Power() const {
    return I * 2.0f * Pi * (1.0f - 0.5f * (cosFalloffStart + cosTotalWidth));
}

Float SpotLight::Pdf_Li(const Intersection &, const Vector3f &) const {
    return 0.f;
}
