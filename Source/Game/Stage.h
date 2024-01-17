#pragma once

#include "GameObject.h"

class Stage : public GameObject
{
public:
	Stage(const char* filename, bool triangulate = false);
	Stage(){}
	~Stage();

	//	唯一のインスタンス取得
	static Stage& Instance()
	{
		static Stage instance_;
		return instance_;
	}

	void Render();
	void DrawDebug();

};