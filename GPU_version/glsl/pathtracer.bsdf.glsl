vec3 FresnelDielectricEval(float cosThetaI) {
    // We will hard-code the indices of refraction to be
    // those of glass
    float etaI = 1.;
    float etaT = 1.55;
    cosThetaI = clamp(cosThetaI, -1.f, 1.f);
    // Potentially swap indices of refraction
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        float tmp = etaI;
        etaI = etaT;
        etaT = tmp;
        cosThetaI = abs(cosThetaI);
    }

    // Compute _cosThetaT_ using Snell's law
    float sinThetaI = sqrt(max(0., 1. - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    // Handle total internal reflection
    if (sinThetaT >= 1) return vec3(1.);
    float cosThetaT = sqrt(max(0., 1. - sinThetaT * sinThetaT));
    float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return vec3((Rparl * Rparl + Rperp * Rperp) / 2.);
}

vec3 f_diffuse(vec3 albedo) {
    // TODO
    return albedo * INV_PI; // R/Pi
}

vec3 Sample_f_diffuse(vec3 albedo, vec2 xi, vec3 nor,
                      out vec3 wiW, out float pdf, out int sampledType) {
    // TODO
    // Make sure you set wiW to a world-space ray direction,
    // since wo is in tangent space. You can use
    // the function LocalToWorld() in the "defines" file
    // to easily make a mat3 to do this conversion.


    vec3 wi = squareToHemisphereCosine(xi);
    pdf = squareToHemisphereCosinePDF(wi);
    wiW = LocalToWorld(nor) * wi;

    return albedo * INV_PI;
}

vec3 Sample_f_specular_refl(vec3 albedo, vec3 nor, vec3 wo,
                            out vec3 wiW, out int sampledType) {
    // TODO
    // Make sure you set wiW to a world-space ray direction,
    // since wo is in tangent space
    vec3 wi = vec3(-wo.x,-wo.y,wo.z);
    if(wo.z<0)
    {
        wi.z *= -1;
    }
    wiW = LocalToWorld(nor) * wi;

    return albedo * vec3(1.0f) /AbsCosTheta(wi);
}

vec3 Sample_f_specular_trans(vec3 albedo, vec3 nor, vec3 wo,
                             out vec3 wiW, out int sampledType) {
    // Hard-coded to index of refraction of glass
    float etaA = 1.;
    float etaB = 1.55;

    bool entering = CosTheta(wo) > 0;
    float etaI = entering ? etaA : etaB;
    float etaT = entering ? etaB : etaA;

    // TODO
    // Make sure you set wiW to a world-space ray direction,
    // since wo is in tangent space
    vec3 wi;
    if(!Refract(wo,Faceforward(vec3(0.,0.,1.), wo), etaI/etaT, wi))
    {
        return vec3(0.);
    }

    vec3 T = vec3(1.0f);
    vec3 ft = T * (vec3(1.) - vec3(0.0f));
    //ft *= (etaI * etaI) / (etaT * etaT);

    wiW = LocalToWorld(nor) * wi;

    return ft/ AbsCosTheta(wi);
}

uvec2 seed;
float rng() {
    seed += uvec2(1);
    uvec2 q = 1103515245U * ( (seed >> 1U) ^ (seed.yx) );
    uint  n = 1103515245U * ( (q.x) ^ (q.y >> 3U) );
    return float(n) * (1.0 / float(0xffffffffU));
}

vec3 Sample_f_glass(vec3 albedo, vec3 nor, vec2 xi, vec3 wo,
                    out vec3 wiW, out int sampledType) {
    // TODO: Sample the specular BRDF component of glass
    // half the time, and sample the specular BTDF component
    // the other half. You can do this simply by choosing one
    // or the other based on whether the current render iteration
    // (u_Iterations) is even or odd.

    // Make sure you remember to incorporate the Fresnel Dielectric
    // scalar in both BxDF return values.

    // As in the CPU path tracer, make sure to double the output
    // of both evaluations since we sample either of them only
    // half the time.
    vec3 result;
    if(rng()>=0.5)
    {
        vec3 wi = vec3(-wo.x,-wo.y,wo.z);
        if(wo.z<0)
        {
            wi.z *= -1;
        }
        wiW = LocalToWorld(nor) * wi;

        result =  albedo *  FresnelDielectricEval(CosTheta(wi)) /AbsCosTheta(wi);
    }
    else
    {
        float etaA = 1.;
        float etaB = 1.55;

        bool entering = CosTheta(wo) > 0;
        float etaI = entering ? etaA : etaB;
        float etaT = entering ? etaB : etaA;

        vec3 wi;
        if(!Refract(wo,Faceforward(vec3(0.,0.,1.), wo), etaI/etaT, wi))
        {
            return vec3(0.);
        }

        vec3 T = vec3(1.0f);
        vec3 ft = T * (vec3(1.) - FresnelDielectricEval(CosTheta(wi)));
        //ft *= (etaI * etaI) / (etaT * etaT);

        wiW = LocalToWorld(nor) * wi;

        result = ft/ AbsCosTheta(wi);
    }

    return result * 2.;
}

// ===================== Begin Microfacet BRDF Implementation ======================
vec3 Sample_wh(vec3 wo, vec2 xi, float roughness) {
    vec3 wh;

    float cosTheta = 0;
    float phi = TWO_PI * xi[1];
    // We'll only handle isotropic microfacet materials
    float tanTheta2 = roughness * roughness * xi[0] / (1.0f - xi[0]);
    cosTheta = 1 / sqrt(1 + tanTheta2);

    float sinTheta =
            sqrt(max(0.f, 1.f - cosTheta * cosTheta));

    wh = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    if (!SameHemisphere(wo, wh)) wh = -wh;

    return wh;
}

float TrowbridgeReitzD(vec3 wh, float roughness) {
    float tan2Theta = Tan2Theta(wh);
    if (isinf(tan2Theta)) return 0.f;

    float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);

    float e =
            (Cos2Phi(wh) / (roughness * roughness) + Sin2Phi(wh) / (roughness * roughness)) *
            tan2Theta;
    return 1 / (PI * roughness * roughness * cos4Theta * (1 + e) * (1 + e));
}

