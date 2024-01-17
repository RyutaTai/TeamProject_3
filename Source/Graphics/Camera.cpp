#include "Camera.h"

#include <algorithm>

#include "../Core/Framework.h"
#include "../Input/Gamepad.h"
#include "../Others/MathHelper.h"

//	�R���X�g���N�^
Camera::Camera()
{
}

//	�f�X�g���N�^
Camera::~Camera()
{
}

//	������
void Camera::Initialize()
{
	eye_ = { 0.0f, 25.0f, 100.0f };	//	�J�����̎��_
	focus_ = { 0,0,0 };				//	�J�����̒����_
	up_ = { 0,1,0 };				//	�J�����̏����
	angle_ = { 0,0,0 };				//	�J�����̉�]�l
	fov_ = 30.0f;					//	����p

}

//	�p�[�X�y�N�e�B�u�ݒ�
void Camera::SetPerspectiveFov()
{
    //	��ʃA�X�y�N�g��
    float aspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    //	�v���W�F�N�V�����s��
    projectionMatrix_ = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov_), aspectRatio, 0.1f, 1000.0f);

	DirectX::XMVECTOR eye, focus, up;
	eye = DirectX::XMVectorSet(eye_.x, eye_.y, eye_.z, 1.0f);
	focus = DirectX::XMVectorSet(focus_.x,focus_.y,focus_.z, 1.0f);//?
	up = DirectX::XMVectorSet(up_.x, up_.y, up_.z, 0.0f);
	//	�J�����̎��_(�r���[�s��)
	viewMatrix_ = DirectX::XMMatrixLookAtLH(eye, focus, up);

}

//	�w�����������
void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	//	���_�A�����_�A���������r���[�s����쐬
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	viewMatrix_ = View;

	//	�r���[���t�s�񉻂��A���[���h�s��ɖ߂�
	//	XMMatrixInverse�֐��ŋt�s������߂�
	DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, World);

	//	�J�����̕��������o��
	this->right_.x = world.m[0][0];
	this->right_.y = world.m[0][1];
	this->right_.z = world.m[0][2];

	this->up_.x = world.m[1][0];
	this->up_.y = world.m[1][1];
	this->up_.z = world.m[1][2];

	this->front_.x = world.m[2][0];
	this->front_.y = world.m[2][1];
	this->front_.z = world.m[2][2];

	//	���_�A�����_��ۑ�
	this->eye_ = eye;
	this->focus_ = focus;

}

//	�X�V����
void Camera::Update(float elapsedTime)
{
	GamePad gamePad;
	gamePad.Acquire();
	float RX = gamePad.ThumbStateRx();
	float RY = gamePad.ThumbStateRy();
	float LY = gamePad.ThumbStateLy();

	//	�J�����̉�]���x
	float aSpeed = rollSpeed_ * elapsedTime;
	float moveSpeed = moveSpeed_ * elapsedTime;

	//	�J�����ړ�(X���AY��)
	//	����Shift�L�[�ƉE�X�e�B�b�N(IJKL�L�[)�����ꂩ�������Ă���Ƃ��A
	//	�J�����̎��_���X�e�B�b�N�̓��͒l�ɍ��킹��X,Y�������Ɉړ�
	if (gamePad.TriggerStateL() && (RX != 0 || RY != 0))
	{
		eyeOffset_.x -= RX * moveSpeed;
		eyeOffset_.y += RY * moveSpeed;
	}
	else	//	�J������]�@�E�X�e�B�b�N�̓��͒l�ɍ��킹��X����Y������](�����_����]������)
	{
		angle_.x += RY * aSpeed;
		angle_.y += RX * aSpeed;
		eyeOffset_.x = 0;	//	X,Y���̃J�����ړ��l�����Z�b�g
		eyeOffset_.y = 0;
	}

	//	�J�����ړ�(Z��)�@���X�e�B�b�N(W,A�L�[)�̓��͒l�ɍ��킹��Z�������Ɉړ�
	//	���͂��Ȃ������烊�Z�b�g
	if (gamePad.ThumbStateLy() != 0)//	���X�e�B�b�N
	{
		eyeOffset_.z -= LY * moveSpeed;
	}
	else	eyeOffset_.z = 0;

#if 1 //	������O���ƌ����Ă�����ɍs���Ȃ��Ȃ�
	//////////////////////////////////////////////////////////////////////////////////////////////////
	DirectX::XMFLOAT3 cameraRight = this->GetRight();
	DirectX::XMFLOAT3 cameraFront = this->GetFront();
	DirectX::XMFLOAT3 cameraUp = this->GetUp();

	//	�J�����E�����x�N�g����P�ʃx�N�g���ɕϊ�
	float Rlength;
	DirectX::XMStoreFloat(&Rlength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraRight)));
	float cameraRightLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraRight)));
	if (cameraRightLength > 0.0f)
	{
		//	�P�ʃx�N�g����
		DirectX::XMVECTOR cameraRightVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraRight));
		DirectX::XMStoreFloat3(&cameraRight, cameraRightVec);
	}

	//	�J�����O�����x�N�g����P�ʃx�N�g���ɕϊ�
	float Zlength;
	DirectX::XMStoreFloat(&Zlength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraFront)));
	float cameraFrontLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraFront)));
	if (cameraFrontLength > 0.0f)
	{
		//	�P�ʃx�N�g����
		DirectX::XMVECTOR cameraFrontVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraFront));
		DirectX::XMStoreFloat3(&cameraFront, cameraFrontVec);
	}

	//	�J����������x�N�g����P�ʃx�N�g���ɕϊ�
	float Ulength;
	DirectX::XMStoreFloat(&Ulength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraUp)));
	float cameraUpLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraUp)));
	if (cameraUpLength > 0.0f)
	{
		//	�P�ʃx�N�g����
		DirectX::XMVECTOR cameraUpVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraUp));
		DirectX::XMStoreFloat3(&cameraUp, cameraUpVec);
	}

	//	�X�e�B�b�N�̐������͒l���J�����E�����ɔ��f���A
	//	�X�e�B�b�N�������͒l���J�����O�����ɔ��f���A
	//	�i�s�x�N�g�����v�Z����
	eyeOffset_.x = cameraFront.x * LY * moveSpeed_ + cameraRight.x * RX * moveSpeed_ + cameraUp.x * RY * moveSpeed_;
	eyeOffset_.y = cameraFront.y * LY * moveSpeed_ + cameraRight.y * RX * moveSpeed_ + cameraUp.y * RY * moveSpeed_;
	eyeOffset_.z = cameraFront.z * LY * moveSpeed_ + cameraRight.z * RX * moveSpeed_ + cameraUp.z * RY * moveSpeed_;
	////////////////////////////////////////////////////////////////////////////////////////////
