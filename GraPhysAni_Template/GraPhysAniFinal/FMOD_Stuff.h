#ifndef _FMOD_Stuff_HD_
#define _FMOD_Stuff_HD_

//#include <fmod.h>		// For C
#include <fmod.hpp>		// For C++ (includes C header, too)
#include <fmod_errors.h>
#include <fmod_dsp.h>

#include <string>
#include <sstream>

struct FMODInitParams
{
	FMOD::System* system;
	//FMOD_RESULT result;
	unsigned int version;
	int numdrivers;
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS caps;
	std::wstring name;	//char name[256];
};


bool FMODERRORCHECK( FMOD_RESULT result, std::wstring &errorDetails );
bool FMODERRORCHECK( FMOD_RESULT result );

bool InitFMOD( FMODInitParams &InitParams, std::wstring &error );


#endif