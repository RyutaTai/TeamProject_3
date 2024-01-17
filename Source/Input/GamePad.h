#pragma once

#include <Windows.h>
#include <xinput.h>
#pragma comment(lib, "xinput9_1_0.lib")

#include <memory>
#include <windows.h>
#include <cstdint>

enum class TriggerMode
{
	NONE,			//	�{�^����������Ă����
	RISING_EDGE,	//	�{�^���������ꂽ�u��
	FALLING_EDGE	//	�{�^���������ꂽ�u��
};

class KeyButton
{
	int vkey_;
	int currentState_;	//	���݂̃X�e�[�g
	int previousState_;	//	�O�̃X�e�[�g
public:
	KeyButton(int vkey) : vkey_(vkey), currentState_(0), previousState_(0)
	{

	}
	virtual ~KeyButton() = default;
	KeyButton(KeyButton&) = delete;
	KeyButton& operator=(KeyButton&) = delete;

	// Signal edge
	// In electronics, a signal edge is a transition in a digital signal either from low to high(0 to 1) or from high to low(1 to 0).It is called an edge because the square wave which represents a signal has edges at those points.
	// A rising edge is the transition from low to high.[1] It is also named positive edge.When a circuit is rising edge - triggered, it becomes active when its clock signal goes from low to high, and ignores the high - to - low transition.
	// A falling edge is the high to low transition.[1] It is also known as the negative edge.When a circuit is falling edge - triggered, it becomes active when the clock signal goes from high to low, and ignores the low - to - high transition.
	// A leading edge is an event that is triggered on the front edge of a pulse.Assuming a clock begins at t = 0, the first position would be triggered at t = 1.
	// A trailing edge is the opposite of a leading edge.It is triggered on the back edge of a pulse.Assuming the clock begins at t = 0, the first position would be triggered at t = 0.
	// The terms front edge or leading edge, and back edge or trailing edge describe the related position of edges in a clock cycle.
	bool State(TriggerMode triggerMode)
	{
		previousState_ = currentState_;
		if (static_cast<USHORT>(GetAsyncKeyState(vkey_)) & 0x8000)	//	0x8000�l�ɂ́A�L�[�����݉�����Ă��邩�ǂ������e�X�g����r�b�g�t���O���܂܂�Ă���
		{
			currentState_++;
		}
		else
		{
			currentState_ = 0;
		}
		if (triggerMode == TriggerMode::RISING_EDGE)		//	�O�ɉ���������Ă��炸�A���݉���������Ă��� �� �{�^���������ꂽ�u��
		{
			return previousState_ == 0 && currentState_ > 0;
		}
		else if (triggerMode == TriggerMode::FALLING_EDGE)	//	�O�ɉ���������Ă��āA���݉���������Ă��Ȃ� �� �{�^���������ꂽ�u��
		{
			return previousState_ > 0 && currentState_ == 0;
		}
		else												//	�{�^����������Ă����
		{
			return currentState_ > 0;
		}
	}
};

class GamePad
{
private:
	enum class Side { L, R };	//	L,R(���E)
	enum class Axis { X, Y };	//	x��,y��
	enum class DeadzoneMode { INDEPENDENT_AXES, CIRCULAR, NONE };
public:
	enum class Button { A, B, X, Y, LEFT_THUMB, RIGHT_THUMB, LEFT_SHOULDER, RIGHT_SHOULDER, START, BACK, UP, DOWN, LEFT, RIGHT, END };
private:
	static const size_t buttonCount_ = static_cast<size_t>(Button::END);

private:
	int userId_ = 0;
	struct State
	{
		DWORD packet_;
		bool connected_ = false;
		bool buttons_[buttonCount_];
		float triggers_[2];			//	[side]
		float thumbSticks_[2][2];	//	[side][axis]
	};
	State currentState_;	//	���݂̃X�e�[�g
	State previousState_;	//	�O�̃X�e�[�g

	// use on emulation_mode
	bool emulationMode_ = false; //if gamepad is disconnected then keyboard is used but only user_id = 0
	std::unique_ptr<KeyButton> buttonKeys_[buttonCount_];
	std::unique_ptr<KeyButton> thumbStickKeys_[2][4]; //[side][+x/-x/+y/-y]
	std::unique_ptr<KeyButton> triggerKeys_[2]; //[side]

public:
	GamePad(int userId = 0, float deadzoneX = 0.05f, float deadzoneY = 0.02f, DeadzoneMode deadzoneMode = DeadzoneMode::INDEPENDENT_AXES);
	virtual ~GamePad() = default;
	GamePad(GamePad const&) = delete;
	GamePad& operator=(GamePad const&) = delete;


	bool Acquire();

	bool ButtonState(Button button, TriggerMode triggerMode = TriggerMode::RISING_EDGE) const;
	
	float ThumbStateLx() const { return ThumbState(Side::L, Axis::X); }	//	���X�e�B�b�NX�����͏�Ԃ̎擾
	float ThumbStateLy() const { return ThumbState(Side::L, Axis::Y); }	//	���X�e�B�b�Ny�����͏�Ԃ̎擾
	float ThumbStateRx() const { return ThumbState(Side::R, Axis::X); }	//	�E�X�e�B�b�NX�����͏�Ԃ̎擾
	float ThumbStateRy() const { return ThumbState(Side::R, Axis::Y); }	//	�E�X�e�B�b�NY�����͏�Ԃ̎擾

	float TriggerStateL()const { return TriggerState(Side::L); }		//	���g���K�[���͏�Ԃ̎擾
	float TriggerStateR()const { return TriggerState(Side::R); }		//	�E�g���K�[���͏�Ԃ̎擾

private:
	float ThumbState(Side side, Axis axis) const;						
	float TriggerState(Side side)const;

	float deadzoneX_ = 0.05f;
	float deadzoneY_ = 0.02f;
	DeadzoneMode deadzoneMode_ = DeadzoneMode::NONE;
	void ApplyStickDeadzone(float x, float y, DeadzoneMode deadzoneMode, float maxValue, float deadzoneSize, _Out_ float& resultX, _Out_ float& resultY);
};

