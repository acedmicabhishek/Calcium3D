#include "Scene.h"

Scene::Scene() {
    
}

Scene::~Scene() {
    Clear();
}

void Scene::Update(float dt) {
    physicsEngine.Update(dt, m_Objects);
}

void Scene::AddObject(GameObject object) {
    m_Objects.push_back(std::move(object));
}

void Scene::RemoveObject(int index) {
    if (index >= 0 && index < (int)m_Objects.size()) {
        m_Objects.erase(m_Objects.begin() + index);
    }
}

void Scene::Clear() {
    m_Objects.clear();
    m_PointLights.clear();
}

Scene::PointLight* Scene::CreatePointLight() {
    PointLight pl;
    
    pl.position = glm::vec3(0.0f, 5.0f, 0.0f);
    pl.color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
    pl.intensity = 5.0f;
    pl.enabled = true;
    pl.constant = 1.0f;
    pl.linear = 0.09f;
    pl.quadratic = 0.032f;
    
    m_PointLights.push_back(pl);
    return &m_PointLights.back();
}

void Scene::RemovePointLight(int index) {
    if (index >= 0 && index < (int)m_PointLights.size()) {
        m_PointLights.erase(m_PointLights.begin() + index);
    }
}
