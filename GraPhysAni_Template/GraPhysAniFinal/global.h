#ifndef _global_HG_
#define _global_HG_

//#include "Utils.h"		// Contains Vertex struct, but no longer neededd
#include <time.h>			// Only needed for the "clock_t g_LastTime" variable

#define GLM_FORCE_CXX98    
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
	
//#include "CGameObjects/CGameObject.h"
//#include "CGameObjects/IGameObject.h"

//#include "CTriangle.h"		
#include "CFactoryMediator.h"	
#include "CModelLoaderManager.h"

#include "CTexture/CTextureFromBMP.h"

#include "CCommonTypes/CHRTimer.h"

#include "CPhysics/CPhysicsCalculatron.h"		// Added November 13, 2014

// Added October 3, 2014
#include "CCamera.h"

#include "CMouseState.h"

#include "ShaderUniformVariables.h"		

#include "CRender/CLightManager.h"

#include "CTriangle.h"	

#include "CAABB.h"
// ************************
#include "CParticleEmitter.h"		// ** NEW **

#include "CGameController/CGameControllerManager.h"

#include "CRender/IGLShaderManager.h"

#include "CTexture/CTextureManager.h"

#include "FMOD_Stuff.h"

struct Light
{
public:
	Light() :
		type(CLight::POINT),
		pos(CVector3f(0.0f, 0.0f, 0.0f)),
		col(CVector3f(1.0f, 1.0f, 1.0f)),
		constAtten(0.48f),
		linAtten(0.0105f),
		quadAtten(0.00039f),
		attenRangeStart(100.0f),
		attenRangeEnd(0.001f)
	{
	}

	CLight::enumLightType type;
	CVector3f pos;
	CVector3f col;
	float constAtten;
	float linAtten;
	float quadAtten;
	float attenRangeStart;
	float attenRangeEnd;
};

struct Model
{
public:
	Model() :
		ID(0),
		type(""),
		file(""),
		tex(0),
		rot(CVector3f(0.0f, 0.0f, 0.0f)),
		pos(CVector3f(0.0f, 0.0f, 0.0f)),
		col(CVector3f(1.0f, 1.0f, 1.0f)),
		scale(1.0f),
		isPlayer(0),
		isSkybox(0),
		isEnvironment(0),
		isTransparent(0),
		isParticle(0),
		isMaze(0),
		isMob(0),
		transparency(0),
		blend(0),
		firstTex(0)
	{}

	unsigned int ID;
	std::string type;
	std::string file;
	std::vector<std::vector<std::string> > tex;
	CVector3f rot;  //rotation
	CVector3f pos;  //position
	CVector3f col;  //color
	float scale;
	float transparency;
	bool isPlayer;
	bool isSkybox;
	bool isEnvironment;
	bool isLightBall;
	bool isTransparent;
	bool isParticle;
	bool isMaze;
	bool isMob;
	unsigned int blend;
	unsigned int firstTex;
};

extern FMODInitParams g_FMODInitparams;
extern std::map<std::string, FMOD::Sound*> g_Sounds;
extern std::map<std::string, FMOD::Channel*> g_channels;
extern FMOD_RESULT g_FMODResult;

extern GLubyte  world[100][50][100];

extern CGameControllerManager*	g_p_GameControllerManager;
extern IGameController*			g_p_GameController_0;

extern CPhysicsCalculatron*		g_p_PhysicsThingy;

extern CParticleEmitter* g_p_myEmitter;	

extern CLightManager* g_p_LightManager;
extern bool isFocusedOnLight;

extern CTextureManager* g_p_TextureManager;

// ************************

extern CShaderUniformVariables g_ShaderUniformVariables;

extern IGLShaderManager* g_pShaderManager;

	// TEXTURE IDs
//extern GLuint g_texture_0_ID;
//extern GLuint g_texture_1_ID;
//extern GLuint g_texture_skymap;

extern GLuint g_sampler_0;
extern GLuint g_sampler_1;
extern GLuint g_sampler_skymap;

// Some texturing stuff:
//extern CTextureFromBMP* g_p_Texture00;
//extern CTextureFromBMP* g_p_Texture01;
//extern CTextureFromBMP* g_p_TextureSkyMap;

extern float g_MyMixValue;

extern std::map<std::string, std::string> g_mapModelTypes;
extern std::map<unsigned int, std::vector<std::string> > g_mapTextures;

//extern glm::mat4 g_matProjection;
//extern glm::mat4 g_matView;
//extern glm::mat4 g_matWorld;		// aka "Model" matrix

//extern GLuint g_ProjectionMatrixUniformLocation;			
//extern GLuint g_ViewMatrixUniformLocation;				
//extern GLuint g_ModelMatrixUniformLocation;				
//extern GLuint g_ObjectColourUniformLocation;				

extern GLuint g_slot_LightPosition;	//uniform vec4 LightPosition;	
extern GLuint g_slot_LightColour;	//uniform vec4 LightColour;		
extern GLuint g_slot_attenuation;	//uniform float attenuation;	

extern CCamera* g_pCamera;

extern CMouseState* g_pMouseState;


extern CModelLoaderManager*  g_pModelLoader;	// "p" for pointer

extern CFactoryMediator* g_pFactoryMediator;			

