#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include "HeaderFiles/StringHelper.h"

using namespace std;

#define CVUI_IMPLEMENTATION
#include "HeaderFiles/cvui.h"
#define CVUI_WINDOW_NAME "Frame grabber"

/* cvUI	related	*/
/* on checkbox	*/ bool isRecordingModeEnabled, isPathInputModeEnabled, isLogoModeEnabled, isMoveLogoModeEnabled;
/* on trackbar	*/ int requestedFPS = 20;
/* common		*/ int margin = 20, padding = 20;
/* */ string secondPanelAlertString = "", secondPanelAdditionString = "";
/* */ bool isLogoMovingMessedUp;

/* Matrices	*/
/* basic	*/ cv::Mat frame;

/* Helpers and other variables */
BYTE clearKeys[256] = { (BYTE)0 }; // All are 0's for clearing keyboard input
cv::VideoCapture cap;
string path;
string userPath = ""; // path to file defined by user
char userChar = 0;
unsigned long long int frameNum = 0; // frame counter
unsigned long long int frameMin = 0; // min frame number
unsigned long long int frameMax = 0; // max frame number
unsigned long long int frameStep = 1; // current frame step
double capWidth = 0.0;
double capHeight = 0.0;
string logoPath = R"(C:\Users\barzo\Desktop\logo-cv.png)"; // TODO
int logoX = 0;
int logoY = 0;
cv::Mat frameWithLogo;
double alpha = 0.3;
cv::Mat3b roi;
bool restore; // 349

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
	bool applyLogo;
	bool moveLogo;
};

struct modes currentMode = { false, false, false, false, false, false, false };

struct logoMoveDirections
{
	bool left;
	bool up;
	bool right;
	bool down;
};

