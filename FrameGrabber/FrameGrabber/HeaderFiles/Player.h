/*
Helper functions for build-in custom player:
	ADD DESCRIPTION HERE
*/

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <vector>
#include <string>
#include <windows.h>

#define ONE_STEP 1
#define KEY_STEP 15
#define BIG_STEP 50

enum signalEnum
{
	PLAYER_BEGIN    = 0,  // Go back to first frame
	PLAYER_END      = 1,  // Go to the last frame
	PLAYER_NEXT_L   = 2,  // Previous frame
	PLAYER_NEXT_R   = 3,  // Next frame
	PLAYER_KEY_L    = 4,  // Go backward 15 frames
	PLAYER_KEY_R    = 5,  // Go forward 15 frames
	PLAYER_BACKWARD = 6,  // Go backward 50 frames
	PLAYER_FORWARD  = 7,  // Go forward 50 frames
	PLAYER_SCENE_L  = 8,  // Go form current frame to first one
	PLAYER_SCENE_R  = 9,  // Go from current frame to last one
	PLAYER_STANDARD = 10, // Add NORM_STEP 
	PLAYER_NONE	    = 11  // No action
};

// Function definitions
namespace player
{
	signalEnum playerSignals;

	long long int frameNum = 0; // frame counter
	long long int frameMin = 0; // min frame number
	long long int frameMax = 0; // max frame number
	int playerSignal = PLAYER_NONE;

	/**
	Set new frameNum based on passed option _playerSignal

	@param customName name added at the end of folder's name
	*/
	void playerAction(long long int *frameNum, int _playerSignal);

} // namespace player

// Function implementations
namespace player
{
	void playerAction(long long int *frameNum, int _playerSignal)
	{
		switch (_playerSignal)
		{
		case PLAYER_BEGIN:
			*frameNum = frameMin;
			break;
		case PLAYER_END:
			*frameNum = frameMax;
			break;
		case PLAYER_NEXT_L:
			if (*frameNum - ONE_STEP >= frameMin)
				*frameNum -= ONE_STEP;
			break;
		case PLAYER_NEXT_R:
			if (*frameNum + ONE_STEP <= frameMax)
				*frameNum += ONE_STEP;
			break;
		case PLAYER_KEY_L:
			if (*frameNum - KEY_STEP >= frameMin)
				*frameNum -= KEY_STEP;
			break;
		case PLAYER_KEY_R:
			if (*frameNum + KEY_STEP <= frameMax)
				*frameNum += KEY_STEP;
			break;
		case PLAYER_BACKWARD:
			if (*frameNum - BIG_STEP >= frameMin)
				*frameNum -= BIG_STEP;
			break;
		case PLAYER_FORWARD:
			if (*frameNum + BIG_STEP <= frameMax)
				*frameNum += BIG_STEP;
			break;
		case PLAYER_SCENE_L:
			playerAction(frameNum, PLAYER_NEXT_L);
			break;
		case PLAYER_SCENE_R:
			playerAction(frameNum, PLAYER_NEXT_R);
			break;
		case PLAYER_STANDARD:
			if (*frameNum + ONE_STEP <= frameMax)
				*frameNum += ONE_STEP;
			break;
		case PLAYER_NONE:
			break;
		default:
			printf("Unknown value for _playerSignal!");
			break;
		}
	}
}

#endif