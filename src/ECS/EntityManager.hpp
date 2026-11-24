#pragma once
#include "Type.hpp"
#include <assert.h>
#include <array>
#include <queue>

namespace lve
{
    class EntityManager
    {
    public:
        EntityManager()
        {
            for (Entity entity = 0; entity < MAX_ENTITIES; entity++)
            {
                mAvailableEntities.push(entity);
            }
        }

        Entity CreateEntity()
        {
            assert(mLivingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

            Entity id = mAvailableEntities.front();
            mAvailableEntities.pop();
            ++mLivingEntityCount;
            return id;
        }

        void DestroyEntity(Entity entity)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            mSignatures[entity].reset();

            mAvailableEntities.push(entity);
            --mLivingEntityCount;
        }

        void SetSignature(Entity entity, Signature signature)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            // Put this entity's signature into the array
            mSignatures[entity] = signature;
        }

        Signature GetSignature(Entity entity)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            // Get this entity's signature from the array
            return mSignatures[entity];
        }

    private:
        std::queue<Entity> mAvailableEntities{};

        std::array<Signature, MAX_ENTITIES> mSignatures{};

        uint32_t mLivingEntityCount{};
    };
}
