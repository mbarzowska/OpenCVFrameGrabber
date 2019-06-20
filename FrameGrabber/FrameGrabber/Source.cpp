#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include<windows.h>
#include "StringHelper.h"

using namespace std;

#define CVUI_IMPLEMENTATION
#include "HeaderFiles/cvui.h"
#define WINDOW_NAME "Frame grabber"

/* cvUI	related	*/
/* on checkbox	*/ bool isRecordingEnabled;
/* on trackbar	*/ int requestedFPS = 20;
/* common		*/ int margin = 20;
/* common		*/ int padding = 20;

/* Matrices	*/
/* basic	*/ cv::Mat frame;
/* stamps	*/ cv::Mat frameToControlMode, frameToSave;

/* Helpers and other variables */
cv::Scalar red = cv::Scalar(0, 0, 255);
string path;
string windowResult = "End result";

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
};

struct modes currentMode = { false, false, false };

/**
 * Using GetAsyncKeyState | Microsoft Docs: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-getasynckeystate
 */
void modeUpdate(int requestedFPS)
{
	if (!outputVideo.isOpened() && isRecordingEnabled && !currentMode.stop)
	{
		videoName = strhelp::createVideoName();
		const auto outputPath = path + videoName;
		const auto codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
		const auto size = cv::Size(320, 240);
		outputVideo.open(outputPath, codec, requestedFPS, size);
		modeString = "New file opened";
	}
	if (GetAsyncKeyState(0x53) || isRecordingEnabled) /* 53 is vk code for S */
	{
		currentMode.recording = true;
		currentMode.stop = false;
		modeString = "Camera to video file";
	}
	if (GetAsyncKeyState(0x45) || !isRecordingEnabled) /* 45 is vk code for E */
	{
		currentMode.recording = false;
		currentMode.stop = true;
		modeString = "Stopped";
		outputVideo.release();
	}
	if (GetAsyncKeyState(0x56)) /* 56 is vk code for V */
	{
		currentMode.recording = false;
		currentMode.stop = true;
		currentMode.modeVideo = true;
		currentMode.playVideo = false;
		modeString = "Stoped recording if there was any, open video mode.";
		outputVideo.release();
	}
	if (GetAsyncKeyState(0x20) || currentMode.modeVideo) /* 20 is vk code for SPACE */
	{
		currentMode.playVideo = !currentMode.playVideo;
		modeString = "Running video";
	}
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
	
	/* Open a camera for video capturing */
	cv::VideoCapture cap;
	cap.open(0);

	/* Set properties */
	auto capWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	auto capHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, capWidth / 2);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, capHeight / 2);
	capWidth = capWidth / 2;
	capHeight = capHeight / 2;

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
			if (!currentMode.modeVideo)
			{
				cap >> frame;
				frame.copyTo(frameToSave);
			}

			/* Set cvUI window */
			cvui::rect(gui, margin, margin, padding + menuWidth + padding, cvUIWindowHeight - 2 * margin, 0x454545, 0x454545);
			cvui::printf(gui, margin + padding, margin + padding, "Menu");
			isRecordingEnabled = cvui::checkbox(gui, margin + padding, margin + 2 * padding, "Record straight to video file", &currentMode.recording);
			cvui::printf(gui, margin + 2 * padding, margin + 3 * padding, "Set FPS:");
			cvui::trackbar(gui, margin + 2 * padding, margin + 4 * padding, menuWidth - padding, &requestedFPS, 10, 100, 1);
			
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
				/// TODO: Add handle of playing the film
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