extern unsigned int g_Galactica_ID;
extern unsigned int g_Player_ID;				// Used to locate the player
extern unsigned int g_lightModelID;
extern unsigned int g_skyBoxID;
extern unsigned int g_enviroID;
extern unsigned int g_ParticleID;
extern unsigned int g_mazeID;
extern std::vector<unsigned int> g_mobID;
extern std::vector<unsigned int> g_vecMazeIDs;
extern std::map< unsigned int, CAABB* > g_mapCAABB;
extern bool bIsUnderground;

extern bool drawHitbox;
extern bool drawParticles;

extern std::vector<Model> vecModelsInfo;      //list of all the loaded objects
extern std::vector<Light> vecLightsInfo;

extern std::map< unsigned int, CAABB* > g_mapCAABB;

extern GLuint g_shadow_depthTexture_FrameBufferObjectID;
extern GLuint g_shadow_depthTexture_ID;
static const unsigned int SHADOW_DEPTH_TEXTURE_SIZE = 4096;

extern bool g_SavingData;

extern CHRTimer g_simTimer;

// *****************************
// *** For the Physics demo ****
// These are the "debug" objects (the balls, etc.) that we are also drawing
extern std::vector< CGameObject* > g_vecPhysDebugObjects;
extern std::vector< CGameObject* > g_vecShipHitBoxes;
// *****************************


// Same sort of thing as the "name" of the buffer, but these are the 
// "names" of the shaders we are loading
//extern GLuint g_ShaderIds[3];// = { 0 };		
// or these might be a little more clear...
extern GLuint g_ShaderProgram_ID;
extern GLuint g_VertexShader_ID;
extern GLuint g_FragmentShader_ID;

extern int g_screenWidth;	// = 1400;
extern int g_screenHeight;	// = 900;
extern int g_windowHandle;	// = 0;
extern unsigned int g_FrameCount;	// = 0;
extern clock_t g_LastTime;		// = 0;		

// Since this is static (i.e. there's only one of these), so we don't need "extern"
static const std::string g_WindowTitleDefault = "OpenGL for the win!";

// bool SetShaderUniformVariables(void);		// Moved to ShaderUniformVariables.h

bool LoadPlyFileIntoGLBuffers( std::string plyFile,
							   GLuint &vertexBufferID, 
							   GLuint &vertexColourBufferID, 
							   GLuint &indexBufferID, 
							   int &numberOfVerticesLoaded, 
							   int &numberOfTrianglesLoaded, 
							   float &maxExtent );


// ***********************************************
// *** WE'LL BE MOVING THIS STUFF OUT OF HERE ****
//extern int g_numberOfTriangles[2]; 
//extern GLuint g_VertexPositionBufferID[2];			// BufferIds[0]		
//extern GLuint g_VertexColourBufferID[2];			// BufferIds[1]		
//extern GLuint g_IndexBufferID[2];					// BufferIds[2]		
// ***********************************************

bool OpenGL_Initialize(int argc, char* argv[], int screenWidth, int screenHeight);

// Loads the "whats_this.txt" file and displays it in the console
void printTheWhatsThisProgramAboutBlurb(void);

// OpenGL callbacks
void glutReshape_callback(int Width, int Height);		// void ResizeFunction(int, int);
void glutDisplay_callback(void);						// void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);
void glutClose_callback(void);							// void DestroyCube(void);

// ADDED in class on September 19, 2014
void glutKeyboard_callback(unsigned char key, int x, int y);	// void keyboardFunctionCallback(unsigned char key, int x, int y);
void glutSpecialKey_callback( int key, int x, int y );			// void specialKeyCallback( int key, int x, int y );

void glutMouseCallback(int button, int state, int x, int y);
void glutMotionCallback( int x, int y );
void glutPassiveMotionCallback( int x, int y );

int GenerateMaze();
void updateMazeRunners();
void collisionResponse(float deltaTime);
CVector3f collisionResponse2(CVector3f oldPosition, CVector3f newPosition);
void StartTunnel();
void EndTunnel();
void GenerateAABBWorld(); //creates the AABB World during initial set up
void TriangleAABBCollision(float deltaTime); //Box to triabgle collision in triangleAABB.cpp
// Moved from Util.cpp (we'll be replacing this at some point...
// ...having a function exit on its own might not be a good idea)
void ExitOnGLError(const char* error_message);						// from Util.cpp
GLuint LoadShader(const char* filename, GLenum shader_type);		// from Util.cpp

static const double PI = 3.14159265358979323846;		// Was in Util.h, but super helpful, yo!

#define FOR_EACH( type, container, it ) \
	for ( type::iterator it = container.begin(); \
	      it != container.end(); it++ )

class CGameState
{
public:
	enum enumGameMode
	{
		GAME_LOADING,
		GAME_RUNNING, 
		EDITING_LIGHTS,
		EDITING_OBJECTS,
		UNKNOWN
	};

	enum enumPlayerCameraMode
	{
		IN_COCKPIT = (UNKNOWN + 1),
		OVER_SHOULDER_FOLLOW
	};

	enumGameMode currentGameMode;
	enumPlayerCameraMode currentPlayerCameraMode;

	unsigned int selectedLightID;
	unsigned int selectedObjectID;

	bool bPrintViperPhysicsStats;
	bool bIsFullScreen;

	CGameState() : 
		currentGameMode(UNKNOWN), selectedLightID(0), selectedObjectID(3),
		currentPlayerCameraMode(OVER_SHOULDER_FOLLOW), 
		bPrintViperPhysicsStats(false), 
		bIsFullScreen(true)
		{};


};


extern CGameState g_gameState;

void ShutDown(void);
				

#endif
