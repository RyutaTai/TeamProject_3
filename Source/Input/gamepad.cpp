#include "GamePad.h"

#include <math.h>
#include <algorithm>
#include <assert.h>

static float ApplyLinearDeadzone(float value, float maxValue, float deadzoneSize)
{
	if (value < -deadzoneSize)
	{
		// Increase negative values to remove the deadzone discontinuity.
		value += deadzoneSize;
	}
	else if (value > deadzoneSize)
	{
		// Decrease positive values to remove the deadzone discontinuity.
		value -= deadzoneSize;
	}
	else
	{
		// Values inside the deadzone come out zero.
		return 0;
	}

	// Scale into 0-1 range.
	float scaledValue = value / (maxValue - deadzoneSize);
	return std::max<float>(-1.f, std::min<float>(scaledValue, 1.f));
}

void GamePad::ApplyStickDeadzone(float x, float y, DeadzoneMode deadzoneMode, float maxValue, float deadzoneSize, _Out_ float& resultX, _Out_ float& resultY)
{
	switch (deadzoneMode)
	{
	case DeadzoneMode::INDEPENDENT_AXES:
		resultX = ApplyLinearDeadzone(x, maxValue, deadzoneSize);
		resultY = ApplyLinearDeadzone(y, maxValue, deadzoneSize);
		break;
	case DeadzoneMode::CIRCULAR:
	{
		float dist = sqrtf(x * x + y * y);
		float wanted = ApplyLinearDeadzone(dist, maxValue, deadzoneSize);

		float scale = (wanted > 0.f) ? (wanted / dist) : 0.f;

		resultX = std::max <float>(-1.f, std::min<float>(x * scale, 1.f));
		resultY = std::max<float>(-1.f, std::min<float>(y * scale, 1.f));
		break;
	}
	default: //DeadzoneMode::NONE
		resultX = ApplyLinearDeadzone(x, maxValue, 0);
		resultY = ApplyLinearDeadzone(y, maxValue, 0);
		break;
	}
}

