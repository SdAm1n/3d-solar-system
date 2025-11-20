#ifndef PLANET_H
#define PLANET_H

#include <glm/glm.hpp>
#include <string>
#include <cmath>
#include <vector>
#include <memory>

class Planet
{
public:
    std::string name;
    float size;
    float orbitRadius;
    float rotationSpeed;
    float orbitSpeed;
    glm::vec3 color;
    glm::vec3 position;
    bool isSun;
    float orbitAngle;
    int planetType; // 0=Sun, 1=Mercury, 2=Venus, 3=Earth, 4=Mars, 5=Jupiter, 6=Saturn, 7=Uranus, 8=Neptune, 9=Moon
    
    // For moons
    std::vector<std::shared_ptr<Planet>> moons;
    Planet* parent; // Pointer to parent planet (for moons)

    Planet(const std::string& name, float size, float orbitRadius, 
           float rotationSpeed, float orbitSpeed, glm::vec3 color, bool isSun = false, int type = 0)
        : name(name), size(size), orbitRadius(orbitRadius), 
          rotationSpeed(rotationSpeed), orbitSpeed(orbitSpeed), 
          color(color), isSun(isSun), orbitAngle(0.0f), planetType(type), parent(nullptr)
    {
        updatePosition();
    }

    void addMoon(std::shared_ptr<Planet> moon)
    {
        moon->parent = this;
        moons.push_back(moon);
    }

    void update(float deltaTime)
    {
        if (!isSun)
        {
            // Update orbit angle
            orbitAngle += orbitSpeed * deltaTime * 0.1f;
            updatePosition();
            
            // Update moons
            for (auto& moon : moons)
            {
                moon->update(deltaTime);
            }
        }
        else
        {
            position = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }

    void updatePosition()
    {
        if (!isSun)
        {
            if (parent == nullptr)
            {
                // Regular planet orbiting the sun
                position.x = cos(orbitAngle) * orbitRadius;
                position.y = 0.0f;
                position.z = sin(orbitAngle) * orbitRadius;
            }
            else
            {
                // Moon orbiting a planet
                position.x = parent->position.x + cos(orbitAngle) * orbitRadius;
                position.y = parent->position.y;
                position.z = parent->position.z + sin(orbitAngle) * orbitRadius;
            }
        }
    }
};

#endif
