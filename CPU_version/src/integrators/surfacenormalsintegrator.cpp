#include "surfacenormalsintegrator.h"

Color3f SurfaceNormalsIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Intersection isect;
    if(!scene.Intersect(ray, &isect)) {
        return Color3f(0.f);
    }

    return (isect.normalGeometric + Vector3f(1.f)) * 0.5f;
}
