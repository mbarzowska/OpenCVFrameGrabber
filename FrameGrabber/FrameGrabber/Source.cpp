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
/* on checkbox	*/ 
bool isRecordingModeEnabled = true;
bool isPathInputModeEnabled = false;
bool isLogoModeEnabled, isMoveLogoModeEnabled, isImageModeEnabled, isFrameGrabbingModeEnabled;

/* on trackbar	*/ int requestedFPS = 20;
/* common		*/ int margin = 20, padding = 20;
/* */ string secondPanelAlertString = "", secondPanelAdditionString = "";
/* */ bool isLogoMovingMessedUp, isImageLoaded;
bool saveToFile;

/* Matrices	*/
/* basic	*/ cv::Mat frame;

/* Helpers and other variables */
cv::VideoCapture cap;
string videoSavingPath;
string userPath = ""; // path to file defined by user
char userChar = 0;
double capWidth = 0.0;
double capHeight = 0.0;
string logoPath = R"(C:\Users\Jan Iwaszkiewicz\Pictures\LBJ\avatar.png)"; // TODO
int logoX = 0;
int logoY = 0;
cv::Mat frameWithLogo;
double alpha = 0.3;
cv::Mat3b roi;
bool restore;
cv::Mat image;
int buttonWidth = 60;
int buttonHeight = 30;
string imagesSavingPath;
string framesFolderPath;
string framesSavingPath;

/* Image saving */
vector<int> compression_params;
string frameGrabbingSessionId;
bool createFrameGrabbingFolderPath = false;

/* Video writing */
string videoName;
cv::VideoWriter outputVideo;
string modeString;

struct modes currentMode = { false, false, false, false, false, false, false, false, false };

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
	bool pressedLeftArrow = GetKeyState(0x25);
	bool pressedUpArrow = GetKeyState(0x26);
	bool pressedRightArrow = GetKeyState(0x27);
	bool pressedDownArrow = GetKeyState(0x28);
	// Get current keyboard input
	keyboard::initKeyboard(keyboard::userKeys);
	// Copy previous iteration keys
	keyboard::updateKeys(keyboard::readKeys, keyboard::userKeys);
	// Clear command input with Q
	keyboard::clearCommands(keyboard::userKeys);
	// Check for input mode
	keyboard::pathInputModeKeyboard(keyboard::readKeys, keyboard::userKeys, &currentMode, &isPathInputModeEnabled);
	// Standard flow
	if (isPathInputModeEnabled)
	{
		keyboard::pathModeKeyboard(keyboard::readKeys, keyboard::userKeys, &currentMode, userPath);
	}
	else
	{
		if ((!outputVideo.isOpened() && isRecordingModeEnabled && !currentMode.stop && !currentMode.modeVideo) ||
			(!outputVideo.isOpened() && isRecordingModeEnabled && currentMode.modeVideo && currentMode.playVideo))
		{
			videoName = strhelp::createVideoName();
			const auto outputPath = videoSavingPath + videoName;
			const auto codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
			const auto size = cv::Size(320, 240);
			outputVideo.open(outputPath, codec, requestedFPS, size);
			modeString = "New file opened";
		}
		// Keyboard handler for recording state
		keyboard::recordingModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
									    &currentMode, &isRecordingModeEnabled, modeString,
										&outputVideo);
		// Keyboard handler for video state
		keyboard::videoModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
									&currentMode, &player::frameNum, &player::frameMax, \
									&player::playerSignal, userPath, modeString, \
									&outputVideo, &cap, &capWidth, &capHeight);
		// FrameGrabbing keyboard handler
		if (isFrameGrabbingModeEnabled && currentMode.modeVideo && !currentMode.playVideo)
		{
			currentMode.frameGrabbing = false;
			//TODO in GUI
			cout << "video not playing, cant save frames" << endl;
		}
		if (isFrameGrabbingModeEnabled && !currentMode.modeVideo ||
			isFrameGrabbingModeEnabled && currentMode.modeVideo && currentMode.playVideo)
		{
			currentMode.frameGrabbing = true;
			//TODO in GUI
			cout << "can save frames" << endl;
		}
		// Logo keyboard handler
		keyboard::logoModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
								   &currentMode, &moveDirection, \
								   &isLogoModeEnabled, &isMoveLogoModeEnabled, \
								   &logoX, &logoY);
		// Image handler
		currentMode.loadImage = isImageModeEnabled;
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

int frameCounter;