#endif
	
	//	�J�����ړ�(X���AY��)
	//	����Shift�L�[�ƉE�X�e�B�b�N(IJKL�L�[)�����ꂩ�������Ă���Ƃ��A
	//	�J�����̎��_���X�e�B�b�N�̓��͒l�ɍ��킹��X,Y�������Ɉړ�
	if (gamePad.TriggerStateL() && (RX != 0 || RY != 0))
	{
		eyeOffset_.x -= RX * moveSpeed;
		eyeOffset_.y += RY * moveSpeed;
	}
	else	//	�J������]�@�E�X�e�B�b�N�̓��͒l�ɍ��킹��X����Y������](�����_����]������)
	{
		angle_.x += RY * aSpeed;
		angle_.y += RX * aSpeed;
		eyeOffset_.x = 0;	//	X,Y���̃J�����ړ��l�����Z�b�g
		eyeOffset_.y = 0;
	}

	//	�J�����ړ�(Z��)�@���X�e�B�b�N(W,A�L�[)�̓��͒l�ɍ��킹��Z�������Ɉړ�
	//	���͂��Ȃ������烊�Z�b�g
	if (gamePad.ThumbStateLy() != 0)//	���X�e�B�b�N
	{
		eyeOffset_.z -= LY * moveSpeed;
	}
	else	eyeOffset_.z = 0;


	//	�J������]�l����]�s��ɕϊ�
	DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle_.x, angle_.y, angle_.z);
	
	//	��]�s�񂩂�O�����x�N�g�������o��
	//	Transform.r[2]�ōs��̂R�s�ڂ̃f�[�^�����o���Ă���
	DirectX::XMVECTOR Front = Transform.r[2];
	DirectX::XMFLOAT3 front;
	DirectX::XMStoreFloat3(&front, Front);
	
	focus_.x = eye_.x - (front.x * range_);
	focus_.y = eye_.y - (front.y * range_);
	focus_.z = eye_.z - (front.z * range_);
	
	eye_ = eye_ + eyeOffset_;	//	�J�������_�̍X�V

	//	�J�����̎��_�ƒ����_��ݒ�
	Camera::Instance().SetLookAt(eye_, focus_, DirectX::XMFLOAT3(0, 1, 0));
}

//	���Z�b�g
void Camera::Reset()
{
	eye_ = { 0.0f, 25.0f, 100.0f };	//	�J�����̎��_
	focus_ = { 0,0,0 };				//	�J�����̒����_
	up_ = { 0,1,0 };				//	�J�����̏����
	angle_ = { 0,0,0 };				//	�J�����̉�]�l
	fov_ = 30.0f;					//	����p
	eyeOffset_ = {};				//	�J�����̎��_eye_�𓮂����Ƃ��̈ړ��l
}

//	�f�o�b�O�`��
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