struct logoMoveDirections moveDirection = { false, false, false, false };

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
	bool pressedLeftArrow = GetKeyState(0x25);
	bool pressedUpArrow = GetKeyState(0x26);
	bool pressedRightArrow = GetKeyState(0x27);
	bool pressedDownArrow = GetKeyState(0x28);
	bool pressedShift = GetKeyState(0x10);
	bool pressedCtrl = GetKeyState(0x11);
	bool pressedAlt = GetKeyState(0x12);

	if (isPathInputModeEnabled)
	{
		if (pressedCtrl && pressedV)
		{
			if (OpenClipboard(NULL)) {
				HANDLE clip = GetClipboardData(CF_TEXT);
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
			(isRecordingModeEnabled && currentMode.modeVideo && currentMode.playVideo))
		{
			currentMode.recording = true;
			currentMode.stop = false;
			isRecordingModeEnabled = true;
			modeString = "To video file";
		}

		if ((pressedE || !isRecordingModeEnabled && !currentMode.modeVideo) ||
			(!isRecordingModeEnabled && !currentMode.playVideo))
		{
			currentMode.recording = false;
			currentMode.stop = true;
			// modeString = "Stopped";
			outputVideo.release();
		}

		if (pressedV)
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

		if (pressedSpace && currentMode.modeVideo)
		{
			currentMode.playVideo = !currentMode.playVideo;
			modeString = "Running/Stopped video";
		}

		if (isLogoModeEnabled)
		{
			currentMode.applyLogo = true;
			if (isMoveLogoModeEnabled)
			{
				currentMode.moveLogo = true;
				if (pressedLeftArrow || moveDirection.left)
				{
					logoX -= 1;
				}
				else if (pressedUpArrow || moveDirection.up)
				{
					logoY -= 1;
				}
				else if (pressedRightArrow || moveDirection.right)
				{
					logoX += 1;
				}
				else if (pressedDownArrow || moveDirection.down)
				{
					logoY += 1;
				}
			}
			if (!isMoveLogoModeEnabled)
			{
				currentMode.moveLogo = false;
			}
		}

		if (!isLogoModeEnabled)
		{
			currentMode.applyLogo = false;
			currentMode.moveLogo = false;
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

	/* Initialize cvUI menu window */
	int menuWidth = 200;
	cvui::init(CVUI_WINDOW_NAME);
	int cvUIWindowWidth = margin + padding + menuWidth + 3 * padding + capWidth + 3 * padding + capWidth + padding + margin;
	int cvUIWindowHeight = 700;
	cv::Mat gui = cv::Mat(cv::Size(cvUIWindowWidth, cvUIWindowHeight), CV_8UC3);
	gui = cv::Scalar(55, 55, 55);

	cv::Mat4b logo = cv::imread(logoPath, cv::IMREAD_UNCHANGED);
	resize(logo, logo, cv::Size(64, 64));

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
			}

			frame.copyTo(frameWithLogo);

			if (currentMode.applyLogo)
			{
				try {
					if (restore)
					{
						logoX = 0;
						logoY = 0;
						roi = frameWithLogo(cv::Rect(logoX, logoY, logo.cols, logo.rows));
					}
					else
					{
						roi = frameWithLogo(cv::Rect(logoX, logoY, logo.cols, logo.rows));
					}
					restore = false;
					for (int r = 0; r < roi.rows; ++r)
					{
						for (int c = 0; c < roi.cols; ++c)
						{
							const cv::Vec4b& vf = logo(r, c);
							if (vf[3] > 0) // alpha channel > 0
							{
								// Blending
								cv::Vec3b& vb = roi(r, c);
								vb[0] = alpha * vf[0] + (1 - alpha) * vb[0];
								vb[1] = alpha * vf[1] + (1 - alpha) * vb[1];
								vb[2] = alpha * vf[2] + (1 - alpha) * vb[2];
							}
						}
					}
					isLogoMovingMessedUp = false;
				}
				catch (cv::Exception &e)
				{
					isLogoMovingMessedUp = true;
					secondPanelAlertString = "Can't move logo any further!";
					secondPanelAdditionString = "Restore previous position to continue.";
				}
			}

			/* Set cvUI window */
			int menuPanelX = margin;
			int menuPanelY = margin;
			int menuPanelWidth = padding + menuWidth + padding;
			int menuPanelHeight = cvUIWindowHeight - 2 * margin;
			cvui::rect(gui, menuPanelX, menuPanelY, menuPanelWidth, menuPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, menuPanelX + padding, menuPanelY + padding, menuWidth, menuPanelHeight, padding);
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
				int status = cvui::iarea(menuPanelX + padding, margin + 10.5 * padding, menuWidth, padding);
				cvui::rect(gui, menuPanelX + padding, margin + 10.5 * padding, menuWidth, padding, 0x4d4d4d, 0x373737);
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
			isLogoModeEnabled = cvui::checkbox("Put logo on", &currentMode.applyLogo);
			cvui::endColumn();

			int firstPanelX = margin + padding + menuWidth + 2 * padding;
			int firstPanelY = margin;
			int firstPanelWidth = padding + capWidth + padding;
			int firstPanelHeight = margin + capHeight + 15 * padding;
			cvui::rect(gui, firstPanelX, firstPanelY, firstPanelWidth, firstPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, firstPanelX + padding, firstPanelY + padding, capWidth, firstPanelHeight, padding);
			cvui::image(frame);
			cvui::text("Frame track bar:");
			// TODO: Zamienic trackbar na wlasciwy, narazie placeholder
			cvui::trackbar(capWidth, &requestedFPS, 10, 100);
			cvui::endColumn();

			// TODO: Obsluga drugiego okna, narazie placeholder
			int secondPanelX = margin + padding + menuWidth + 3 * padding + capWidth + 2 * padding;
			int secondPanelY = margin;
			int secondPanelWidth = padding + capWidth + padding;
			int secondPanelHeight = padding + capHeight + 15 * padding;
			cvui::rect(gui, secondPanelX, secondPanelY, secondPanelWidth, secondPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, secondPanelX + padding, secondPanelY + padding, capWidth, secondPanelHeight, padding);
			if (isLogoModeEnabled) {
				cvui::image(frameWithLogo);
				isMoveLogoModeEnabled = cvui::checkbox("Enable logo move", &currentMode.moveLogo);
				if (isMoveLogoModeEnabled) {
					if (isLogoMovingMessedUp) {
						cvui::text(secondPanelAlertString);
						cvui::text(secondPanelAdditionString);
					}
					int buttonWidth = 60;
					int buttonHeight = 30;
					cvui::beginRow();
					moveDirection.left = cvui::button(buttonWidth, buttonHeight, "LEFT");
					moveDirection.right = cvui::button(buttonWidth, buttonHeight, "RIGHT");
					moveDirection.up = cvui::button(buttonWidth, buttonHeight, "UP");
					moveDirection.down = cvui::button(buttonWidth, buttonHeight, "DOWN");
					cvui::endRow();
					if (isLogoMovingMessedUp)
					{
						cvui::text("Or restore to starting point:");
						restore = cvui::button(buttonWidth * 3, buttonHeight, "Restore to (0, 0)");
					}
				}
			}
			cvui::endColumn();

			cvui::imshow(CVUI_WINDOW_NAME, gui);

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
				{
					// replaying
					frameNum = 0;
					cap.set(cv::CAP_PROP_POS_FRAMES, 0);
				}
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
			_getch();
			exit(cap);
			return -1;
		}
	}

	exit(cap);
	return 0;
}