GamePad::GamePad(int userId, float deadzoneX, float deadzoneY, DeadzoneMode deadzoneMode) 
	: userId_(userId), emulationMode_(userId == 0), deadzoneX_(deadzoneX), deadzoneY_(deadzoneY), deadzoneMode_(deadzoneMode)
{
	memset(&currentState_, 0, sizeof(State));
	memset(&previousState_, 0, sizeof(State));

	if (emulationMode_)
	{
		const int buttonKeymap[buttonCount_] = {
			'Z'/*A*/, 'X'/*B*/, 'C'/*X*/, 'V'/*Y*/,
			VK_HOME/*LEFT_THUMB*/, VK_END/*RIGHT_THUMB*/, VK_LCONTROL/*LEFT_SHOULDER*/, VK_RCONTROL/*RIGHT_SHOULDER*/,
			VK_SPACE/*START*/, VK_BACK/*BACK*/, VK_F1/*UP*/, VK_F2/*DOWN*/, VK_F3/*LEFT*/, VK_F4/*RIGHT*/};

		for (size_t buttonIndex = 0; buttonIndex < buttonCount_; ++buttonIndex)
		{
			buttonKeys_[buttonIndex] = std::make_unique<KeyButton>(buttonKeymap[buttonIndex]);
		}
#if 1
		thumbStickKeys_[static_cast<size_t>(Side::L)][0] = std::make_unique<KeyButton>('D');
		thumbStickKeys_[static_cast<size_t>(Side::L)][1] = std::make_unique<KeyButton>('A');
		thumbStickKeys_[static_cast<size_t>(Side::L)][2] = std::make_unique<KeyButton>('W');
		thumbStickKeys_[static_cast<size_t>(Side::L)][3] = std::make_unique<KeyButton>('S');
		thumbStickKeys_[static_cast<size_t>(Side::R)][0] = std::make_unique<KeyButton>('L');
		thumbStickKeys_[static_cast<size_t>(Side::R)][1] = std::make_unique<KeyButton>('J');
		thumbStickKeys_[static_cast<size_t>(Side::R)][2] = std::make_unique<KeyButton>('I');
		thumbStickKeys_[static_cast<size_t>(Side::R)][3] = std::make_unique<KeyButton>('K');
		//thumbStickKeys_[static_cast<size_t>(Side::R)][0] = std::make_unique<KeyButton>(VK_RIGHT);
		//thumbStickKeys_[static_cast<size_t>(Side::R)][1] = std::make_unique<KeyButton>(VK_LEFT);
		//thumbStickKeys_[static_cast<size_t>(Side::R)][2] = std::make_unique<KeyButton>(VK_UP);
		//thumbStickKeys_[static_cast<size_t>(Side::R)][3] = std::make_unique<KeyButton>(VK_DOWN);
#else
		thumbStickKeys_[static_cast<size_t>(Side::R)][0] = std::make_unique<KeyButton>('D');
		thumbStickKeys_[static_cast<size_t>(Side::R)][1] = std::make_unique<KeyButton>('A');
		thumbStickKeys_[static_cast<size_t>(Side::R)][2] = std::make_unique<KeyButton>('W');
		thumbStickKeys_[static_cast<size_t>(Side::R)][3] = std::make_unique<KeyButton>('S');
		thumbStickKeys_[static_cast<size_t>(Side::L)][0] = std::make_unique<KeyButton>(VK_RIGHT);
		thumbStickKeys_[static_cast<size_t>(Side::L)][1] = std::make_unique<KeyButton>(VK_LEFT);
		thumbStickKeys_[static_cast<size_t>(Side::L)][2] = std::make_unique<KeyButton>(VK_UP);
		thumbStickKeys_[static_cast<size_t>(Side::L)][3] = std::make_unique<KeyButton>(VK_DOWN);
#endif

		triggerKeys_[static_cast<size_t>(Side::L)] = std::make_unique<KeyButton>(VK_LSHIFT);
		triggerKeys_[static_cast<size_t>(Side::R)] = std::make_unique<KeyButton>(VK_RSHIFT);

	}
}

bool GamePad::Acquire()
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(static_cast<DWORD>(userId_), &state);
	if (result == ERROR_DEVICE_NOT_CONNECTED)
	{
		currentState_.connected_ = false;
	}
	else
	{
		previousState_ = currentState_;
		memset(&currentState_, 0, sizeof(currentState_));
		currentState_.connected_ = true;

		currentState_.packet_ = state.dwPacketNumber;

		WORD buttonStates = state.Gamepad.wButtons;
		currentState_.buttons_[static_cast<size_t>(Button::A)]				= (buttonStates & XINPUT_GAMEPAD_A) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::B)]				= (buttonStates & XINPUT_GAMEPAD_B) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::X)]				= (buttonStates & XINPUT_GAMEPAD_X) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::Y)]				= (buttonStates & XINPUT_GAMEPAD_Y) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::LEFT_THUMB)]		= (buttonStates & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::RIGHT_THUMB)]	= (buttonStates & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::LEFT_SHOULDER)]	= (buttonStates & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::RIGHT_SHOULDER)]	= (buttonStates & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::BACK)]			= (buttonStates & XINPUT_GAMEPAD_BACK) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::START)]			= (buttonStates & XINPUT_GAMEPAD_START) != 0;

		currentState_.buttons_[static_cast<size_t>(Button::UP)]				= (buttonStates & XINPUT_GAMEPAD_DPAD_UP) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::DOWN)]			= (buttonStates & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::RIGHT)]			= (buttonStates & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
		currentState_.buttons_[static_cast<size_t>(Button::LEFT)]			= (buttonStates & XINPUT_GAMEPAD_DPAD_LEFT) != 0;

		if (deadzoneMode_ == DeadzoneMode::NONE)
		{
			currentState_.triggers_[static_cast<size_t>(Side::L)] = ApplyLinearDeadzone(static_cast<float>(state.Gamepad.bLeftTrigger), 255.f, 0.f);
			currentState_.triggers_[static_cast<size_t>(Side::R)] = ApplyLinearDeadzone(static_cast<float>(state.Gamepad.bRightTrigger), 255.f, 0.f);
		}
		else
		{
			currentState_.triggers_[static_cast<size_t>(Side::L)] = ApplyLinearDeadzone(static_cast<float>(state.Gamepad.bLeftTrigger), 255.f, static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
			currentState_.triggers_[static_cast<size_t>(Side::R)] = ApplyLinearDeadzone(static_cast<float>(state.Gamepad.bRightTrigger), 255.f, static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
		}

		ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbLX), static_cast<float>(state.Gamepad.sThumbLY),
			deadzoneMode_, 32767.f, static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
			currentState_.thumbSticks_[static_cast<size_t>(Side::L)][static_cast<size_t>(Axis::X)], currentState_.thumbSticks_[static_cast<size_t>(Side::L)][static_cast<size_t>(Axis::Y)]);

		ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbRX), static_cast<float>(state.Gamepad.sThumbRY),
			deadzoneMode_, 32767.f, static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
			currentState_.thumbSticks_[static_cast<size_t>(Side::R)][static_cast<size_t>(Axis::X)], currentState_.thumbSticks_[static_cast<size_t>(Side::R)][static_cast<size_t>(Axis::Y)]);
	}
	return true;
}

