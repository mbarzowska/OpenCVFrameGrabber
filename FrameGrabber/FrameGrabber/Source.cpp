#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include<windows.h>
#include <conio.h>
#include "HeaderFiles/StringHelper.h"

using namespace std;

#define CVUI_IMPLEMENTATION
#include "HeaderFiles/cvui.h"
#define WINDOW_NAME "Frame grabber"

/* cvUI	related	*/
/* on checkbox	*/ bool isRecordingModeEnabled, isPathInputModeEnabled;
/* on trackbar	*/ int requestedFPS = 20;
/* common		*/ int margin = 20;
/* common		*/ int padding = 20;

/* Matrices	*/
/* basic	*/ cv::Mat frame;
/* stamps	*/ cv::Mat frameToControlMode, frameToSave;

/* Helpers and other variables */
BYTE clearKeys[256] = { (BYTE)0 }; // All are 0's for clearing keyboard input
cv::VideoCapture cap;
cv::Scalar red = cv::Scalar(0, 0, 255);
string path;
string userPath = ""; // path to file defined by user
char userChar = 0;
string windowResult = "End result";
unsigned long long int frameNum = 0; // frame counter
unsigned long long int frameMin = 0; // min frame number
unsigned long long int frameMax = 0; // max frame number
unsigned long long int frameStep = 1; // current frame step
double capWidth = 0.;
double capHeight = 0.;

/* Video writing */
string videoName;
cv::VideoWriter outputVideo;
string modeString;

struct modes
{
	bool recording;
	bool stop;
	bool modeVideo;	// Open video mode, exit the mode pressing V
	bool playVideo; // Start/stop running of video
	bool pathInput; // Let get the path
};

