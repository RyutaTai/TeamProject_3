#pragma once

#include "../Others/Transform.h"

class Camera
{
private:
	Camera();
	~Camera();

public:
	static Camera& Instance()
	{
		static Camera instance_;
		return instance_;
	}

	void Initialize();
	void Update(float elapsedTime);	//	更新処理

	//	デバッグ用
	void DrawDebug();	//	デバッグ描画
	void Reset();		//	リセット

	void SetPerspectiveFov();		//	パースペクティブ設定
	void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up);	//	指定方向を向く
	void SetEye(const DirectX::XMFLOAT3& eye)		{ this->eye_ = eye; }		//	視点設定
	void SetTarget(const DirectX::XMFLOAT3& target) { this->focus_ = target; }	//	ターゲット位置設定

	Transform* GetTransform() { return &transform_; }
	const DirectX::XMMATRIX& GetViewMatrix()		const{ return viewMatrix_; }		//	ビュー行列取得
	const DirectX::XMMATRIX& GetProjectionMatrix()	const{ return projectionMatrix_; }	//	プロジェクション行列取得

	const DirectX::XMFLOAT3& GetEye()	const { return eye_; }		//	視点取得
	const DirectX::XMFLOAT3& GetFocus() const { return focus_; }	//	注視点取得
	const DirectX::XMFLOAT3& GetUp()	const { return up_; }		//	カメラ上方向取得
	const DirectX::XMFLOAT3& GetFront() const { return front_; }	//	カメラ前方向取得
	const DirectX::XMFLOAT3& GetRight() const { return right_; }	//	カメラ右方向取得
	
private:
	Transform transform_;

	DirectX::XMMATRIX viewMatrix_;							//	ビュー行列
	DirectX::XMMATRIX projectionMatrix_;					//	プロジェクション行列
	DirectX::XMFLOAT3 eye_;									//	カメラの視点
	DirectX::XMFLOAT3 focus_ = { 0,-10,10 };				//	カメラの注視点
	DirectX::XMFLOAT3 angle_ = { 0,0,0 };					//	カメラの回転値
	DirectX::XMFLOAT3 eyeOffset_ = { 0,0,0 };				//	カメラの視点eye_を動かすときの移動値

	float fov_;												//	視野角
	float range_ = 10.0f;									//	カメラとターゲットの距離を決めるのに使う
	float rollSpeed_ = DirectX::XMConvertToRadians(45);		//	カメラの回転速度
	float maxAngleX_ = DirectX::XMConvertToRadians(45);		//	カメラの回転値制限
	float minAngleX_ = DirectX::XMConvertToRadians(-45);	//	カメラの回転値制限
	float moveSpeed_ = 0.1f;								//	カメラの移動速度
	float maxEyeOffset_  = 2.0f;
	float minEyeOffset_  = -2.0f;


	DirectX::XMFLOAT3 up_;									//	カメラの上方向
	DirectX::XMFLOAT3 front_;								//	カメラの前方向
	DirectX::XMFLOAT3 right_;								//	カメラの右方向

};

