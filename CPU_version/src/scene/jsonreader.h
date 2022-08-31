#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QList>
#include <raytracing/film.h>
#include <scene/scene.h>
#include <scene/camera.h>
#include <scene/transform.h>

class JSONReader
{
public:
    void LoadSceneFromFile(QFile &file, const QString &local_path, Scene &scene);
    bool LoadGeometry(QJsonObject &geometry, QMap<QString, std::shared_ptr<Material>> mtl_map, const QString &local_path, std::vector<std::unique_ptr<Primitive> > *primitives);
    bool LoadLights(QJsonObject &geometry, QMap<QString, std::shared_ptr<Material>> mtl_map, const QString &local_path, std::vector<std::unique_ptr<Primitive> > *primitives, std::vector<std::shared_ptr<Light>> *lights);
    bool LoadMaterial(QJsonObject &material, QString local_path, QMap<QString, std::shared_ptr<Material> > *mtl_map);
    Camera LoadCamera(QJsonObject &camera);
    Transform LoadTransform(QJsonObject &transform);
    glm::vec3 ToVec3(const QJsonArray &s);
    glm::vec3 ToVec3(const QString &s);

    Scene* mScene;
};
