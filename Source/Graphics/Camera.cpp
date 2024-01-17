#include "Camera.h"

#include <algorithm>

#include "../Core/Framework.h"
#include "../Input/Gamepad.h"
#include "../Others/MathHelper.h"

//	コンストラクタ
Camera::Camera()
{
}

//	デストラクタ
Camera::~Camera()
{
}

//	初期化
void Camera::Initialize()
{
	eye_ = { 0.0f, 25.0f, 100.0f };	//	カメラの視点
	focus_ = { 0,0,0 };				//	カメラの注視点
	up_ = { 0,1,0 };				//	カメラの上方向
	angle_ = { 0,0,0 };				//	カメラの回転値
	fov_ = 30.0f;					//	視野角

}

//	パースペクティブ設定
void Camera::SetPerspectiveFov()
{
    //	画面アスペクト比
    float aspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    //	プロジェクション行列
    projectionMatrix_ = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov_), aspectRatio, 0.1f, 1000.0f);

	DirectX::XMVECTOR eye, focus, up;
	eye = DirectX::XMVectorSet(eye_.x, eye_.y, eye_.z, 1.0f);
	focus = DirectX::XMVectorSet(focus_.x,focus_.y,focus_.z, 1.0f);//?
	up = DirectX::XMVectorSet(up_.x, up_.y, up_.z, 0.0f);
	//	カメラの視点(ビュー行列)
	viewMatrix_ = DirectX::XMMatrixLookAtLH(eye, focus, up);

}

//	指定方向を向く
void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	//	視点、注視点、上方向からビュー行列を作成
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	viewMatrix_ = View;

	//	ビューを逆行列化し、ワールド行列に戻す
	//	XMMatrixInverse関数で逆行列を求める
	DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, World);

	//	カメラの方向を取り出す
	this->right_.x = world.m[0][0];
	this->right_.y = world.m[0][1];
	this->right_.z = world.m[0][2];

	this->up_.x = world.m[1][0];
	this->up_.y = world.m[1][1];
	this->up_.z = world.m[1][2];

	this->front_.x = world.m[2][0];
	this->front_.y = world.m[2][1];
	this->front_.z = world.m[2][2];

	//	視点、注視点を保存
	this->eye_ = eye;
	this->focus_ = focus;

}

//	更新処理
void Camera::Update(float elapsedTime)
{
	GamePad gamePad;
	gamePad.Acquire();
	float RX = gamePad.ThumbStateRx();
	float RY = gamePad.ThumbStateRy();
	float LY = gamePad.ThumbStateLy();

	//	カメラの回転速度
	float aSpeed = rollSpeed_ * elapsedTime;
	float moveSpeed = moveSpeed_ * elapsedTime;

	//	カメラ移動(X軸、Y軸)
	//	左のShiftキーと右スティック(IJKLキー)いずれかを押しているとき、
	//	カメラの視点をスティックの入力値に合わせてX,Y軸方向に移動
	if (gamePad.TriggerStateL() && (RX != 0 || RY != 0))
	{
		eyeOffset_.x -= RX * moveSpeed;
		eyeOffset_.y += RY * moveSpeed;
	}
	else	//	カメラ回転　右スティックの入力値に合わせてX軸とY軸を回転(注視点を回転させる)
	{
		angle_.x += RY * aSpeed;
		angle_.y += RX * aSpeed;
		eyeOffset_.x = 0;	//	X,Y軸のカメラ移動値をリセット
		eyeOffset_.y = 0;
	}

	//	カメラ移動(Z軸)　左スティック(W,Aキー)の入力値に合わせてZ軸方向に移動
	//	入力がなかったらリセット
	if (gamePad.ThumbStateLy() != 0)//	左スティック
	{
		eyeOffset_.z -= LY * moveSpeed;
	}
	else	eyeOffset_.z = 0;

#if 1 //	これを外すと向いてる方向に行かなくなる
	//////////////////////////////////////////////////////////////////////////////////////////////////
	DirectX::XMFLOAT3 cameraRight = this->GetRight();
	DirectX::XMFLOAT3 cameraFront = this->GetFront();
	DirectX::XMFLOAT3 cameraUp = this->GetUp();

	//	カメラ右方向ベクトルを単位ベクトルに変換
	float Rlength;
	DirectX::XMStoreFloat(&Rlength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraRight)));
	float cameraRightLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraRight)));
	if (cameraRightLength > 0.0f)
	{
		//	単位ベクトル化
		DirectX::XMVECTOR cameraRightVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraRight));
		DirectX::XMStoreFloat3(&cameraRight, cameraRightVec);
	}

	//	カメラ前方向ベクトルを単位ベクトルに変換
	float Zlength;
	DirectX::XMStoreFloat(&Zlength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraFront)));
	float cameraFrontLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraFront)));
	if (cameraFrontLength > 0.0f)
	{
		//	単位ベクトル化
		DirectX::XMVECTOR cameraFrontVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraFront));
		DirectX::XMStoreFloat3(&cameraFront, cameraFrontVec);
	}

	//	カメラ上方向ベクトルを単位ベクトルに変換
	float Ulength;
	DirectX::XMStoreFloat(&Ulength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraUp)));
	float cameraUpLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraUp)));
	if (cameraUpLength > 0.0f)
	{
		//	単位ベクトル化
		DirectX::XMVECTOR cameraUpVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraUp));
		DirectX::XMStoreFloat3(&cameraUp, cameraUpVec);
	}

	//	スティックの水平入力値をカメラ右方向に反映し、
	//	スティック垂直入力値をカメラ前方向に反映し、
	//	進行ベクトルを計算する
	eyeOffset_.x = cameraFront.x * LY * moveSpeed_ + cameraRight.x * RX * moveSpeed_ + cameraUp.x * RY * moveSpeed_;
	eyeOffset_.y = cameraFront.y * LY * moveSpeed_ + cameraRight.y * RX * moveSpeed_ + cameraUp.y * RY * moveSpeed_;
	eyeOffset_.z = cameraFront.z * LY * moveSpeed_ + cameraRight.z * RX * moveSpeed_ + cameraUp.z * RY * moveSpeed_;
	////////////////////////////////////////////////////////////////////////////////////////////
