#include "scene.h"

Scene::Scene()
{
    for(f32 x = -1.0f; x < 1.0f; x += 0.2f)
    {
        for(f32 y = -1.0f; y < 1.0f; y += 0.2f)
        {
            triPos.push_back(glm::vec3(x, y, 0.0f));
        }
    }
}

Scene::~Scene() {}
