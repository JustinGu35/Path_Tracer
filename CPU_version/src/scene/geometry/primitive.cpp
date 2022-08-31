#include "primitive.h"


Primitive::Primitive() :
    name("Some Primitive"), shape(nullptr), material(nullptr), areaLight(nullptr)
{}
Primitive::Primitive(sPtr<Shape> shape, sPtr<Material> material, sPtr<AreaLight> areaLight)
    : shape(shape),
      material(material),
      areaLight(areaLight)
{}

bool Primitive::Intersect(const Ray &r, Intersection *isect) const
{
    if(!shape->Intersect(r, isect)) return false;
    isect->objectHit = this;
    // We create a BSDF for this intersection in our Integrator classes
    return true;
}

bool Primitive::ProduceBSDF(Intersection *isect) const
{
    if(material)
    {
        material->ProduceBSDF(isect);
        return true;
    }
    return false;
}

const AreaLight* Primitive::GetAreaLight() const
{
    return areaLight.get();
}


const Material* Primitive::GetMaterial() const
{
    return material.get();
}
