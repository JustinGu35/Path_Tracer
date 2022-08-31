#include "pointLight.h"
#include <scene/scene.h>

PointLight::PointLight(const Transform &t,
                     const Color3f &I,
                       Scene* s)
    : Light(t),
      pLight(Point3f(t.T()*glm::vec4(0, 0, 0, 1.f))),
      I(I),
      scene(s)
{}

Color3f PointLight::Sample_Li(const Intersection &ref, const Point2f &u,
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

    return I / glm::length2(pLight - ref.point);
}


Color3f PointLight::Power() const {
    return 4.0f * Pi * I;
}

Float PointLight::Pdf_Li(const Intersection &, const Vector3f &) const {
    return 0.f;
}
