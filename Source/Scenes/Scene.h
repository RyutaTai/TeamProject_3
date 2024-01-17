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

	void SetReady() { ready = true; }		//	€”õŠ®—¹İ’è
	bool IsReady()const { return ready; }	//	€”õŠ®—¹‚µ‚Ä‚¢‚é‚©

private:
	bool ready = false;

};

