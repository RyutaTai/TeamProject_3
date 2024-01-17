#pragma once

class Scene
{
public:
	Scene(){}
	virtual ~Scene(){}

	virtual void Initialize() = 0;
	virtual void Finalize() = 0;
	virtual void Update(const float& elapsedTime) = 0;
	virtual void Render() = 0;
	virtual void DrawDebug() = 0;

	void SetReady() { ready = true; }		//	���������ݒ�
	bool IsReady()const { return ready; }	//	�����������Ă��邩

private:
	bool ready = false;

};

