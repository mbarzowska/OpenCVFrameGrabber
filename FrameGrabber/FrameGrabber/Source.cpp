#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <regex>
#include "HeaderFiles/Modes.h"
#include "HeaderFiles/StringHelper.h"
#include "HeaderFiles/Player.h"
#include "HeaderFiles/Keyboard.h"

using namespace std;

#define CVUI_IMPLEMENTATION
#include "HeaderFiles/cvui.h"
#define CVUI_WINDOW_NAME "Frame grabber"

/* cvUI	related	*/
/* aesthetic	*/ int margin = 20, padding = 20;
/* aesthetic	*/ int offsetX = 6, offsetY = 4;
/* aesthetic	*/ int buttonWidth = 60, buttonHeight = 30;
/* alerts		*/ string secondPanelLogoAlertString = "", secondPanelLogoAdditionString = "";
/* mode flags	*/ bool isLogoMovingMessedUp;
/* mode flags	*/ bool isImageLoaded;
/* on buttons	*/ bool saveFrameToFile;
/* on trackbars	*/ int requestedFPS = 20;

/* Matrices	*/
cv::Mat frame;
cv::Mat firstFrame;
cv::Mat lastFrame;
cv::Mat frameWithLogo;
cv::Mat image;
cv::Mat3b roi;
cv::Mat4b logo;

/* VideoCapture	*/
cv::VideoCapture cap;
double capWidth = 0.0, capHeight = 0.0;

/* Logo */
bool restore;
double alpha = 0.3;
int logoX = 0, logoY = 0;

/* User paths */
char userChar = 0;
string userPathFrames = "";
string userPathImage = "";
string userPathLogo = "";
string userPathVideo = "";

/* Frame grabbing strings and ints */
string strFrameBasedStart = "0";
string strFrameBasedQuantity = "0";
string strTimeBasedStart = "0";
string strTimeBasedQuantity = "0";
long long int valFrameBasedStart = 0;
long long int valFrameBasedQuantity = 0;
long long int valTimeBasedStart = 0;
long long int valTimeBasedQuantity = 0;

/* Frame reading */
bool isFramesFolderRead;
string frameFolder; 
string framesFolderPath;

/* Frame / image saving */
bool createFrameGrabbingFolderPath = false;
string imagesSavingPath;
string framesSavingPath;
string frameGrabbingSessionId;
vector<int> compression_params;

/* Video writing */
cv::VideoWriter outputVideo;
string videoName;
string videoSavingPath;

/* Alert */
string modeString;


struct modes currentMode = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

struct logoMoveDirections moveDirection = { false, false, false, false };

inline void openCamera()
{
	/* Open a camera for video capturing */
	try
	{
		cap.open(0);
		/* Set properties */
		capWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
		capHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
		cap.set(cv::CAP_PROP_FRAME_WIDTH, capWidth / 2);
		cap.set(cv::CAP_PROP_FRAME_HEIGHT, capHeight / 2);
		capWidth = capWidth / 2;
		capHeight = capHeight / 2;
	}
	catch (cv::Exception &e)
	{
		modeString += "Can't open the camera! ";
	}
}

