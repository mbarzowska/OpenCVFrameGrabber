#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include<windows.h>

using namespace cv;
using namespace std;

//TODO: smooth things

/* Matrices	*/
/* basic	*/ Mat frame, prev_frame;
/* stamps	*/ Mat frame_to_control_mode, frame_to_save;

/* Helpers and other variables */
Scalar red = Scalar(0, 0, 255);
string path;
string timestamp_for_display, timestamp_for_filename, day;
string window_result = "End result";
SYSTEMTIME lt;

/* Video writing */
string video_name;
VideoWriter output_video;
string mode_string = "Camera to video file";

struct modes
{
	bool recording;
	bool stop;
};

struct modes current_mode = { true, false };

/**
 * Using GetAsyncKeyState | Microsoft Docs: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-getasynckeystate
 */
void mode_update()
{
	if (!output_video.isOpened() && !current_mode.stop) 
	{
		const auto output_path = path + video_name;
		const auto codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
		const auto size = Size(320, 240);
		output_video.open(output_path, codec, 20, size);
		mode_string = "Camera to video file";
	}
	if (GetAsyncKeyState('1')) 
	{
		current_mode.recording = true;
		current_mode.stop = false;
		mode_string = "Camera to video file";
	}
	if (GetAsyncKeyState('3')) 
	{
		current_mode.recording = false;
		current_mode.stop = true;
		mode_string = "Stop";
		output_video.release();
	}
}

/* Custom method used to release VideoCapture objects and destroy all of the HighGUI windows. */
void exit(VideoCapture obj) 
{
	obj.release();
	destroyAllWindows();
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
	VideoCapture cap;
	cap.open(0);

	/* Set properties */
	const auto width = cap.get(CAP_PROP_FRAME_WIDTH);
	const auto height = cap.get(CAP_PROP_FRAME_HEIGHT);
	cap.set(CAP_PROP_FRAME_WIDTH, width / 2);
	cap.set(CAP_PROP_FRAME_HEIGHT, height / 2);

	/* Declare important windows */
	namedWindow(window_result, WINDOW_AUTOSIZE);

	while (waitKey(15) != char(27)) 
	{
		try 
		{
			cap >> frame;

			frame.copyTo(frame_to_save);

			/* Get timestamp */
			GetLocalTime(&lt);
			timestamp_for_filename = to_string(lt.wDay) + "-" + to_string(lt.wMonth) + "-" + to_string(lt.wYear) + " " + to_string(lt.wHour) + "h" + to_string(lt.wMinute) + "m" + to_string(lt.wSecond) + "s";

			video_name = timestamp_for_filename + ".avi";

			mode_update();

			if (current_mode.recording) 
			{
				output_video.write(frame_to_save);
			}

			frame_to_save.copyTo(frame_to_control_mode);

			/* Describe mode */
			putText(frame_to_control_mode, mode_string, Point(15, 25), FONT_HERSHEY_PLAIN, 1, red, 1);

			/* Show a frame with both the mode type and timestamp */
			imshow(window_result, frame_to_control_mode);
		}
		catch (Exception &e) 
		{
			exit(cap);
			return -1;
		}
	}

	exit(cap);
	return 0;
}