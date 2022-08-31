#include <scene/scene.h>

#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/squareplane.h>
#include <scene/materials/mattematerial.h>
#include <scene/lights/diffusearealight.h>


Scene::Scene()
{}

void Scene::SetCamera(const Camera &c)
{
    camera = Camera(c);
    camera.create();
//    camera = thinLensCamera(c);
//    camera.create();

    film.SetDimensions(c.width, c.height);
}

bool Scene::Intersect(const Ray &ray, Intersection *isect) const
{
    bool result = false;
    for(auto &p : primitives)
    {
        Intersection testIsect;
        if(p->Intersect(ray, &testIsect))
        {
            if(testIsect.t < isect->t || isect->t < 0)
            {
                *isect = testIsect;
                result = true;
            }
        }
    }
    return result;
}

void Scene::CreateTestScene()
{
    //Floor
    //Area light
    //Figure in front of light

    auto matteWhite = mkS<MatteMaterial>(Color3f(0.725, 0.71, 0.68), 0, nullptr, nullptr);
    auto matteRed = mkS<MatteMaterial>(Color3f(0.63, 0.065, 0.05), 0, nullptr, nullptr);
    auto matteGreen = mkS<MatteMaterial>(Color3f(0.14, 0.45, 0.091), 0, nullptr, nullptr);

    // Floor, which is a large white plane
    sPtr<Shape> floor = mkS<SquarePlane>();
    floor->transform = Transform(Vector3f(0,-2.5,0), Vector3f(-90,0,0), Vector3f(10,10,1));
    auto floorPrim = mkU<Primitive>(std::move(floor));
    floorPrim->material = matteWhite;
    floorPrim->name = QString("Floor");

    // Left wall, which is a large red plane
    sPtr<Shape> leftWall = mkS<SquarePlane>();
    leftWall->transform = Transform(Vector3f(5,2.5,0), Vector3f(0,-90,0), Vector3f(10,10,1));
    auto leftWallPrim = mkU<Primitive>(std::move(leftWall));
    leftWallPrim->material = matteRed;
    leftWallPrim->name = QString("Left Wall");

    // Right wall, which is a large green plane
    sPtr<Shape> rightWall = mkS<SquarePlane>();
    rightWall->transform = Transform(Vector3f(-5,2.5,0), Vector3f(0,90,0), Vector3f(10,10,1));
    auto rightWallPrim = mkU<Primitive>(std::move(rightWall));
    rightWallPrim->material = matteGreen;
    rightWallPrim->name = QString("Right Wall");

    // Back wall, which is a large white plane
    sPtr<Shape> backWall = mkS<SquarePlane>();
    backWall->transform = Transform(Vector3f(0,2.5,5), Vector3f(0,180,0), Vector3f(10,10,1));
    auto backWallPrim = mkU<Primitive>(std::move(backWall));
    backWallPrim->material = matteWhite;
    backWallPrim->name = QString("Back Wall");

    // Ceiling, which is a large white plane
    sPtr<Shape> ceiling = mkS<SquarePlane>();
    ceiling->transform = Transform(Vector3f(0,7.5,0), Vector3f(90,0,0), Vector3f(10,10,1));
    auto ceilingPrim = mkU<Primitive>(std::move(ceiling));
    ceilingPrim->material = matteWhite;
    ceilingPrim->name = QString("Ceiling");

    // Long cube
    sPtr<Shape> longCube = mkS<Cube>();
    longCube->transform = Transform(Vector3f(2, 0, 3), Vector3f(0, 27.5, 0), Vector3f(3, 6, 3));
    auto longCubePrim = mkU<Primitive>(std::move(longCube));
    longCubePrim->material = matteWhite;
    longCubePrim->name = QString("Long Cube");

    // Short cube
    sPtr<Shape> shortCube = mkS<Cube>();
    shortCube->transform = Transform(Vector3f(-2, -1, 0.75), Vector3f(0, -17.5, 0), Vector3f(3, 3, 3));
    auto shortCubePrim = mkU<Primitive>(std::move(shortCube));
    shortCubePrim->material = matteWhite;
    shortCubePrim->name = QString("Short Cube");

    // Light source, which is a diffuse area light with a large plane as its shape
    sPtr<Shape> lightSquare = mkS<SquarePlane>();
    lightSquare->transform = Transform(Vector3f(0,7.45f,0), Vector3f(90,0,0), Vector3f(3, 3, 1));
//    auto lightSource = mkS<DiffuseAreaLight>(lightSquare->transform, Color3f(17,12,4) * 2.f, lightSquare);
    auto lightSource = mkS<DiffuseAreaLight>(lightSquare->transform, Color3f(40,40,40), lightSquare);
    auto lightPrim = mkU<Primitive>(std::move(lightSquare), nullptr, lightSource);
    lightPrim->name = QString("Light Source");

    primitives.push_back(std::move(floorPrim));
    primitives.push_back(std::move(lightPrim));
    primitives.push_back(std::move(leftWallPrim));
    primitives.push_back(std::move(rightWallPrim));
    primitives.push_back(std::move(backWallPrim));
    primitives.push_back(std::move(ceilingPrim));
    primitives.push_back(std::move(longCubePrim));
    primitives.push_back(std::move(shortCubePrim));

    lights.push_back(lightSource);

    for(uPtr<Primitive> &p : primitives)
    {
        p->shape->create();
    }

    camera = Camera(400, 400, Point3f(0, 5.5, -30), Point3f(0,2.5,0), Vector3f(0,1,0));
    camera.near_clip = 0.1f;
    camera.far_clip = 100.0f;
    camera.fovy = 19.5f;
    camera.create();
    camera.RecomputeAttributes();
    film = Film(400, 400);
}

void Scene::Clear()
{
    // These lists contain shared_ptrs
    // so the pointers will be freed
    // if appropriate when we clear the lists.
    primitives.clear();
    lights.clear();
    materials.clear();
    camera = Camera();
    film = Film();
}
