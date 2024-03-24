#include "ECS/entity_manager.hpp"
#include <cassert>

namespace lve {
	EntityManager::EntityManager() {
		for (Entity entity=0; entity<MAX_ENTITIES; entity++) {
			mAvailableEntities.push(entity);
		}
	}

	Entity EntityManager::CreateEntity() {
		assert(mLivingEntityCount < MAX_ENTITIES && "Too man entities in existence");

		Entity id = mAvailableEntities.front(); // Take the ID at the front of the queue
		mAvailableEntities.pop();
		mLivingEntityCount++;

		return id;
	}

	void EntityManager::DestroyEntity(Entity entity) {
		assert(entity < MAX_ENTITIES && "Entity out of range");

		mSignatures[entity].reset(); // Invalidate signature of destroyed entity

		mAvailableEntities.push(entity); // Put the destroyed entity at the back of the queue
		mLivingEntityCount--;
	}

	void EntityManager::SetSignature(Entity entity, Signature signature) {
		assert(entity < MAX_ENTITIES && "Entity out of range");

		mSignatures[entity] = signature; // Set the entity's signature
	}
	Signature EntityManager::GetSignature(Entity entity) {
		assert(entity < MAX_ENTITIES && "Entity out of range");

		return mSignatures[entity]; // Get 
	}
}