void modeUpdate(int requestedFPS)
{
	// Get info about mode
	if (currentMode.modeVideo)
	{
		modeString = "VIDEO | ";
		if (currentMode.playVideo)
			modeString += "Playing video. ";
		else
			modeString += "Stopped video. ";
		if (currentMode.recording)
			modeString += "Recording from video. ";
	}
	if (!currentMode.modeVideo)
	{
		modeString = "CAMERA | ";
		if (currentMode.recording)
			modeString += "Recording from camera. ";
	}
	if (currentMode.loadImage)
		modeString = "IMAGE | ";
	if (currentMode.pathInput)
		modeString = "PATH INPUT | ";
	// Get current keyboard input
	keyboard::initKeyboard(keyboard::userKeys);
	// Copy previous iteration keys
	keyboard::updateKeys(keyboard::readKeys, keyboard::userKeys);
	// Update Command String
	keyboard::updateCommandString(keyboard::userKeys, keyboard::currentCommand);
	// Clear command input with Q
	keyboard::clearCommands(keyboard::readKeys, keyboard::userKeys);
	// Check for input mode
	keyboard::pathInputModeKeyboard(keyboard::readKeys, keyboard::userKeys, &currentMode);
	// Standard flow
	if (currentMode.pathInput)
	{
		keyboard::pathModeKeyboard(keyboard::readKeys, keyboard::userKeys, &currentMode, \
			userPathVideo, userPathFrames, userPathImage, userPathLogo);
	}
	else
	{
		if ((!outputVideo.isOpened() && currentMode.recording && !currentMode.modeVideo) ||
			(!outputVideo.isOpened() && currentMode.recording && currentMode.modeVideo && currentMode.playVideo))
		{
			videoName = strhelp::createVideoName();
			const auto outputPath = videoSavingPath + videoName;
			const auto codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
			const auto size = cv::Size(320, 240);
			try
			{
				outputVideo.open(outputPath, codec, requestedFPS, size);
			}
			catch (cv::Exception &e)
			{
				modeString += "Can't open video file! ";
			}
		}
		// Keyboard handler for recording state
		keyboard::recordingModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
			&currentMode, modeString, &outputVideo);
		// Keyboard handler for video state
		keyboard::videoModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
			&currentMode, &player::frameNum, &player::frameMax, \
			&player::playerSignal, userPathVideo, modeString, \
			&outputVideo, &cap, &capWidth, &capHeight, &firstFrame, &lastFrame);
		// FrameGrabbing handler
		if (currentMode.frameGrabbingOnDemand && currentMode.modeVideo && !currentMode.playVideo)
		{
			currentMode.frameGrabbingOnDemand = false;
			modeString += "Video is not playing, can't save frames. ";
		}
		if (currentMode.frameGrabbingOnDemand && !currentMode.modeVideo ||
			currentMode.frameGrabbingOnDemand && currentMode.modeVideo && currentMode.playVideo)
		{
			modeString += "Saving frames. ";
		}
		// Logo keyboard handler
		keyboard::logoModeKeyboard(keyboard::readKeys, keyboard::userKeys, \
			&currentMode, &moveDirection, \
			&logoX, &logoY);
	}
	// Input for the input paths and FrameGrabbing
	keyboard::readRestKeyboard(keyboard::readKeys, keyboard::userKeys, &currentMode, \
		userPathVideo, userPathFrames, userPathImage, userPathLogo, \
		strFrameBasedStart, strFrameBasedQuantity, \
		strTimeBasedStart, strTimeBasedQuantity);
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

int frameCounter = 0;

std::vector<string> framesToJoin;

