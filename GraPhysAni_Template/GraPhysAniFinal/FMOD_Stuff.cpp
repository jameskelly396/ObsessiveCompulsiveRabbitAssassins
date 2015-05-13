#include "FMOD_Stuff.h"

////#include <fmod.h>
//#include <fmod.hpp>
//#include <fmod_errors.h>
//#include <fmod_dsp.h>

bool InitFMOD( FMODInitParams &InitParams, std::wstring &error )
{
	//FMOD::System *system;
	FMOD_RESULT result;
	//unsigned int version;
	//int numdrivers;
	//FMOD_SPEAKERMODE speakermode;
	//FMOD_CAPS caps;
	char name[256];
	/*
	Create a System object and initialize.
	*/
	result = FMOD::System_Create( &(InitParams.system) );
	if ( !FMODERRORCHECK( result, error ) )	return false;

	result = InitParams.system->getVersion( &(InitParams.version) );
	if ( !FMODERRORCHECK( result, error ) )	return false;

	if ( InitParams.version < FMOD_VERSION )
	{
		std::wstringstream ss;
		ss << "Error! You are using an old version of FMOD " << InitParams.version 
			<< ". This program requires " << FMOD_VERSION;
		error = ss.str();
		return false;
	}

	result = InitParams.system->getNumDrivers( &(InitParams.numdrivers) );
	if ( !FMODERRORCHECK( result, error ) )	return false;

	if ( InitParams.numdrivers == 0 )
	{
		result = InitParams.system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		if ( !FMODERRORCHECK( result, error ) )	return false;
	}
	else
	{
		// ** There's an error in the documentation for this... **
		result = InitParams.system->getDriverCaps(0, &(InitParams.caps), 0, &(InitParams.speakermode) );
		if ( !FMODERRORCHECK( result, error ) )	return false;
		/*
		Set the user selected speaker mode.
		*/
		result = InitParams.system->setSpeakerMode( InitParams.speakermode );
		if ( !FMODERRORCHECK( result, error ) )	return false;

		if ( InitParams.caps & FMOD_CAPS_HARDWARE_EMULATED )
		{
			/*
			The user has the 'Acceleration' slider set to off! This is really bad
			for latency! You might want to warn the user about this.
			*/
			result = InitParams.system->setDSPBufferSize(1024, 10);
			if ( !FMODERRORCHECK( result, error ) )	return false;
		}
		result = InitParams.system->getDriverInfo(0, name, 256, 0);
		if ( !FMODERRORCHECK( result, error ) )	return false;

		if (strstr(name, "SigmaTel"))
		{
			/*
			Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
			PCM floating point output seems to solve it.
			*/
			result = InitParams.system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0,
			FMOD_DSP_RESAMPLER_LINEAR);
			if ( !FMODERRORCHECK( result, error ) )	return false;
		}
	}
	result = InitParams.system->init(100, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		/*
		Ok, the speaker mode selected isn't supported by this soundcard. Switch it
		back to stereo...
		*/
		result = InitParams.system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		if ( !FMODERRORCHECK( result, error ) )	return false;
		/*
		... and re-init.
		*/
		result = InitParams.system->init(100, FMOD_INIT_NORMAL, 0);
	}
	if ( !FMODERRORCHECK( result, error ) )	return false;

	// Copy name char array to InitParams wstring
	std::wstringstream ss;
	ss << name;
	InitParams.name = ss.str();

	// All is well
	return true;
}


//// This idiot thing is all over the documentation, but isn't explained...
//void ERRCHECK(FMOD_RESULT result)
//{
//    if (result != FMOD_OK)
//    {
//        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
//        exit(-1);
//    }
//}

bool FMODERRORCHECK( FMOD_RESULT result, std::wstring &errorDetails )
{
	if ( result != FMOD_OK )
	{
		std::wstringstream ss;
		ss << FMOD_ErrorString(result);
		errorDetails = ss.str();
		return false;
	}
	return true;
}

bool FMODERRORCHECK( FMOD_RESULT result )
{
	std::wstring error;
	return FMODERRORCHECK( result, error );
}