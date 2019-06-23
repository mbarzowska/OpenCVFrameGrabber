/*
Keyboard input helpers.
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <vector>
#include <string>
#include <windows.h>
#include "HeaderFiles/Modes.h"
#include "HeaderFiles/Player.h"

#define KEYBOARD_SIZE 256
#define VK_E 0x45
#define VK_S 0x53
#define VK_P 0x50
#define VK_Q 0x51
#define VK_V 0x56
#define VK_ALT VK_MENU

#define CHECK_MSB(x) ((x & 0x80) == 0x80)

// Function definitions
namespace keyboard
{
	BYTE readKeys[KEYBOARD_SIZE] = { (BYTE)0 };
	BYTE userKeys[KEYBOARD_SIZE] = { (BYTE)0 };
	BYTE clearKeys[KEYBOARD_SIZE] = { (BYTE)0 }; // All are 0's for clearing keyboard input
	std::string currentCommand = "";

	void initKeyboard(BYTE *_userKeys);

	void clearKeyboard();

	void updateKeys(BYTE *keysOne, BYTE *keysTwo);

	void updateCommandString(BYTE *_userKeys, std::string& _cmdStr);

	void clearCommands(BYTE *_readKeys, BYTE *_userKeys);

	void pathInputModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, bool *_isPathInputModeEnabled);

	void pathModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, std::string& _userPath);

	void recordingModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, \
							   std::string& _modeString, \
							   cv::VideoWriter *_outputVideo);

	void videoModeKeyboard(BYTE * _readKeys, BYTE * _userKeys, modes * _currentMode, \
						   long long int * frameNum, long long int * frameMax, int * playerSignal, \
						   std::string & _userPath, std::string & _modeString, \
						   cv::VideoWriter *_outputVideo, cv::VideoCapture *_cap, \
						   double * capWidth, double * capHeight);

	void logoModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, \
						  logoMoveDirections *_moveDirection, \
						  int *_logoX, int *_logoY);

}

namespace keyboard
{
	void initKeyboard(BYTE *_userKeys)
	{
		// Get current keyboard input
		GetKeyboardState(_userKeys);
		return;
	}

	void clearKeyboard()
	{
		SetKeyboardState(clearKeys);
		return;
	}

	void updateKeys(BYTE *keysOne, BYTE *keysTwo)
	{
		for (int i = 0; i < KEYBOARD_SIZE; i++)
		{
			if (CHECK_MSB(keysOne[i]))
				keysTwo[i] = keysOne[i];
		}
		return;
	}

	void updateCommandString(BYTE *_userKeys, std::string& _cmdStr)
	{
		_cmdStr = "";
		if (CHECK_MSB(_userKeys[VK_SHIFT]))
			_cmdStr += "SHIFT + ";
		if (CHECK_MSB(_userKeys[VK_CONTROL]))
			_cmdStr += "CTRL + ";
		if (CHECK_MSB(_userKeys[VK_ALT]))
			_cmdStr += "ALT + ";
		if (CHECK_MSB(_userKeys[VK_LEFT]))
			_cmdStr += "LEFT + ";
		if (CHECK_MSB(_userKeys[VK_RIGHT]))
			_cmdStr += "RIGHT + ";
		if (CHECK_MSB(_userKeys[VK_UP]))
			_cmdStr += "UP + ";
		if (CHECK_MSB(_userKeys[VK_DOWN]))
			_cmdStr += "DOWN + ";
		if (CHECK_MSB(_userKeys[VK_SPACE]))
			_cmdStr += "SPACE + ";
		if (CHECK_MSB(_userKeys[VK_DELETE]))
			_cmdStr += "DELETE + ";
		if (CHECK_MSB(_userKeys[VK_E]))
			_cmdStr += "E + ";
		if (CHECK_MSB(_userKeys[VK_S]))
			_cmdStr += "S + ";
		if (CHECK_MSB(_userKeys[VK_V]))
			_cmdStr += "V + ";
		if (CHECK_MSB(_userKeys[VK_P]))
			_cmdStr += "P + ";
		if (CHECK_MSB(_userKeys[VK_Q]))
			_cmdStr += "Q + ";
		// Strip last three
		_cmdStr = _cmdStr.substr(0, _cmdStr.size() - 3);
		return;
	}

	void clearCommands(BYTE *_readKeys, BYTE *_userKeys)
	{
		if (CHECK_MSB(_userKeys[VK_Q]))
		{
			_readKeys[VK_E] = 0x0;
			_readKeys[VK_S] = 0x0;
			_readKeys[VK_V] = 0x0;
			_readKeys[VK_P] = 0x0;
			_readKeys[VK_Q] = 0x0;
			_readKeys[VK_SPACE] = 0x0;
			_readKeys[VK_DELETE] = 0x0;
			_readKeys[VK_LEFT] = 0x0;
			_readKeys[VK_RIGHT] = 0x0;
			_readKeys[VK_UP] = 0x0;
			_readKeys[VK_DOWN] = 0x0;
			_readKeys[VK_CONTROL] = 0x0;
			_readKeys[VK_SHIFT] = 0x0;
			_readKeys[VK_ALT] = 0x0;
			_userKeys[VK_E] = 0x0;
			_userKeys[VK_S] = 0x0;
			_userKeys[VK_V] = 0x0;
			_userKeys[VK_P] = 0x0;
			_userKeys[VK_Q] = 0x0;
			_userKeys[VK_SPACE] = 0x0;
			_userKeys[VK_DELETE] = 0x0;
			_userKeys[VK_LEFT] = 0x0;
			_userKeys[VK_RIGHT] = 0x0;
			_userKeys[VK_UP] = 0x0;
			_userKeys[VK_DOWN] = 0x0;
			_userKeys[VK_CONTROL] = 0x0;
			_userKeys[VK_SHIFT] = 0x0;
			_userKeys[VK_ALT] = 0x0;
		}
		return;
	}

	void pathInputModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode)
	{
		if (CHECK_MSB(_userKeys[VK_P]))
		{
			_currentMode->pathInput = !(_currentMode->pathInput);
			_userKeys[VK_P] = 0x0;
			_readKeys[VK_P] = 0x0;
		}
		return;
	}

	void pathModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, std::string& _userPath)
	{
		if (_currentMode->pathInput)
		{
			if (CHECK_MSB(_userKeys[VK_CONTROL]) && CHECK_MSB(_userKeys[VK_V]))
			{
				if (OpenClipboard(NULL)) {
					HANDLE clip = GetClipboardData(CF_TEXT);
					CloseClipboard();
					_userPath = (char *)clip;
				}
				_userKeys[VK_V] = 0x0;
				_userKeys[VK_CONTROL] = 0x0;
				_readKeys[VK_V] = 0x0;
				_readKeys[VK_CONTROL] = 0x0;
			}
			if (CHECK_MSB(userKeys[VK_DELETE]))
			{
				_userPath = "";
				_userKeys[VK_DELETE] = 0x0;
				_readKeys[VK_DELETE] = 0x0;
			}
		}
		return;
	}

	void recordingModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, \
						       std::string& _modeString, \
							   cv::VideoWriter *_outputVideo)
	{
		if ((CHECK_MSB(_userKeys[VK_S]) || !_currentMode->modeVideo) ||
			(_currentMode->modeVideo && _currentMode->playVideo))
		{
			_currentMode->recording = true;
			_currentMode->stop = false;
			_modeString = "To video file";
			_userKeys[VK_S] = 0x0;
			_readKeys[VK_S] = 0x0;
		}
		if ((CHECK_MSB(_userKeys[VK_E]) || !_currentMode->modeVideo) ||
			(!_currentMode->playVideo))
		{
			_currentMode->recording = false;
			_currentMode->stop = true;
			_modeString = "Stopped";
			_outputVideo->release();
			_userKeys[VK_E] = 0x0;
			_readKeys[VK_E] = 0x0;
		}
		return;
	}

	void videoModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, \
						   long long int *frameNum, long long int *frameMax, int *playerSignal, \
						   std::string& _userPath, std::string& _modeString, \
						   cv::VideoWriter *_outputVideo, cv::VideoCapture *_cap, \
						   double *capWidth, double *capHeight)
	{
		if (CHECK_MSB(_userKeys[VK_V]))
		{
			_currentMode->recording = false;
			_currentMode->stop = true;
			_currentMode->modeVideo = !_currentMode->modeVideo;
			_currentMode->playVideo = false;
			if (_currentMode->modeVideo && _userPath != "")
			{
				// cap.release();
				try
				{
					_outputVideo->release();
					_cap->open(_userPath);
					// Set max and starting frames
					*frameNum = 0;
					*frameMax = static_cast<long long int>(_cap->get(cv::CAP_PROP_FRAME_COUNT)) - 1; // -1 !!!!
					_modeString = "Stopped recording if there was any, open video mode.";
				}
				catch (cv::Exception &e)
				{
					_modeString = "Set valid path to video file.";
				}
			}
			else
			{
				// cap.release();
				/* Open a camera for video capturing */
				_cap->open(0);

				/* Set properties */
				*capWidth = _cap->get(cv::CAP_PROP_FRAME_WIDTH);
				*capHeight = _cap->get(cv::CAP_PROP_FRAME_HEIGHT);
				_cap->set(cv::CAP_PROP_FRAME_WIDTH, *capWidth / 2);
				_cap->set(cv::CAP_PROP_FRAME_HEIGHT, *capHeight / 2);
				*capWidth = *capWidth / 2;
				*capHeight = *capHeight / 2;
				_modeString = "Stopped video mode, started recording mode.";
			}
			_userKeys[VK_V] = 0x0;
			_readKeys[VK_V] = 0x0;
		}
		if (_currentMode->modeVideo)
		{
			// Restore the signal after loop
			if (_currentMode->playVideo)
				*playerSignal = PLAYER_STANDARD;
			else
				*playerSignal = PLAYER_NONE;

			if (CHECK_MSB(_userKeys[VK_SPACE]))
			{
				_currentMode->playVideo = !_currentMode->playVideo;
				if (_currentMode->playVideo)
					*playerSignal = PLAYER_STANDARD;
				else
					*playerSignal = PLAYER_NONE;
				_modeString = "Running/Stopped video";
				_userKeys[VK_SPACE] = 0x0;
				_readKeys[VK_SPACE] = 0x0;
			}
			// Order here is an important thing
			if (CHECK_MSB(_userKeys[VK_CONTROL]) && CHECK_MSB(_userKeys[VK_SHIFT]) && CHECK_MSB(_userKeys[VK_LEFT]))
			{
				*playerSignal = PLAYER_SCENE_L;
				_userKeys[VK_LEFT] = 0x0;
				_userKeys[VK_SHIFT] = 0x0;
				_userKeys[VK_CONTROL] = 0x0;
				_readKeys[VK_LEFT] = 0x0;
				_readKeys[VK_SHIFT] = 0x0;
				_readKeys[VK_CONTROL] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_CONTROL]) && CHECK_MSB(_userKeys[VK_SHIFT]) && CHECK_MSB(_userKeys[VK_RIGHT]))
			{
				*playerSignal = PLAYER_SCENE_R;
				_userKeys[VK_RIGHT] = 0x0;
				_userKeys[VK_SHIFT] = 0x0;
				_userKeys[VK_CONTROL] = 0x0;
				_readKeys[VK_RIGHT] = 0x0;
				_readKeys[VK_SHIFT] = 0x0;
				_readKeys[VK_CONTROL] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_CONTROL]) && CHECK_MSB(_userKeys[VK_LEFT]))
			{
				*playerSignal = PLAYER_BEGIN;
				_userKeys[VK_LEFT] = 0x0;
				_userKeys[VK_CONTROL] = 0x0;
				_readKeys[VK_LEFT] = 0x0;
				_readKeys[VK_CONTROL] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_CONTROL]) && CHECK_MSB(_userKeys[VK_RIGHT]))
			{
				*playerSignal = PLAYER_END;
				_userKeys[VK_RIGHT] = 0x0;
				_userKeys[VK_CONTROL] = 0x0;
				_readKeys[VK_RIGHT] = 0x0;
				_readKeys[VK_CONTROL] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_SHIFT]) && CHECK_MSB(_userKeys[VK_LEFT]))
			{
				*playerSignal = PLAYER_KEY_L;
				_userKeys[VK_LEFT] = 0x0;
				_userKeys[VK_SHIFT] = 0x0;
				_readKeys[VK_LEFT] = 0x0;
				_readKeys[VK_SHIFT] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_SHIFT]) && CHECK_MSB(_userKeys[VK_RIGHT]))
			{
				*playerSignal = PLAYER_KEY_R;
				_userKeys[VK_RIGHT] = 0x0;
				_userKeys[VK_SHIFT] = 0x0;
				_readKeys[VK_RIGHT] = 0x0;
				_readKeys[VK_SHIFT] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_ALT]) && CHECK_MSB(_userKeys[VK_LEFT]))
			{
				*playerSignal = PLAYER_BACKWARD;
				_userKeys[VK_LEFT] = 0x0;
				_userKeys[VK_MENU] = 0x0;
				_readKeys[VK_LEFT] = 0x0;
				_readKeys[VK_MENU] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_ALT]) && CHECK_MSB(_userKeys[VK_RIGHT]))
			{
				*playerSignal = PLAYER_FORWARD;
				_userKeys[VK_RIGHT] = 0x0;
				_userKeys[VK_MENU] = 0x0;
				_readKeys[VK_RIGHT] = 0x0;
				_readKeys[VK_MENU] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_LEFT]))
			{
				*playerSignal = PLAYER_NEXT_L;
				_userKeys[VK_LEFT] = 0x0;
				_readKeys[VK_LEFT] = 0x0;
			}
			if (CHECK_MSB(_userKeys[VK_RIGHT]))
			{
				*playerSignal = PLAYER_NEXT_R;
				_userKeys[VK_RIGHT] = 0x0;
				_readKeys[VK_RIGHT] = 0x0;
			}
		}
		return;
	}

	void logoModeKeyboard(BYTE *_readKeys, BYTE *_userKeys, modes *_currentMode, \
					      logoMoveDirections *_moveDirection, \
						  int *_logoX, int *_logoY)
	{
		if (_currentMode->applyLogo)
		{
			if (_currentMode->moveLogo)
			{
				if (CHECK_MSB(_userKeys[VK_LEFT]) || _moveDirection->left)
				{
					*_logoX -= 1;
					_userKeys[VK_LEFT] = 0x0;
					_readKeys[VK_LEFT] = 0x0;
				}
				else if (CHECK_MSB(_userKeys[VK_UP]) || _moveDirection->up)
				{
					*_logoY -= 1;
					_userKeys[VK_UP] = 0x0;
					_readKeys[VK_UP] = 0x0;
				}
				else if (CHECK_MSB(_userKeys[VK_RIGHT]) || _moveDirection->right)
				{
					*_logoX += 1;
					_userKeys[VK_RIGHT] = 0x0;
					_readKeys[VK_RIGHT] = 0x0;
				}
				else if (CHECK_MSB(_userKeys[VK_DOWN]) || _moveDirection->down)
				{
					*_logoY += 1;
					_userKeys[VK_DOWN] = 0x0;
					_readKeys[VK_DOWN] = 0x0;
				}
			}
		}
		else
		{
			_currentMode->moveLogo = false;
		}
		return;
	}

}

#endif