#include "integrator.h"

void Integrator::run()
{
    Render();
}

inline Color3f reinhardOp(Color3f c) {
    // TODO
    return c/(glm::vec3(1.0f)+c);
}

inline Color3f gammaCorrect(Color3f c) {
    // TODO
    return glm::pow(c,Vector3f(1.0f/2.2f));
}

void Integrator::Render()
{
    // Compute the bounds of our sample, clamping to screen's max bounds if necessary
    // Instantiate a film tile to store this thread's pixel colors
    std::vector<Point2i> tilePixels = bounds.GetPoints();
    // For every pixel in the FilmTile:
    for(Point2i &pixel : tilePixels) {
        ///Uncomment this to debug a particular pixel within this tile
//        if(pixel.x != 200 || pixel.y != 200) {
//            continue;
//        }
//        if(pixel.x == 200 && pixel.y == 150) {
//            std::cout << "";
//        }
        Color3f pixelColor(0.f);
        // Ask our sampler for a collection of stratified samples,
        // then raycast through each sample
        std::vector<Point2f> pixelSamples = sampler->GenerateStratifiedSamples();
        for(Point2f sample : pixelSamples) {
            // _sample_ is [0, 1), but it needs to be translated to the pixel's origin.
            sample = sample + Point2f(pixel);
            // Generate a ray from this pixel sample
            Ray ray = camera->Raycast(sample);
            // Get the L (energy) for the ray by calling Li(ray, scene, tileSampler, arena)
            // Li is implemented by Integrator subclasses, like DirectLightingIntegrator
            Color3f L = Li(ray, *scene, sampler, recursionLimit);
            // Accumulate color in the pixel
            pixelColor += L;
        }
        // Average all samples' energies
        pixelColor /= pixelSamples.size();

        // TODO implement these functions in order to convert
        // our color from High Dynamic Range [0, INF]
        // to Standard RGB [0, 1]
        pixelColor = reinhardOp(pixelColor);
        pixelColor = gammaCorrect(pixelColor);

        film->SetPixelColor(pixel, glm::clamp(pixelColor, 0.f, 1.f));
    }
    //We're done here! All pixels have been given an averaged color.
}


void Integrator::ClampBounds()
{
    Point2i max = bounds.Max();
    max = Point2i(std::min(max.x, film->bounds.Max().x), std::min(max.y, film->bounds.Max().y));
    bounds = Bounds2i(bounds.Min(), max);
}


float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    if (fPdf == 0.f && gPdf == 0.f) return 0.f;
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    if (fPdf == 0.f && gPdf == 0.f) return 0.f;

    Float f = nf * fPdf;
    Float g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}