void readDirectory(const std::string& name, std::vector<string>& v)
{
	std::string pattern(name);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {
			if (regex_search(data.cFileName, std::regex("[0-9]+\.jpg")) == 1)
				v.emplace_back(data.cFileName);
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
}

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
	int menuWidth = 220;
	cvui::init(CVUI_WINDOW_NAME);
	int cvUIWindowWidth = margin + padding + menuWidth + 3 * padding + capWidth + 3 * padding + capWidth + padding + margin;
	int cvUIWindowHeight = 750;
	cv::Mat gui = cv::Mat(cv::Size(cvUIWindowWidth, cvUIWindowHeight), CV_8UC3);
	gui = cv::Scalar(55, 55, 55);

	/* Set compression parameters */
	compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
	compression_params.push_back(100);

	while (cv::waitKey(15) != char(27))
	{
		try
		{
			if (currentMode.movieMaker)
			{
				isImageLoaded = false;
				currentMode = { false, currentMode.modeVideo, \
					false, false, currentMode.applyLogo, \
					false, currentMode.loadImage, false, \
					false, false, false, false, \
					currentMode.previousSceneRequest, currentMode.nextSceneRequest, \
					currentMode.frameGrabbingFrameBased, currentMode.frameGrabbingTimeBased, currentMode.movieMaker, \
					false, false, false, false };

				if (!isFramesFolderRead)
				{
					frameFolder = userPathFrames;
					readDirectory(frameFolder, framesToJoin);
					if (framesToJoin.empty())
					{
						currentMode.movieMaker = false;
						modeString += "No files found. ";
					}
					else
					{
						isFramesFolderRead = true;
						outputVideo.open(
							videoSavingPath + strhelp::createVideoName(),
							cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
							requestedFPS,
							cv::Size(320, 240));
					}
				}

				if (isFramesFolderRead)
				{
					for (int i = 0; i < framesToJoin.size(); i++)
					{
						cv::Mat tmpFrame = cv::imread(frameFolder + "\\" + framesToJoin[i]);
						outputVideo.write(tmpFrame);
					}
					outputVideo.release();
					currentMode.movieMaker = false;
					currentMode.pathFramesInput = false;
					currentMode.pathInput = false;
					isFramesFolderRead = false;
					userPathFrames.clear();
					framesToJoin.clear();
				}
			}
			else
			{
				if (currentMode.loadImage)
				{
					if (!isImageLoaded)
					{
						image = cv::imread(userPathImage);
						if (image.rows == 0 || image.cols == 0)
						{
							modeString += "Can't load the image! ";
						}
						else
						{
							int tmpRatio = image.rows / image.cols;
							if (image.rows > frame.rows && image.cols > frame.cols)
							{
								cv::resize(image, image, cv::Size(frame.cols, frame.cols * tmpRatio));
							}
							isImageLoaded = true;
							frame = image;
						}
					}
				}
				else if (currentMode.modeVideo)
				{
					isImageLoaded = false;
					cap.set(cv::CAP_PROP_POS_FRAMES, player::frameNum);
					cap >> frame;
				}
				else
				{
					isImageLoaded = false;
					cap >> frame;
				}
			}
			// For scene modes ifs
			if (currentMode.previousSceneRequest)
			{
				firstFrame.copyTo(frameWithLogo);
			}
			else if (currentMode.nextSceneRequest)
			{
				lastFrame.copyTo(frameWithLogo);
			}
			else
			{
				frame.copyTo(frameWithLogo);
			}
			// Counter for frames saved TODO: restore 0 at new film etc add only if needed!!!!!!!!
			frameCounter += 1;

			if (currentMode.applyLogo)
			{
				try
				{
					logo = cv::imread(userPathLogo, cv::IMREAD_UNCHANGED);
					resize(logo, logo, cv::Size(64, 64));
				}
				catch (cv::Exception &e)
				{
					modeString += "Reading logo error! ";
				}
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
					secondPanelLogoAlertString = "Can't move logo any further!";
					secondPanelLogoAdditionString = "Restore previous position to continue.";
				}
			}

			/* Set cvUI window */
			int menuPanelX = margin;
			int menuPanelY = margin;
			int menuPanelWidth = padding + menuWidth + padding;
			int menuPanelHeight = margin + capHeight + 12 * padding;
			cvui::rect(gui, menuPanelX, menuPanelY, menuPanelWidth, menuPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, menuPanelX + padding, menuPanelY + padding, menuWidth, menuPanelHeight, padding / 2);
			cvui::text("Current command:");
			cvui::text(keyboard::currentCommand);
			cvui::text("Menu");
			currentMode.applyLogo = cvui::checkbox("Put logo on", &currentMode.applyLogo);
			if (!currentMode.loadImage)
			{
				currentMode.recording = cvui::checkbox("Record straight to video file", &currentMode.recording);
				cvui::text("Set FPS:");
				cvui::trackbar(menuWidth, &requestedFPS, 10, 100);
				cvui::text("Save video to frames:");
				currentMode.frameGrabbingOnDemand = cvui::checkbox("On demand (must be running)", &currentMode.frameGrabbingOnDemand);
				if (currentMode.modeVideo)
				{
					// TODO
					currentMode.frameGrabbingFrameBased = cvui::checkbox("Given frames", &currentMode.frameGrabbingFrameBased);
					if (currentMode.frameGrabbingFrameBased)
					{
						if (currentMode.frameGrabbingTimeBased)
						{
							currentMode.frameGrabbingTimeBased = false;
							currentMode.timeBasedStartPointInput = false;
							currentMode.timeBasedQuantityInput = false;
						}

						currentMode.frameBasedStartPointInput = cvui::checkbox("Start point: ", &currentMode.frameBasedStartPointInput);

						cvui::rect(gui, margin + padding + 95, 293, menuWidth - 95, padding, 0x4d4d4d, 0x373737);
						const char *strFrameBasedStartC = strFrameBasedStart.c_str();
						cvui::printf(gui, margin + padding + 95 + offsetX, 293 + offsetY, strFrameBasedStartC);

						currentMode.frameBasedQuantityInput = cvui::checkbox("Quantity: ", &currentMode.frameBasedQuantityInput);

						cvui::rect(gui, margin + padding + 85, 318, menuWidth - 85, padding, 0x4d4d4d, 0x373737);
						const char *strFrameBasedQuantityC = strFrameBasedQuantity.c_str();
						cvui::printf(gui, margin + padding + 85 + offsetX, 318 + offsetY, strFrameBasedQuantityC);
					}

					currentMode.frameGrabbingTimeBased = cvui::checkbox("Given time", &currentMode.frameGrabbingTimeBased);
					if (currentMode.frameGrabbingTimeBased)
					{
						if (currentMode.frameGrabbingFrameBased)
						{
							currentMode.frameGrabbingFrameBased = false;
							currentMode.frameBasedStartPointInput = false;
							currentMode.frameBasedQuantityInput = false;
						}

						currentMode.timeBasedStartPointInput = cvui::checkbox("Start point: ", &currentMode.timeBasedStartPointInput);

						cvui::rect(gui, margin + padding + 95, 318, menuWidth - 95, padding, 0x4d4d4d, 0x373737);
						const char *strTimeBasedStartC = strTimeBasedStart.c_str();
						cvui::printf(gui, margin + padding + 95 + offsetX, 318 + offsetY, strTimeBasedStartC);

						currentMode.timeBasedQuantityInput = cvui::checkbox("Quantity: ", &currentMode.timeBasedQuantityInput);

						cvui::rect(gui, margin + padding + 85, 343, menuWidth - 85, padding, 0x4d4d4d, 0x373737);
						const char *strTimeBasedQuantityC = strTimeBasedQuantity.c_str();
						cvui::printf(gui, margin + padding + 85 + offsetX, 343 + offsetY, strTimeBasedQuantityC);
					}
				}
			}
			saveFrameToFile = cvui::button(buttonWidth * 2.5, buttonHeight, "Save current to file");
			cvui::space(padding);
			if (!userPathImage.empty() || isImageLoaded)
			{
				currentMode.loadImage = cvui::checkbox("Show image", &currentMode.loadImage);
				if (currentMode.loadImage == false)
				{
					isImageLoaded = false;
				}
			}
			if (!userPathFrames.empty())
			{
				currentMode.movieMaker = cvui::checkbox("Join frames to video", &currentMode.movieMaker);
			}
			cvui::endColumn();

			int firstPanelX = margin + padding + menuWidth + 2 * padding;
			int firstPanelY = margin;
			int firstPanelWidth = padding + capWidth + padding;
			int firstPanelHeight = margin + capHeight + 12 * padding;
			cvui::rect(gui, firstPanelX, firstPanelY, firstPanelWidth, firstPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, firstPanelX + padding, firstPanelY + padding, capWidth, firstPanelHeight - padding, padding);
			// Left image
			try
			{
				cvui::image(frame);
			}
			catch (cv::Exception &e)
			{
				modeString += "Unable to load left frame! ";
			}
			if (currentMode.modeVideo && !(currentMode.loadImage))
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
			int secondPanelHeight = padding + capHeight + 12 * padding;
			cvui::rect(gui, secondPanelX, secondPanelY, secondPanelWidth, secondPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, secondPanelX + padding, secondPanelY + padding, capWidth, secondPanelHeight - padding, padding);
			if (currentMode.applyLogo || currentMode.previousSceneRequest || currentMode.nextSceneRequest)
			{
				try
				{
					cvui::image(frameWithLogo);
				}
				catch (cv::Exception &e)
				{
					modeString += "Unable to load right frame! ";
				}
			}
			if (currentMode.applyLogo)
			{
				// Right image
				currentMode.moveLogo = cvui::checkbox("Enable logo move", &currentMode.moveLogo);
				if (currentMode.moveLogo)
				{
					if (isLogoMovingMessedUp)
					{
						cvui::text(secondPanelLogoAlertString);
						cvui::text(secondPanelLogoAdditionString);
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

			int pathPanelX = menuPanelX;
			int pathPanelY = menuPanelY + menuPanelHeight + padding;
			int pathPanelWidth = cvUIWindowWidth - 2 * margin;
			int pathPanelHeight = cvUIWindowHeight - margin - menuPanelHeight - padding - 2 * padding - margin;
			cvui::rect(gui, pathPanelX, pathPanelY, pathPanelWidth, pathPanelHeight, 0x454545, 0x454545);
			cvui::beginColumn(gui, pathPanelX + padding, pathPanelY + padding, pathPanelWidth - 2 * padding, pathPanelHeight - padding, padding / 2);
			currentMode.pathInput = cvui::checkbox("Enable path input mode", &currentMode.pathInput);
			// Start input section
			int pathAreaHeight = padding;
			// if path to video mode
			int pathToVideoAreaX = pathPanelX + 7 * padding;
			int pathToVideoAreaY = pathPanelY + 2 * padding;
			int pathToVideoAreaWidth = pathPanelWidth - 8 * padding;
			currentMode.pathVideoInput = cvui::checkbox("Path to video: ", &currentMode.pathVideoInput);
			cvui::iarea(pathToVideoAreaX, pathToVideoAreaY, pathToVideoAreaWidth, pathAreaHeight);
			cvui::rect(gui, pathToVideoAreaX, pathToVideoAreaY, pathToVideoAreaWidth, pathAreaHeight, 0x4d4d4d, 0x373737);
			const char *userPathVideoC = userPathVideo.c_str();
			cvui::printf(gui, pathToVideoAreaX + offsetX, pathToVideoAreaY + offsetY, userPathVideoC);
			// if path to frames
			int pathToFramesAreaX = pathPanelX + 9.5 * padding;
			int pathToFramesAreaY = pathPanelY + 3.3 * padding;
			int pathToFramesAreaWidth = pathPanelWidth - 10.5 * padding;
			currentMode.pathFramesInput = cvui::checkbox("Path to frames folder: ", &currentMode.pathFramesInput);
			cvui::iarea(pathToFramesAreaX, pathToFramesAreaY, pathToFramesAreaWidth, pathAreaHeight);
			cvui::rect(gui, pathToFramesAreaX, pathToFramesAreaY, pathToFramesAreaWidth, pathAreaHeight, 0x4d4d4d, 0x373737);
			const char *userPathFramesC = userPathFrames.c_str();
			cvui::printf(gui, pathToFramesAreaX + offsetX, pathToFramesAreaY + offsetY, userPathFramesC);
			// if path to image
			int pathToImageAreaX = pathPanelX + 7 * padding;
			int pathToImageAreaY = pathPanelY + 4.6 * padding;
			int pathToImageAreaWidth = pathPanelWidth - 8 * padding;
			currentMode.pathImageInput = cvui::checkbox("Path to image: ", &currentMode.pathImageInput);
			cvui::iarea(pathToImageAreaX, pathToImageAreaY, pathToImageAreaWidth, pathAreaHeight);
			cvui::rect(gui, pathToImageAreaX, pathToImageAreaY, pathToImageAreaWidth, pathAreaHeight, 0x4d4d4d, 0x373737);
			const char *userPathImageC = userPathImage.c_str();
			cvui::printf(gui, pathToImageAreaX + offsetX, pathToImageAreaY + offsetY, userPathImageC);
			// if path to logo
			int pathToLogoAreaX = pathPanelX + 6.5 * padding;
			int pathToLogoAreaY = pathPanelY + 5.9 * padding;
			int pathToLogoAreaWidth = pathPanelWidth - 7.5 * padding;
			currentMode.pathLogoInput = cvui::checkbox("Path to logo: ", &currentMode.pathLogoInput);
			cvui::iarea(pathToLogoAreaX, pathToLogoAreaY, pathToLogoAreaWidth, pathAreaHeight);
			cvui::rect(gui, pathToLogoAreaX, pathToLogoAreaY, pathToLogoAreaWidth, pathAreaHeight, 0x4d4d4d, 0x373737);
			const char *userPathLogoC = userPathLogo.c_str();
			cvui::printf(gui, pathToLogoAreaX + offsetX, pathToLogoAreaY + offsetY, userPathLogoC);

			/*
			if (currentMode.pathInput)
			{
				//if (userChar != 0)
				//{
				//	userPath += userChar;
				//	userChar = 0;
				//}
				const char *userPathC = userPath.c_str();
				cvui::text(userPathC);
			}
			*/
			cvui::endColumn();

			/* Error message section */
			int alertPanelX = menuPanelX;
			int alertPanelY = cvUIWindowHeight - padding - margin;
			int alertPanelWidth = cvUIWindowWidth - 2 * margin;
			int alertPanelHeight = 1.5 * padding;
			cvui::rect(gui, alertPanelX, alertPanelY, alertPanelWidth, alertPanelHeight, 0xDC4343, 0x696969); // 0xDC4343
			cvui::beginColumn(gui, alertPanelX + padding, alertPanelY + padding / 2, alertPanelWidth - 2 * padding, alertPanelHeight - padding, padding / 2);

			cvui::text(modeString);

			cvui::endColumn();

			cvui::imshow(CVUI_WINDOW_NAME, gui);

			modeUpdate(requestedFPS);

			if (saveFrameToFile)
			{
				try
				{
					if (currentMode.applyLogo) { cv::imwrite(imagesSavingPath + strhelp::generateRandomString(20) + ".jpg", frameWithLogo, compression_params); }
					else { cv::imwrite(imagesSavingPath + strhelp::generateRandomString(20) + ".jpg", frame, compression_params); }
				}
				catch (cv::Exception &e)
				{
					modeString += "Error while saving to image! ";
				}
			}

			if (!currentMode.frameGrabbingOnDemand)
			{
				frameGrabbingSessionId = strhelp::generateRandomString(5);
				createFrameGrabbingFolderPath = true;
			}

			if (currentMode.modeVideo)
			{
				// TODO zapetlanie filmu na zadanie (continuous loop)?
				// TODO wznowienie filmu na przycisk, pauza na przycisk?
				// TODO jesli zadanie nagraj od 1 klatki to 
				// TODO		wykorzystanie nagrywania przedzialu klatkowego + pokazanie tego w 2 oknie
				// TODO		przedzial czasowy nagrywania (czas * FPS = klatki)
				if (currentMode.recording && currentMode.playVideo && player::frameNum <= player::frameMax && player::frameNum >= player::frameMin) {
					try
					{
						if (currentMode.applyLogo) { outputVideo.write(frameWithLogo); }
						else { outputVideo.write(frame); }
					}
					catch (cv::Exception &e)
					{
						modeString += "Error while recording! ";
					}
				}
				// TODO wiadomosc, obostrzenia i porzadek, jesli video nie jest odtwarzane, wyswietl wiadomosc
				if (currentMode.frameGrabbingOnDemand && currentMode.playVideo)
				{
					try
					{
						if (createFrameGrabbingFolderPath)
						{
							framesSavingPath = framesFolderPath + frameGrabbingSessionId;
							CreateDirectory(framesSavingPath.c_str(), NULL);
							createFrameGrabbingFolderPath = false;
						}

						if (currentMode.applyLogo)
						{
							cv::imwrite(framesSavingPath + "\\" + std::to_string(frameCounter) + ".jpg", frameWithLogo, compression_params);
						}
						else
						{
							cv::imwrite(framesSavingPath + "\\" + std::to_string(frameCounter) + ".jpg", frame, compression_params);
						}
					}
					catch (cv::Exception &e)
					{
						modeString += "Error while frame grabbing! ";
					}
				}
				if (currentMode.previousSceneRequest)
				{
					// User new signal
					if (!(player::playerSignal == PLAYER_STANDARD))
						player::playerAction(&player::frameNum, player::playerSignal);
					// Old signal restored if not paused
					if (!(player::playerSignal == PLAYER_NONE))
						player::playerAction(&player::frameNum, PLAYER_SCENE_L);
				}
				else if (currentMode.nextSceneRequest)
				{
					// User new signal
					// Don't allow PLAYER_STANDARD
					if (!(player::playerSignal == PLAYER_STANDARD))
						player::playerAction(&player::frameNum, player::playerSignal);
					// Old signal restored if not paused
					if (!(player::playerSignal == PLAYER_NONE))
						player::playerAction(&player::frameNum, PLAYER_SCENE_R);
				}
				else
				{
					player::playerAction(&player::frameNum, player::playerSignal);
				}
			}

			if (!currentMode.modeVideo && !currentMode.loadImage)
			{
				try
				{
					if (currentMode.recording) {
						if (currentMode.applyLogo) { outputVideo.write(frameWithLogo); }
						else { outputVideo.write(frame); }
					}
				}
				catch (cv::Exception &e)
				{
					modeString += "Error while recording! ";
				}
				try
				{
					// TODO wiadomosc, obostrzenia i porzadek, jesli video nie jest odtwarzane, wyswietl wiadomosc
					if (currentMode.frameGrabbingOnDemand)
					{
						if (createFrameGrabbingFolderPath)
						{
							framesSavingPath = framesFolderPath + frameGrabbingSessionId;
							CreateDirectory(framesSavingPath.c_str(), NULL);
							createFrameGrabbingFolderPath = false;
						}
						if (currentMode.applyLogo) { cv::imwrite(framesSavingPath + "\\" + std::to_string(frameCounter) + ".jpg", frameWithLogo, compression_params); }
						else { cv::imwrite(framesSavingPath + "\\" + std::to_string(frameCounter) + ".jpg", frame, compression_params); }
					}
				}
				catch (cv::Exception &e)
				{
					modeString += "Error while frame grabbing! ";
				}
			}

			// if (currentMode.loadImage)
			// TODO: zablokowac skroty klawiszowe jesli nie sa zablokowane! 

			//TODO: Eventually get rid of it
			cout << modeString << endl;
		}
		catch (cv::Exception &e)
		{
			modeString += "Some error(s) has occured. ";
			_getch();
			exit(cap);
			getchar();
			return -1;
		}
	}

	exit(cap);
	return 0;
}