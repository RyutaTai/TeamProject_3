#pragma once

#include <memory>

#include "../Resources/SkinnedMesh.h"
#include "../Others/Transform.h"

class GameObject
{
public:
	GameObject(const char* fbxFilename, bool triangulate = false);
	GameObject() {}
	~GameObject() {}

	void Render(const float& scale);

	Transform* GetTransform() { return &transform_; }

private:
	std::unique_ptr<SkinnedMesh>	model_;
	Transform						transform_;
	Animation::Keyframe				keyframe_ = {};

};

