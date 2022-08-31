#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //TODO

    Intersection isect;
    if(!scene.Intersect(ray, &isect)) {
        return Color3f(0.f);
    }
    if(depth == 0 || isect.objectHit->GetMaterial() ==nullptr)
    {
        return isect.Le(-ray.direction);
    }

    isect.ProduceBSDF();

    Vector3f wiW(0.0f);
    float pdf = 0.0f;
    Color3f f = isect.bsdf->Sample_f(-ray.direction,&wiW, sampler->Get2D(),&pdf);

    //not in same hemisphere
    if(pdf==0.0f)
    {
        return isect.Le(-ray.direction);
    }

    wiW= glm::normalize(wiW);

    Ray newRay = isect.SpawnRay(wiW);

    Color3f L = Li(newRay, scene, sampler,--depth);

    return f * L * AbsDot(wiW, isect.normalGeometric) /pdf;

}
