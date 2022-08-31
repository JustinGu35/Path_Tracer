#pragma once
#include "shape.h"
#include <scene/lights/light.h>

class AreaLight;
class Shape;

// A class that holds all the information for a given object in a scene,
// such as its Shape, its Material, and (if applicable) ita AreaLight
// Parallels the GeometricPrimitive class from PBRT
class Primitive
{
public:
    Primitive();
    Primitive(sPtr<Shape> shape,
              sPtr<Material> material = nullptr,
              sPtr<AreaLight> areaLight = nullptr);
    // Returns whether or not the given Ray intersects this Primitive.
    // Passes additional intersection data through the Intersection pointer
    bool Intersect(const Ray& r, Intersection* isect) const;

    const AreaLight* GetAreaLight() const;
    const Material* GetMaterial() const;
    // Ask our _material_ to generate a BSDF containing
    // BxDFs and store it in isect.
    bool ProduceBSDF(Intersection *isect) const;


    QString name;//Mainly used for debugging purposes
    sPtr<Shape> shape;
    sPtr<Material> material;
    sPtr<AreaLight> areaLight;
};