int main(int argc, char* argv[])
{
	//TODO: find more elegant way to determine path
	string argvStr(argv[0]);
	string base = argvStr.substr(0, argvStr.find_last_of("\\"));
	string base2 = base.substr(0, base.find_last_of("\\"));
	string base3 = base2.substr(0, base2.find_last_of("\\"));
	videoSavingPath = base3 + "\\FrameGrabber\\Video\\";
	framesFolderPath = base3 + "\\FrameGrabber\\Frames\\";
	imagesSavingPath = base3 + "\\FrameGrabber\\Images\\";

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

	/* Set compression parameters */
	compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
	compression_params.push_back(100);

	while (cv::waitKey(15) != char(27))
	{
		try
		{

			if (currentMode.modeVideo)
			{
				cap.set(cv::CAP_PROP_POS_FRAMES, player::frameNum);
				cap >> frame;
			}
			else if (currentMode.loadImage)
			{
				if (!isImageLoaded)
				{
					image = cv::imread(userPath);
					int tmpRatio = image.rows / image.cols;

					if (image.rows > frame.rows && image.cols > frame.cols)
					{
						cv::resize(image, image, cv::Size(image.cols / tmpRatio,frame.rows));
					}
				}
				frame = image;
			}
			else
			{
				cap >> frame;
			}

			frameCounter += 1;
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
			if (!isImageModeEnabled)
			{
				isRecordingModeEnabled = cvui::checkbox("Record straight to video file", &currentMode.recording);
				cvui::text("    Set FPS:");
				cvui::trackbar(menuWidth, &requestedFPS, 10, 100);

			}
				isPathInputModeEnabled = cvui::checkbox("Enable path input mode", &currentMode.pathInput);
			if (isPathInputModeEnabled)
			{
				//if (userChar != 0)
				//{
				//	userPath += userChar;
				//	userChar = 0;
				//}
				// TODO: Zapanowac nad menu +1
				int status;
				if (isImageModeEnabled)
				{
					status = cvui::iarea(menuPanelX + padding, margin + 4 * padding, menuWidth, padding);
					cvui::rect(gui, menuPanelX + padding, margin + 4 * padding, menuWidth, padding, 0x4d4d4d, 0x373737);
				}
				else
				{
					status = cvui::iarea(menuPanelX + padding, margin + 10.5 * padding, menuWidth, padding);
					cvui::rect(gui, menuPanelX + padding, margin + 10.5 * padding, menuWidth, padding, 0x4d4d4d, 0x373737);
				}
				const char *userPathC = userPath.c_str();
				cvui::text(userPathC);
				switch (status) {
				case cvui::CLICK:  std::cout << "Clicked!" << std::endl; break;
				case cvui::DOWN:   cvui::text("Mouse is: DOWN"); break;
				case cvui::OVER:   cvui::text("Mouse is: OVER"); break;
				case cvui::OUT:    cvui::text("Mouse is: OUT"); break;
				}
			}
			isLogoModeEnabled = cvui::checkbox("Put logo on", &currentMode.applyLogo);
			isImageModeEnabled = cvui::checkbox("Show image", &currentMode.loadImage);
			if (isImageModeEnabled)
			{
				saveToFile = cvui::button(buttonWidth, buttonHeight, "Save to file");
			}
			isFrameGrabbingModeEnabled = cvui::checkbox("Save video to files", &currentMode.frameGrabbing);
			cvui::endColumn();

			int firstPanelX = margin + padding + menuWidth + 2 * padding;
			int firstPanelY = margin;
			int firstPanelWidth = padding + capWidth + padding;
			int firstPanelHeight = margin + capHeight + 15 * padding;
			cvui::rect(gui, firstPanelX, firstPanelY, firstPanelWidth, firstPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, firstPanelX + padding, firstPanelY + padding, capWidth, firstPanelHeight, padding);
			cvui::image(frame);
			if (!isImageModeEnabled)
			{
				cvui::text("Frame track bar:");
        // Need to convert in that way, maybe put in the player to do it under the hood
			  cvui::trackbar(capWidth, &player::frameNum, player::frameMin, player::frameMax + (player::frameMax == 0 ? 1 : 0));
			}
			cvui::endColumn();

			// TODO: Pe�na obsluga drugiego okna
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

			if (!currentMode.frameGrabbing) {
				frameGrabbingSessionId = strhelp::generateRandomString(5);
				createFrameGrabbingFolderPath = true;
			}

			// TODO wiadomosc, obostrzenia i porzadek
			// TODO jesli video nie jest odtwarzane, wyswietl wiadomosc
			// TODO a jesli od 1 klatki? start zapisu video do pliku razem z zadaniem odtworzenia? 
			if (!currentMode.modeVideo && isFrameGrabbingModeEnabled ||
				currentMode.modeVideo && currentMode.playVideo && isFrameGrabbingModeEnabled)
			{
				if (createFrameGrabbingFolderPath)
				{
					framesSavingPath =  framesFolderPath + frameGrabbingSessionId;
					CreateDirectory(framesSavingPath.c_str(), NULL);
					createFrameGrabbingFolderPath = false;
				}

				if (isLogoModeEnabled)
				{
					cv::imwrite(framesSavingPath + "\\" + std::to_string(frameCounter) + ".jpg", frameWithLogo, compression_params);
				}
				else
				{
					cv::imwrite(framesSavingPath + "\\" + std::to_string(frameCounter) + ".jpg", frame, compression_params);
				}
			}

			if (saveToFile)
			{
				if (isLogoModeEnabled)
				{
					cv::imwrite(videoSavingPath + strhelp::generateRandomString(20) + ".jpg", frameWithLogo, compression_params);
				}
				else
				{
					cv::imwrite(videoSavingPath + strhelp::generateRandomString(20) + ".jpg", frame, compression_params);
				}
			}
			// Some video is opened right now
			if (currentMode.modeVideo)
			{
				if (currentMode.playVideo && currentMode.recording && player::frameNum <= player::frameMax && player::frameNum >= player::frameMin) // TODO: Mode logoMode?
				{
					outputVideo.write(frame);
				}
				printf("%d\n", player::frameNum);
				player::playerAction(&player::frameNum, player::playerSignal);
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
			getchar();
			return -1;
		}
	}

	exit(cap);
	return 0;
}