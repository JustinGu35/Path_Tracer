#include "fulllightingintegrator.h"
#include <scene/lights/diffusearealight.h>
#include "glm/gtx/component_wise.hpp"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f accumLight = Color3f(0.0f);
    Color3f rayColor = Color3f(1.f);
    Ray bounce = ray;
    bool specularBounce = false;

    for(int i = 0; i < depth; ++i)
    {
        Intersection isect;
        bool foundIntersection = scene.Intersect(bounce, &isect);
        Vector3f woW = -bounce.direction;

        if(i == 0 || specularBounce)
        {
            //Add isect.Le to our accumLight
            if(foundIntersection)
            {
                accumLight += rayColor * isect.Le(woW);
            }
        }

        //terminate if no intersection
        if(!foundIntersection) {
            break;
        }

        if(isect.objectHit->GetAreaLight())
        {
             break;
        }

        isect.ProduceBSDF();


        if(isect.bsdf->BxDFsMatchingFlags(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0)
        {
            Color3f directLight = MIS(bounce,scene, isect, sampler, depth);

            accumLight += directLight * rayColor;
        }

        // Global illumination
        Vector3f gi_woW = -bounce.direction;
        float gi_pdf = 0.0f;
        Vector3f gi_W(0.f);
        BxDFType flags;

        Color3f gi_f = isect.bsdf->Sample_f(gi_woW, &gi_W, sampler->Get2D(), &gi_pdf, BSDF_ALL, &flags);
        if(IsBlack(gi_f) || gi_pdf == 0.f)
        {
            break;
        }
        gi_W = glm::normalize(gi_W);
        rayColor *= gi_f * AbsDot(gi_W, isect.normalGeometric) / gi_pdf;

        bounce = isect.SpawnRay(gi_W);
        specularBounce = ((flags & BSDF_SPECULAR) != 0);

        if (i > 3) {
            float max = ( rayColor[0] < rayColor[1] ) ? rayColor[1] : rayColor[0];
            max = ( ( max < rayColor[2] ) ? rayColor[2] : max );
            Float q = std::max(0.05f, 1.f - max);
            if (sampler->Get1D() < q)
            {
                break;
            }
            rayColor /= 1 - q;
        }
    }

    return accumLight;
}

Color3f FullLightingIntegrator::MIS(const Ray &ray, const Scene &scene,  const Intersection &isect, std::shared_ptr<Sampler> sampler, int depth) const
{
    //Intersection isect;
    Vector3f woW = -ray.direction;

    Color3f result(0.0f);

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
                (glm::length(hitPos-oriPos) > glm::length(light->transform.position()-oriPos)))
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