bool GamePad::ButtonState(Button button, TriggerMode triggerMode) const
{
	bool buttonState = false;
	if (currentState_.connected_)
	{
		switch (triggerMode)
		{
		case TriggerMode::NONE:
			buttonState = currentState_.buttons_[static_cast<size_t>(button)];
			break;
		case TriggerMode::RISING_EDGE:
			buttonState = !previousState_.buttons_[static_cast<size_t>(button)] && currentState_.buttons_[static_cast<size_t>(button)];
			break;
		case TriggerMode::FALLING_EDGE:
			buttonState = previousState_.buttons_[static_cast<size_t>(button)] && !currentState_.buttons_[static_cast<size_t>(button)];
			break;
		}
	}
	if (emulationMode_ && !buttonState)
	{
		buttonState = buttonKeys_[static_cast<size_t>(button)]->State(triggerMode);
	}
	return buttonState;
}

float GamePad::ThumbState(Side side, Axis axis) const
{
	float stickState = 0.0f;
	if (currentState_.connected_)
	{
		stickState = currentState_.thumbSticks_[static_cast<size_t>(side)][static_cast<size_t>(axis)];
	}
	if (emulationMode_ && stickState == 0.0f)
	{
		stickState = axis == Axis::X ? static_cast<float>(std::min<size_t>(1, thumbStickKeys_[static_cast<size_t>(side)][0]->State(TriggerMode::NONE))) - (std::min<size_t>(1, thumbStickKeys_[static_cast<size_t>(side)][1]->State(TriggerMode::NONE))) :
			static_cast<float>(std::min<size_t>(1, thumbStickKeys_[static_cast<size_t>(side)][2]->State(TriggerMode::NONE))) - (std::min<size_t>(1, thumbStickKeys_[static_cast<size_t>(side)][3]->State(TriggerMode::NONE)));
	}
	return stickState;
}
float GamePad::TriggerState(Side side) const
{
	float triggerState = 0.0f;
	if (currentState_.connected_)
	{
		triggerState = currentState_.triggers_[static_cast<size_t>(side)];
	}
	if (emulationMode_ && triggerState == 0)
	{
		triggerState = triggerKeys_[static_cast<size_t>(side)]->State(TriggerMode::NONE) ? 1.0f : 0.0f;
	}
	return triggerState;
}
