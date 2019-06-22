#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include "HeaderFiles/Modes.h"
#include "HeaderFiles/StringHelper.h"
#include "HeaderFiles/Player.h"
#include "HeaderFiles/Keyboard.h"

using namespace std;

#define CVUI_IMPLEMENTATION
#include "HeaderFiles/cvui.h"
#define CVUI_WINDOW_NAME "Frame grabber"

/* cvUI	related	*/
/* on checkbox	*/ bool isRecordingModeEnabled, isPathInputModeEnabled;
/* on trackbar	*/ int requestedFPS = 20;
/* common		*/ int margin = 20, padding = 20;

/* Matrices	*/
/* basic	*/ cv::Mat frame;

/* Helpers and other variables */
cv::VideoCapture cap;
string path;
string userPath = ""; // path to file defined by user
char userChar = 0;
string windowResult = "End result";
double capWidth = 0.;
double capHeight = 0.;

/* Video writing */
string videoName;
cv::VideoWriter outputVideo;
string modeString;

struct modes currentMode = { false, false, false, false, false };

inline void openCamera()
{
	/* Open a camera for video capturing */
	cap.open(0);

	/* Set properties */
	capWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	capHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, capWidth / 2);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, capHeight / 2);
	capWidth = capWidth / 2;
	capHeight = capHeight / 2;
}

void modeUpdate(int requestedFPS)
{
	// Get current keyboard input
	keyboard::initKeyboard(keyboard::userKeys);
	// Copy previous iteration keys
	keyboard::updateKeys(keyboard::readKeys, keyboard::userKeys);
	// Clear command input with Q
	keyboard::clearCommands(keyboard::userKeys);

	if (isPathInputModeEnabled)
	{
		keyboard::pathModeKeyboard(keyboard::readKeys, keyboard::userKeys, userPath);
	}
	else
	{
		if ((!outputVideo.isOpened() && isRecordingModeEnabled && !currentMode.stop && !currentMode.modeVideo) ||
			(!outputVideo.isOpened() && isRecordingModeEnabled && currentMode.modeVideo && currentMode.playVideo))
		{
			videoName = strhelp::createVideoName();
			const auto outputPath = path + videoName;
			const auto codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
			const auto size = cv::Size(320, 240);
			outputVideo.open(outputPath, codec, requestedFPS, size);
			modeString = "New file opened";
		}
		// Keyboard handler for recording state
		keyboard::recordingModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
									    &currentMode, &isRecordingModeEnabled, modeString,
										&outputVideo);
		keyboard::videoModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
									&currentMode, &player::frameNum, &player::frameMax, \
									&player::playerSignal, userPath, modeString, \
									&outputVideo, &cap, &capWidth, &capHeight);
	}
	// Clear KeyboardState
	keyboard::clearKeyboard();
	// Copy current pressed to next state used later at the begining
	keyboard::updateKeys(keyboard::userKeys, keyboard::readKeys);
}

/* Custom method used to release VideoCapture objects and destroy all of the HighGUI windows. */
void exit(cv::VideoCapture obj)
{
	obj.release();
	cv::destroyAllWindows();
}

