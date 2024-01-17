#pragma once

#include "GameObject.h"

class Stage : public GameObject
{
public:
	Stage(const char* filename, bool triangulate = false);
	Stage(){}
	~Stage();

	//	�B��̃C���X�^���X�擾
	static Stage& Instance()
	{
		static Stage instance_;
		return instance_;
	}

	void Render();
	void DrawDebug();

};