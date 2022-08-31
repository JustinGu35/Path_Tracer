#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    //TODO
    Intersection insectLight = Sample(xi,pdf);

    Vector3f wi = glm::normalize(ref.point - insectLight.point);
    float cos = AbsDot(glm::normalize(insectLight.normalGeometric), wi);

    if(cos == 0.f)
    {
        *pdf = 0.f;
    }
    else
    {
        *pdf *= glm::length2(ref.point - insectLight.point)/ cos;
    }
    return insectLight;
}

float Shape::Pdf(const Intersection &ref, const Vector3f &wi) const {
    //TODO
    Ray ray = ref.SpawnRay(wi);
    Intersection isectLight;
    if (!Intersect(ray, &isectLight))
    {
        return 0;
    }

    float cos = AbsDot(glm::normalize(isectLight.normalGeometric), -wi);
    if(cos == 0)
    {
        return 0.f;
    }

    Float pdf =  glm::length2(ref.point - isectLight.point) / (cos * Area());


    return pdf;
}
