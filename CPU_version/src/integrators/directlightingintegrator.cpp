#include "directlightingintegrator.h"

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //TODO
    Intersection isect;
    Vector3f woW = -ray.direction;
    if(!scene.Intersect(ray, &isect)) {
        return Color3f(0.f);
    }

    if(isect.objectHit->GetMaterial() ==nullptr)
    {
        return isect.Le(woW);
    }

    Color3f result = isect.Le(woW);

    isect.ProduceBSDF();
    Vector3f wiW(0.0f);
    float pdf = 0.0f;
    int nLights = int(scene.lights.size());
    if (nLights == 0) return Color3f(0.f);
    int lightNum = std::min((int)(sampler->Get1D() * nLights), nLights - 1);
    const std::shared_ptr<Light> &light = scene.lights[lightNum];


    Color3f lightColor = light->Sample_Li(isect, sampler->Get2D(), &wiW, &pdf);
    Color3f f = isect.bsdf->f(woW, wiW);

    //test if obstcles on the way
    Ray testRay = isect.SpawnRay(wiW);
    Intersection testIsect;

    if (scene.Intersect(testRay, &testIsect))
    {
        Vector3f hitPos = testIsect.point;
        Vector3f oriPos = isect.point;
        if (testIsect.objectHit->GetAreaLight() == light.get() ||
                (glm::length(hitPos-oriPos) >= glm::length(light->transform.position()-oriPos)))
        {
            if (pdf != 0.f)
            {
                pdf/= nLights;
                float brdfPdf = isect.bsdf->Pdf(woW, wiW);
                float weight = PowerHeuristic(1, pdf, 1, brdfPdf);
                //float weight = BalanceHeuristic(1, pdf, 1, brdfPdf);
                result += weight * f * lightColor * AbsDot(wiW, isect.normalGeometric) /pdf;
            }
        }
    }
    //BRDF PDF
    wiW = Vector3f(0.0f);
    pdf = 0.0f;
    f = isect.bsdf->Sample_f(woW,&wiW, sampler->Get2D(),&pdf);
    if(pdf==0.0f)
    {
        return result;
    }
    wiW = glm::normalize(wiW);

    testRay = isect.SpawnRay(wiW);

    float lightPdf = scene.lights[lightNum]->Pdf_Li(isect, wiW);
    lightPdf /= nLights;

    float weight = 1.f;
    Color3f secondColor(0.0f);
    // if PDF of light is nonzero given w_j
    if (lightPdf > 0.f)
    {
       weight = PowerHeuristic(1, pdf, 1, lightPdf);
       //weight = BalanceHeuristic(1, pdf, 1, lightPdf);
       if (scene.Intersect(testRay, &testIsect))
       {
           if (testIsect.objectHit->GetAreaLight() == scene.lights[lightNum].get())
           {
               secondColor = testIsect.Le(-wiW);
           }
       }
    }
    result += weight * f * secondColor * AbsDot(wiW, isect.normalGeometric) / pdf;

    return result;
}
