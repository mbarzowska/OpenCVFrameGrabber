/*
Definitions of modes used in program.
*/

#ifndef _MODES_H_
#define _MODES_H_

struct modes
{
	bool recording;
	bool stop;
	bool modeVideo;	// Open video mode, exit the mode pressing V
	bool playVideo; // Start/stop running of video
	bool pathInput; // Let get the path
};

#endif