int main(int argc, char* argv[]) 
{
	//TODO: find more elegant way to determine path
	string argvStr(argv[0]);
	string base = argvStr.substr(0, argvStr.find_last_of("\\"));
	string base2 = base.substr(0, base.find_last_of("\\"));
	string base3 = base2.substr(0, base2.find_last_of("\\"));
	path = base3 + "\\FrameGrabber\\Video\\";

	openCamera();

	/* Initialize cvUI menu window */
	int menuWidth = 200;
	cvui::init(CVUI_WINDOW_NAME);
	int cvUIWindowWidth = margin + padding + menuWidth + 3 * padding + capWidth + 3 * padding + capWidth + padding + margin;
	int cvUIWindowHeight = 700;
	cv::Mat gui = cv::Mat(cv::Size(cvUIWindowWidth, cvUIWindowHeight), CV_8UC3);
	gui = cv::Scalar(55, 55, 55);

	while (cv::waitKey(15) != char(27))
	{
		try 
		{
			if (currentMode.modeVideo)
			{
				cap.set(cv::CAP_PROP_POS_FRAMES, player::frameNum);
				cap >> frame;
			}
			else
			{
				cap >> frame;
			}

			/* Set cvUI window */
			int firstPanelX = margin;
			int firstPanelY = margin;
			int firstPanelWidth = padding + menuWidth + padding;
			int firstPanelHeight = cvUIWindowHeight - 2 * margin;
			cvui::rect(gui, firstPanelX, firstPanelY, firstPanelWidth, firstPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, firstPanelX + padding, firstPanelY + padding, menuWidth, firstPanelHeight, padding);
			cvui::text("Menu");
			isRecordingModeEnabled = cvui::checkbox("Record straight to video file", &currentMode.recording);
			cvui::text("    Set FPS:");
			cvui::trackbar(menuWidth, &requestedFPS, 10, 100);
			isPathInputModeEnabled = cvui::checkbox("Enable path input mode", &currentMode.pathInput);
			if (isPathInputModeEnabled)
			{
				//if (userChar != 0)
				//{
				//	userPath += userChar;
				//	userChar = 0;
				//}
				int status = cvui::iarea(firstPanelX + padding, margin + 10.5 * padding, menuWidth, padding);
				cvui::rect(gui, firstPanelX + padding, margin + 10.5 * padding, menuWidth, padding, 0x4d4d4d, 0x373737);
				const char *userPathC = userPath.c_str();
				cvui::text(userPathC);
				// TODO: get rid of it, helper only
				switch (status) {
				case cvui::CLICK:  std::cout << "Clicked!" << std::endl; break;
				case cvui::DOWN:   cvui::text("Mouse is: DOWN"); break;
				case cvui::OVER:   cvui::text("Mouse is: OVER"); break;
				case cvui::OUT:    cvui::text("Mouse is: OUT"); break;
				}
			}
			cvui::endColumn();

			int secondPanelX = margin + padding + menuWidth + 2 * padding;
			int secondPanelY = margin;
			int secondPanelWidth = padding + capWidth + padding;
			int secondPanelHeight = margin + capHeight + 4 * padding;
			cvui::rect(gui, secondPanelX, secondPanelY, secondPanelWidth, secondPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, secondPanelX + padding, secondPanelY + padding, capWidth, secondPanelHeight, padding);
			cvui::image(frame);
			// Need to convert in that way, maybe put in the player to do it under the hood
			cvui::trackbar(capWidth, &player::frameNum, player::frameMin, player::frameMax + (player::frameMax == 0 ? 1 : 0));
			cvui::endColumn();

			// TODO: Obsluga drugiego okna, narazie placeholder
			int thirdPanelX = margin + padding + menuWidth + 3 * padding + capWidth + 2 * padding;
			int thirdPanelY = margin;
			int thirdPanelWidth = padding + capWidth + padding;
			int thirdPanelHeight = padding + capHeight + padding;
			cvui::rect(gui, thirdPanelX, thirdPanelY, thirdPanelWidth, thirdPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, thirdPanelX + padding, thirdPanelY + padding, capWidth, thirdPanelHeight);
			cvui::image(frame);
			cvui::endColumn();

			cvui::imshow(CVUI_WINDOW_NAME, gui);

			modeUpdate(requestedFPS);

			// Some video is opened right now
			if (currentMode.modeVideo)
			{
				printf("%d\n", player::frameNum);
;				player::playerAction(&player::frameNum, player::playerSignal);
			}
			else
			{
				if (currentMode.recording)
				{
					outputVideo.write(frame);
				}
			}

			//TODO: Eventually get rid of it
			cout << modeString << endl;
		}
		catch (cv::Exception &e)
		{
			exit(cap);
			getchar();
			return -1;
		}
	}

	exit(cap);
	return 0;
}