struct modes currentMode = { false, false, false, false };

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
	bool pressedS = GetKeyState(0x53);
	bool pressedE = GetKeyState(0x45);
	bool pressedV = GetKeyState(0x56);
	bool pressedSpace = GetKeyState(0x20);
	bool pressedLeft = GetKeyState(0x25);
	bool pressedRight = GetKeyState(0x27);
	bool pressedShift = GetKeyState(0x10);
	bool pressedCtrl = GetKeyState(0x11);
	bool pressedAlt = GetKeyState(0x12);

	if (isPathInputModeEnabled)
	{
		if (pressedCtrl && pressedV)
		{
			HANDLE clip;
			// Check if we can open it
			if (OpenClipboard(NULL)) {
				clip = GetClipboardData(CF_TEXT);
				CloseClipboard();
				userPath = (char *)clip;
			}
		}
		// TODO: Write letters
		// if (pressedS)
			// userChar = 's';
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
		if ((pressedS || isRecordingModeEnabled && !currentMode.modeVideo) ||
			(isRecordingModeEnabled && currentMode.modeVideo && currentMode.playVideo)) /* 53 is vk code for S */
		{
			currentMode.recording = true;
			currentMode.stop = false;
			isRecordingModeEnabled = true;
			modeString = "To video file";
		}
		if ((pressedE || !isRecordingModeEnabled && !currentMode.modeVideo) ||
			(!isRecordingModeEnabled && !currentMode.playVideo)) /* 45 is vk code for E */
		{
			currentMode.recording = false;
			currentMode.stop = true;
			modeString = "Stopped";
			outputVideo.release();
		}
		if (pressedV) /* 56 is vk code for V */
		{
			currentMode.recording = false;
			currentMode.stop = true;
			currentMode.modeVideo = !currentMode.modeVideo;
			currentMode.playVideo = false;
			if (currentMode.modeVideo)
			{
				// cap.release();
				outputVideo.release();
				cap.open(userPath);
				// Set max and starting frames
				frameNum = 0;
				frameMax = static_cast<unsigned long long int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
				modeString = "Stopped recording if there was any, open video mode.";
			}
			else
			{
				// cap.release();
				openCamera();
				modeString = "Stopped video mode, started recording mode.";
			}
		}
		if (pressedSpace && currentMode.modeVideo) /* 20 is vk code for SPACE */
		{
			currentMode.playVideo = !currentMode.playVideo;
			modeString = "Running/Stopped video";
		}
	}
	// Clear keyborad
	SetKeyboardState(clearKeys);
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

	/* Declare important windows */
	cv::namedWindow(windowResult, cv::WINDOW_AUTOSIZE);

	/* Initialize cvUI menu window */
	int menuWidth = 200;
	cvui::init(WINDOW_NAME);
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
				cap.set(cv::CAP_PROP_POS_FRAMES, frameNum);
				cap >> frame;
			}
			else
			{
				cap >> frame;
				frame.copyTo(frameToSave);
			}

			/* Set cvUI window */
			cvui::rect(gui, margin, margin, padding + menuWidth + padding, cvUIWindowHeight - 2 * margin, 0x454545, 0x454545);
			cvui::printf(gui, margin + padding, margin + padding, "Menu");
			isRecordingModeEnabled = cvui::checkbox(gui, margin + padding, margin + 2 * padding, "Record straight to video file", &currentMode.recording);
			cvui::printf(gui, margin + 2 * padding, margin + 3 * padding, "Set FPS:");
			cvui::trackbar(gui, margin + 2 * padding, margin + 4 * padding, menuWidth - padding, &requestedFPS, 10, 100, 1);
			isPathInputModeEnabled = cvui::checkbox(gui, margin + padding, margin + 7 * padding, "Enable path input mode", &currentMode.pathInput);
			if (isPathInputModeEnabled)
			{
				//if (userChar != 0)
				//{
				//	userPath += userChar;
				//	userChar = 0;
				//}
				int status = cvui::iarea(margin + 2 * padding, margin + 8 * padding, menuWidth - padding, padding);
				cvui::rect(gui, margin + 2 * padding, margin + 8 * padding, menuWidth - padding, padding, 0x4d4d4d, 0x373737);
				const char *userPathC = userPath.c_str();
				cvui::printf(gui, margin + 2 * padding, margin + 8 * padding, userPathC);
				// TODO: get rid of it, helper only
				switch (status) {
				case cvui::CLICK:  std::cout << "Clicked!" << std::endl; break;
				case cvui::DOWN:   cvui::printf(gui, margin + padding, margin + 9 * padding, "Mouse is: DOWN"); break;
				case cvui::OVER:   cvui::printf(gui, margin + padding, margin + 9 * padding, "Mouse is: OVER"); break;
				case cvui::OUT:    cvui::printf(gui, margin + padding, margin + 9 * padding, "Mouse is: OUT"); break;
				}
			}

			cvui::rect(gui, margin + padding + menuWidth + 2 * padding, margin, padding + capWidth + padding, margin + capHeight + 4 * padding, 0x454545, 0x454545);
			cvui::image(gui, margin + padding + menuWidth + 3 * padding, margin + padding, frame);
			cvui::trackbar(gui, margin + padding + menuWidth + 3 * padding, margin + padding + capHeight + padding, capWidth, &requestedFPS, 10, 100, 1);

			//TODO: Obs³uga drugiego okna, narazie placeholder
			cvui::rect(gui, margin + padding + menuWidth + 3 * padding + capWidth + 2 * padding, margin, padding + capWidth + padding, padding + capHeight + padding, 0x454545, 0x454545);
			cvui::image(gui, margin + padding + menuWidth + 3 * padding + capWidth + 3 * padding, margin + padding, frame);
			
			cvui::imshow(WINDOW_NAME, gui);

			modeUpdate(requestedFPS);

			// Some video is opened right now
			if (currentMode.modeVideo)
			{
				if (currentMode.playVideo && currentMode.recording)
				{
					outputVideo.write(frame);
				}
				/// TODO: Add handle of playing the film
				if (currentMode.playVideo && (frameNum + frameStep) < frameMax)
				{
					frameNum += frameStep;
				} 
				else if (currentMode.playVideo && (frameNum + frameStep) >= frameMax)
					// replaying
				{
					frameNum = 0;
					cap.set(cv::CAP_PROP_POS_FRAMES, 0);
				}
			}
			else
			{
				if (currentMode.recording)
				{
					outputVideo.write(frameToSave);
				}
			}

			frameToSave.copyTo(frameToControlMode);

			/* Describe mode */
			cv::putText(frameToControlMode, modeString, cv::Point(15, 25), cv::FONT_HERSHEY_PLAIN, 1, red, 1);

			//TODO: Eventually get rid of it
			/* Show a frame with mode type */
			cv::imshow(windowResult, frameToControlMode);
		}
		catch (cv::Exception &e)
		{
			exit(cap);
			return -1;
		}
	}

	exit(cap);
	return 0;
}