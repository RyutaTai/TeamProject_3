#include "Stage.h"

#include "../Graphics/Graphics.h"

//	コンストラクタ
Stage::Stage(const char* filename, bool triangulate)
	: GameObject(filename, triangulate)
{
	GetTransform()->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	GetTransform()->SetScale(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
	GetTransform()->SetRotation(DirectX::XMFLOAT4(0.0f, DirectX::XMConvertToRadians(0), 0.0f, 0.0f));

}

//デストラクタ
Stage::~Stage()
{

}

//	描画処理
void Stage::Render()
{
	GameObject::Render(1.0f);
}

//	デバッグ描画
void Stage::DrawDebug()
{
	GetTransform()->DrawDebug();
}