float Lambda(vec3 w, float roughness) {
    float absTanTheta = abs(TanTheta(w));
    if (isinf(absTanTheta)) return 0.;

    // Compute alpha for direction w
    float alpha =
            sqrt(Cos2Phi(w) * roughness * roughness + Sin2Phi(w) * roughness * roughness);
    float alpha2Tan2Theta = (roughness * absTanTheta) * (roughness * absTanTheta);
    return (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2;
}

float TrowbridgeReitzG(vec3 wo, vec3 wi, float roughness) {
    return 1 / (1 + Lambda(wo, roughness) + Lambda(wi, roughness));
}

float TrowbridgeReitzPdf(vec3 wo, vec3 wh, float roughness) {
    return TrowbridgeReitzD(wh, roughness) * AbsCosTheta(wh);
}

vec3 f_microfacet_refl(vec3 albedo, vec3 wo, vec3 wi, float roughness) {
    float cosThetaO = AbsCosTheta(wo);
    float cosThetaI = AbsCosTheta(wi);
    vec3 wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0 || cosThetaO == 0) return vec3(0.f);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return vec3(0.f);
    wh = normalize(wh);
    // TODO: Handle different Fresnel coefficients
    vec3 F = vec3(1.);//fresnel->Evaluate(glm::dot(wi, wh));
    float D = TrowbridgeReitzD(wh, roughness);
    float G = TrowbridgeReitzG(wo, wi, roughness);
    return albedo * D * G * F /
            (4 * cosThetaI * cosThetaO);
}

vec3 Sample_f_microfacet_refl(vec3 albedo, vec3 nor, vec2 xi, vec3 wo, float roughness,
                              out vec3 wiW, out float pdf, out int sampledType) {
    if (wo.z == 0) return vec3(0.);

    vec3 wh = Sample_wh(wo, xi, roughness);
    vec3 wi = reflect(-wo, wh);
    wiW = LocalToWorld(nor) * wi;
    if (!SameHemisphere(wo, wi)) return vec3(0.f);

    // Compute PDF of _wi_ for microfacet reflection
    pdf = TrowbridgeReitzPdf(wo, wh, roughness) / (4 * dot(wo, wh));
    return f_microfacet_refl(albedo, wo, wi, roughness);
}
// ===================== End Microfacet BRDF Implementation ======================

vec3 f(Intersection isect, vec3 wo, vec3 wi) {
    // Convert wo and wi to local space from world space.
    // The various f()s assume this is the case.
    wo = inverse(LocalToWorld(isect.nor)) * wo;
    wi = inverse(LocalToWorld(isect.nor)) * wi;

    if(isect.material.type == DIFFUSE_REFL) {
        return f_diffuse(isect.material.albedo);
    }
    else if(isect.material.type == SPEC_REFL ||
            isect.material.type == SPEC_TRANS ||
            isect.material.type == GLASS) {
        return vec3(0.);
    }
    else if(isect.material.type == MICROFACET_REFL) {
        return f_microfacet_refl(isect.material.albedo, wo, wi, isect.material.roughness);
    }
    // Default case, unhandled material
    else {
        return vec3(1,0,1);
    }
}

vec3 Sample_f(Intersection isect, vec3 wo, vec2 xi, out vec3 wiW, out float pdf, out int sampledType) {
    // Convert wo to local space from world space.
    // The various Sample_f()s return a wi in world space,
    // but assume wo is in local space.
    wo = inverse(LocalToWorld(isect.nor)) * wo;

    if(isect.material.type == DIFFUSE_REFL) {
        return Sample_f_diffuse(isect.material.albedo, xi, isect.nor, wiW, pdf, sampledType);
    }
    else if(isect.material.type == SPEC_REFL) {
        pdf = 1.;
        return Sample_f_specular_refl(isect.material.albedo, isect.nor, wo, wiW, sampledType);
    }
    else if(isect.material.type == SPEC_TRANS) {
        pdf = 1.;
        return Sample_f_specular_trans(isect.material.albedo, isect.nor, wo, wiW, sampledType);
    }
    else if(isect.material.type == GLASS) {
        pdf = 1.;
        return Sample_f_glass(isect.material.albedo, isect.nor, xi, wo, wiW, sampledType);
    }
    else if(isect.material.type == MICROFACET_REFL) {
        return Sample_f_microfacet_refl(isect.material.albedo,
                                        isect.nor, xi, wo,
                                        isect.material.roughness,
                                        wiW, pdf,
                                        sampledType);
    }
    // Default case, unhandled material
    else {
        return vec3(1,0,1);
    }
}