#endif
	
	//	カメラ移動(X軸、Y軸)
	//	左のShiftキーと右スティック(IJKLキー)いずれかを押しているとき、
	//	カメラの視点をスティックの入力値に合わせてX,Y軸方向に移動
	if (gamePad.TriggerStateL() && (RX != 0 || RY != 0))
	{
		eyeOffset_.x -= RX * moveSpeed;
		eyeOffset_.y += RY * moveSpeed;
	}
	else	//	カメラ回転　右スティックの入力値に合わせてX軸とY軸を回転(注視点を回転させる)
	{
		angle_.x += RY * aSpeed;
		angle_.y += RX * aSpeed;
		eyeOffset_.x = 0;	//	X,Y軸のカメラ移動値をリセット
		eyeOffset_.y = 0;
	}

	//	カメラ移動(Z軸)　左スティック(W,Aキー)の入力値に合わせてZ軸方向に移動
	//	入力がなかったらリセット
	if (gamePad.ThumbStateLy() != 0)//	左スティック
	{
		eyeOffset_.z -= LY * moveSpeed;
	}
	else	eyeOffset_.z = 0;


	//	カメラ回転値を回転行列に変換
	DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle_.x, angle_.y, angle_.z);
	
	//	回転行列から前方向ベクトルを取り出す
	//	Transform.r[2]で行列の３行目のデータを取り出している
	DirectX::XMVECTOR Front = Transform.r[2];
	DirectX::XMFLOAT3 front;
	DirectX::XMStoreFloat3(&front, Front);
	
	focus_.x = eye_.x - (front.x * range_);
	focus_.y = eye_.y - (front.y * range_);
	focus_.z = eye_.z - (front.z * range_);
	
	eye_ = eye_ + eyeOffset_;	//	カメラ視点の更新

	//	カメラの視点と注視点を設定
	Camera::Instance().SetLookAt(eye_, focus_, DirectX::XMFLOAT3(0, 1, 0));
}

//	リセット
void Camera::Reset()
{
	eye_ = { 0.0f, 25.0f, 100.0f };	//	カメラの視点
	focus_ = { 0,0,0 };				//	カメラの注視点
	up_ = { 0,1,0 };				//	カメラの上方向
	angle_ = { 0,0,0 };				//	カメラの回転値
	fov_ = 30.0f;					//	視野角
	eyeOffset_ = {};				//	カメラの視点eye_を動かすときの移動値
}

//	デバッグ描画
void Camera::DrawDebug()
{
    if (ImGui::TreeNode("Camera"))
    {
        GetTransform()->DrawDebug();
		ImGui::DragFloat3("Eye", &eye_.x, 0.01f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Focus", &focus_.x, 0.01f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Up", &up_.x, 0.01f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Angle", &angle_.x, 0.01f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Velocity", &eyeOffset_.x, 0.01f, -FLT_MAX, FLT_MAX);
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
        ImGui::TreePop();
    }
}
