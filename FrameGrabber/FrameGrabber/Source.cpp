#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include<windows.h>

using namespace std;

#define CVUI_IMPLEMENTATION
#include "HeaderFiles/cvui.h"
#define WINDOW_NAME "Frame grabber"

/* cvUI	related	*/
/* on checkbox	*/ bool isRecordingEnabled;
/* on trackbar	*/ int requestedFPS = 20;
/* common		*/ int margin = 20;

/* Matrices	*/
/* basic	*/ cv::Mat frame, prev_frame;
/* stamps	*/ cv::Mat frame_to_control_mode, frame_to_save;

/* Helpers and other variables */
cv::Scalar red = cv::Scalar(0, 0, 255);
string path;
string timestamp_for_filename, day;
string window_result = "End result";
SYSTEMTIME lt;

/* Video writing */
string video_name;
cv::VideoWriter output_video;
string mode_string;

struct modes
{
	bool recording;
	bool stop;
};

struct modes current_mode = { false, false };

/**
 * Using GetAsyncKeyState | Microsoft Docs: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-getasynckeystate
 */
void mode_update(int requestedFPS)
{
	if (!output_video.isOpened() && isRecordingEnabled && !current_mode.stop)
	{
		const auto output_path = path + video_name;
		const auto codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
		const auto size = cv::Size(320, 240);
		output_video.open(output_path, codec, requestedFPS, size);
		mode_string = "New file opened";
	}
	if (GetAsyncKeyState(0x53) || isRecordingEnabled) /* 53 is vk code for S */
	{
		current_mode.recording = true;
		current_mode.stop = false;
		mode_string = "Camera to video file";
	}
	if (GetAsyncKeyState(0x45)) /* 45 is vk code for E */
	{
		current_mode.recording = false;
		current_mode.stop = true;
		mode_string = "Stopped";
		output_video.release();
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
	string argv_str(argv[0]);
	string base = argv_str.substr(0, argv_str.find_last_of("\\"));
	string base2 = base.substr(0, base.find_last_of("\\"));
	string base3 = base2.substr(0, base2.find_last_of("\\"));
	path = base3 + "\\FrameGrabber\\Video\\";

	
	/* Open a camera for video capturing */
	cv::VideoCapture cap;
	cap.open(0);

	/* Set properties */
	const auto cap_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	const auto cap_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Declare important windows */
	cv::namedWindow(window_result, cv::WINDOW_AUTOSIZE);

	/* Initialize cvUI menu window */
	cvui::init(WINDOW_NAME);
	cv::Mat menu = cv::Mat(cv::Size(cap_width * 2, cap_height + 40), CV_8UC3);

	while (cv::waitKey(15) != char(27))
	{
		try 
		{
			cap >> frame;
			frame.copyTo(frame_to_save);

			/* Set cvUI window */
			menu = cv::Scalar(49, 52, 49);
			cvui::image(menu, margin, margin, frame);
			isRecordingEnabled = cvui::checkbox(menu, 2 * margin + cap_width, margin, "Enable recording", &current_mode.recording);
			cvui::printf(menu, 2 * margin + cap_width, margin + 30, "Set FPS:");
			cvui::trackbar(menu, 2 * margin + cap_width, margin + 50, 320, &requestedFPS, 10, 100, 1);
			cvui::imshow(WINDOW_NAME, menu);
			
			/* Get timestamp */
			GetLocalTime(&lt);
			timestamp_for_filename = to_string(lt.wDay) + "-" + to_string(lt.wMonth) + "-" + to_string(lt.wYear) + " " + to_string(lt.wHour) + "h" + to_string(lt.wMinute) + "m" + to_string(lt.wSecond) + "s";

			video_name = timestamp_for_filename + ".avi";
			
			mode_update(requestedFPS);

			if (current_mode.recording) 
			{
				output_video.write(frame_to_save);
			}

			frame_to_save.copyTo(frame_to_control_mode);

			/* Describe mode */
			cv::putText(frame_to_control_mode, mode_string, cv::Point(15, 25), cv::FONT_HERSHEY_PLAIN, 1, red, 1);

			//TODO: Eventually get rid of it
			/* Show a frame with mode type */
			cv::imshow(window_result, frame_to_control_mode);
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