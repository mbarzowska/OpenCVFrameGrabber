/*
Helper functions using strings:
+ Create file name for:
	- folder (timestamp)
	- video (timestamp + .avi)
	- frames (timestamp + .jpg)
*/

#ifndef _STRING_HELPER_H_
#define _STRING_HELPER_H_

#include <vector>
#include <string>
#include <windows.h>
// #include <iostream>
// #include <map>
// #include <stdarg.h>

SYSTEMTIME lt; // System local time

// Function definitions
namespace strhelp
{
	/**
	Create random string.

	@param len length of string
	*/
	std::string generateRandomString(int len);

	/**
	Create folder name using current local time.

	@param customName name added at the end of folder's name
	*/
	std::string createFolderName(const std::string& customName = "video");

	/**
	Create video name using current local time.

	@param format given format of video, default - avi
	*/
	std::string createVideoName(const std::string& format = "avi");

	/**
	Create frame name using current local time.

	@param frameNumber given frame "index"
	@param format given format of picture, default - jpg
	*/
	std::string createFrameName(long long int frameNumber, const std::string& format = "jpg");

	/**
	Make valid path.
	Ex:
	C:\\Folder\\20_6_2019_21h27m44s.avi

	@param _userPath string given by user 
	*/
	void makeValidPath(std::string& _userPath);

} // namespace strhelp

// Function implementations
namespace strhelp
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	int alphabetLength = sizeof(alphanum) - 1;

	std::string generateRandomString(int len)
	{
		srand(time(0));
		std::string str;
		for (unsigned int i = 0; i < len; ++i)
		{
			str += alphanum[rand() % alphabetLength];
		}
		return str;
	}

	std::string createFolderName(const std::string& customName)
	{
		// Get timestamp
		GetLocalTime(&lt);
		std::string folderName = std::to_string(lt.wDay) + "_" + \
			std::to_string(lt.wMonth) + "_" + \
			std::to_string(lt.wYear) + "_" + \
			std::to_string(lt.wHour) + "h" + \
			std::to_string(lt.wMinute) + "m" + \
			std::to_string(lt.wSecond) + "s" + "_" + customName;
		return folderName;
	}

	std::string createVideoName(const std::string& format)
	{
		// Get timestamp
		GetLocalTime(&lt);
		std::string videoName = std::to_string(lt.wDay) + "_" + \
			std::to_string(lt.wMonth) + "_" + \
			std::to_string(lt.wYear) + "_" + \
			std::to_string(lt.wHour) + "h" + \
			std::to_string(lt.wMinute) + "m" + \
			std::to_string(lt.wSecond) + "s" + "." + format;
		return videoName;
	}

	std::string createFrameName(long long int frameNumber, const std::string& format)
	{
		// Get timestamp
		GetLocalTime(&lt);
		std::string videoName = std::to_string(frameNumber) + "." + format;
		return videoName;
	}

	void makeValidPath(std::string& _userPath)
	{
		std::replace(_userPath.begin(), _userPath.end(), '\\', '/');
		return;
	}

}

#endif