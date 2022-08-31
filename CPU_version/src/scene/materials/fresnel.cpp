#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    return Color3f(FrDielectric(cosThetaI, etaI, etaT));
}

float FrDielectric(float cosThetaI, Float etaI, Float etaT)
{
    //TODO
    cosThetaI = glm::clamp(cosThetaI, -1.0f, 1.0f);
    //Potentially swap indices of refraction
    bool entering = cosThetaI > 0.f;
    if (!entering) {
       std::swap(etaI, etaT);
       cosThetaI = std::abs(cosThetaI);
    }

    //Compute cosThetaT using Snell’s law
    Float sinThetaI = std::sqrt(std::max((Float)0, 1 - cosThetaI * cosThetaI));
    Float sinThetaT = etaI / etaT * sinThetaI;
    //Handle total internal reflection
    Float cosThetaT = std::sqrt(std::max((Float)0, 1 - sinThetaT * sinThetaT));

    Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2;
}
