#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
    : worldToTangent(glm::transpose(Matrix3x3(isect.tangent, isect.bitangent, isect.normalGeometric))),
      tangentToWorld(Matrix3x3(isect.tangent, isect.bitangent, isect.normalGeometric)),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    tangentToWorld = Matrix3x3(t, b, n);
    worldToTangent = glm::transpose(tangentToWorld);
}


// Computes the overall light scattering properties of a point on a Material.
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    // Convert the incoming and outgoing light rays from
    // world space to local tangent space
    Vector3f wi = worldToTangent * wiW;
    Vector3f wo = worldToTangent * woW;

    // If the outgoing ray is parallel to the surface,
    // we know we can return black b/c the Lambert term
    // in the overall Light Transport Equation will be 0.
    if (wo.z == 0) return Color3f(0.f);

    // Was wi reflected off the surface, or transmitted through it?
    bool reflect = glm::dot(wiW, normal) * glm::dot(woW, normal) > 0;

    // Iterate over every BxDF that composes this BSDF and
    // invoke its f(), if and only if it would contribute light
    // along wi (e.g. only reflection BxDFs contribute to reflected wis,
    // only transmission BxDFs contribute to transmitted wis).
    // We sum up each f() result to get the overall light scattering
    // properties of this BSDF, given the directions in which light enters
    // and exits the surface.
    Color3f f(0.f);
    for(int i = 0; i < numBxDFs; i++)
    {
        if(bxdfs[i]->MatchesFlags(flags) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                 (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
        {
            f += bxdfs[i]->f(wo, wi);
        }
    }
    return f;
}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, Point2f xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    // See if any BxDFs could be sampled at all
    int matchingComps = BxDFsMatchingFlags(type);
    if (matchingComps == 0)
    {
        *pdf = 0;
        if (sampledType) *sampledType = BxDFType(0);
        return Color3f(0.f);
    }

    // Choose a BxDF at random from the set of candidates:

    // Get an index that falls within the set of matching BxDFs using one of our
    // pair of uniform random variables
    int comp = std::min((int)std::floor(xi[0] * matchingComps), matchingComps - 1);
    BxDF* bxdf = nullptr;
    int count = comp;
    for (int i = 0; i < numBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }
    }

    // Remap BxDF sample xi to [0,1) in x and y
    // If we don't do this, then we bias the _wi_ sample
    // we'll get from BxDF::Sample_f, e.g. if we have two
    // BxDFs each with a probability of 0.5 of being chosen, then
    // when we sample the first BxDF (xi[0] = [0, 0.5)) we'll ALWAYS
    // use a value between 0 and 0.5 to generate our wi for that BxDF.
    Point2f xiRemapped(std::min(xi[0] * matchingComps - comp, OneMinusEpsilon), xi[1]);

    // Sample chosen BxDF
    Vector3f wi;
    Vector3f wo = worldToTangent * woW; // Remember to convert woW to tangent space!
    if (wo.z == 0) return Color3f(0.f); // The tangent-space wo is perpendicular to the normal,
    // so Lambert's law reduces its contribution to 0.
    *pdf = 0;
    if (sampledType) {*sampledType = bxdf->type;}

    // Sample a tangent-space wi based on the BxDF's PDF and then
    // compute the BxDF::f of the chosen BxDF based on wi.
    Color3f f = bxdf->Sample_f(wo, &wi, xiRemapped, pdf, sampledType);
    if (*pdf == 0)
    {
        if (sampledType) {*sampledType = BxDFType(0);}
        return Color3f(0.f);
    }
    *wiW = tangentToWorld * wi;

    // Compute overall PDF with all matching BxDFs
    // We treat specular BxDFs differently because their PDFs
    // are delta distributions (0 in all cases but the *one* case
    // where wi = reflect(wo, N), and 1 in that case)
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        for (int i = 0; i < numBxDFs; ++i)
        {
            if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
            {
                *pdf += bxdfs[i]->Pdf(wo, wi);
            }
        }
    }
    if (matchingComps > 1) *pdf /= matchingComps;

    // Compute the overall value of this BSDF for sampled direction wiW
    // This means looking at the rest of the BxDFs that match _type_
    // and invoking their implementations of BxDF::f.
    // Again, we're going to skip this if the randomly chosen
    // BxDF is specular because its PDF is a delta distribution.
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        bool reflect = glm::dot(*wiW, normal) * glm::dot(woW, normal) > 0;
        f = Color3f(0.f);
        for (int i = 0; i < numBxDFs; ++i)
        {
            if (bxdfs[i]->MatchesFlags(type) &&
                    ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                     (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
            {
                f += bxdfs[i]->f(wo, wi);
            }
        }
    }
    return f;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    if (numBxDFs == 0.f) return 0.f;

    Vector3f wo = worldToTangent * woW;
    Vector3f wi = worldToTangent * wiW;

    if (wo.z == 0) return 0.; // The cosine of this vector would be zero

    Float pdf = 0.f;
    int matchingComps = 0;
    // Iterate through all our BxDFs and compute the PDF
    // of each BxDF that matches the flags put into this function.
    // Sum the PDFs.
    for (int i = 0; i < numBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(flags))
        {
            ++matchingComps;
            pdf += bxdfs[i]->Pdf(wo, wi);
        }
    }
    // Take the average of all PDFs that could have been sampled.
    float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
    return v;
}

// Uniformly sample a hemisphere to generate wi,
// then invoke f() on the wo and wi you now have.
// When generating wi, make sure to *= -1 if its z coord is < 0.
// This ensures it's in the same hemisphere as wo.
// Also don't forget to set *pdf to the appropriate value.
// We implement this function so that when you are given new
// BxDF subclasses to write, you can test-render your scenes
// without having implemented their specific Sample_f()s yet.
Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereUniform(xi);
    if (wi->z < 0)
    {
        wi->z = - wi->z;
    }
    *pdf=Pdf(wo,*wi);
    return f(wo,*wi);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? InvPi : 0.0f;
}

BSDF::~BSDF()
{
    for(int i = 0; i < numBxDFs; i++)
    {
        delete bxdfs[i];
    }
}

