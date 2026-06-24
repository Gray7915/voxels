#pragma once
#include "CollisionVolume.h"

namespace lve
{
    class SphereVolume : CollisionVolume
    {
        SphereVolume(float sphereRadius = 1.0f){
            type = VolumeType::Sphere;
            radius = sphereRadius;
        }

        ~ SphereVolume () {}

        float GetRadius() const{
            return radius;
        }

        protected:
            float radius;
    };
}
