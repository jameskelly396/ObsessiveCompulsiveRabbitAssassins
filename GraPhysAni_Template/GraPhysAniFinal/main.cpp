//Graphics Assignment 2 by James Kelly & Eric Marcinowski

// Some code originally from : http://openglbook.com/the-book.html 

// Comments are from Michael Feeney (mfeeney(at)fanshawec.ca)

#include "global.h"


#define WINDOW_TITLE_PREFIX "OpenGL for the win!"
#include <iostream>		// Added
#include <sstream>		// Added

#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <vector>
#include <fstream>
#include <fmod.h>

#include "CRender/CGLShaderManager.h"

void LoadShaders(void);

void SetUpTextures(void);

void SetupLights(void);

void ReadInModelsInfo(std::string fileLocation);

bool CreateAllTheSounds(std::wstring &error);

//#define FOREACH( type, it, list ) \
//	for( type it = (list).begin(); it != (list).end(); it++ )

GLuint g_shadow_depthTexture_FrameBufferObjectID = MAXINT32;
GLuint g_shadow_depthTexture_ID = MAXINT32;

void setUpShadowTexture(void)
{
	glGenFramebuffers(1, &g_shadow_depthTexture_FrameBufferObjectID);
	glBindFramebuffer(GL_FRAMEBUFFER, g_shadow_depthTexture_FrameBufferObjectID);

	glGenTextures(1, &g_shadow_depthTexture_ID);
	glBindTexture(GL_TEXTURE_2D, g_shadow_depthTexture_ID);
	glTexStorage2D(GL_TEXTURE_2D, 11, GL_DEPTH_COMPONENT32F, SHADOW_DEPTH_TEXTURE_SIZE, SHADOW_DEPTH_TEXTURE_SIZE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, g_shadow_depthTexture_ID, 0);

	glDrawBuffer(GL_NONE); // No color buffer is drawn to.
 
	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	ExitOnGLError("There was a problem setting up the shadow texture.");

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return;
}


int main(int argc, char* argv[])
{
	printTheWhatsThisProgramAboutBlurb();

	::g_gameState.currentGameMode = CGameState::GAME_LOADING;

	::OpenGL_Initialize( argc, argv, 1200, 800 );		// Initialize(argc, argv);
	//::OpenGL_Initialize( argc, argv, 640, 480 );		// Initialize(argc, argv);

	std::wstring error;
	if (!InitFMOD(g_FMODInitparams, error))
	{
		MessageBox(NULL, error.c_str(), L"Can't init FMOD.", MB_OK);
		return -1;
	}

	if (!CreateAllTheSounds(error))
	{
		MessageBox(NULL, error.c_str(), L"Error: Some of the sounds couldn't be loaded.", MB_OK);
		return -1;
	}

	FMOD::Channel *channel;
	g_FMODResult = g_FMODInitparams.system->playSound(FMOD_CHANNEL_REUSE, g_Sounds["assets/Sound/Lab.mp3"], false, &channel);

	std::wstringstream ssErrors;
	if (!FMODERRORCHECK(g_FMODResult, error))
	{
		ssErrors << error << std::endl;
		error = ssErrors.str();
		return false;
	}

	// CModelLoaderManager
	g_pModelLoader = new CModelLoaderManager();
	g_pModelLoader->SetRootDirectory( "assets/models" );

	std::vector< CModelLoaderManager::CLoadInfo > vecModelsToLoad;

	ReadInModelsInfo("assets/Scene.txt");

	for (unsigned int i = 0; i < vecModelsInfo.size(); i++)
	{
		::g_mapModelTypes[vecModelsInfo[i].type] = vecModelsInfo[i].file;
		if (vecModelsInfo[i].tex.size() > 0){
			::g_mapTextures[i] = vecModelsInfo[i].tex[0];
		}
		//vecModelsToLoad.push_back(CModelLoaderManager::CLoadInfo(vecModelsInfo[i].file, 1.0f, true));
	}
	for (unsigned int i = 0; i < vecModelsInfo.size(); i++)
	{
		for (std::map<std::string, std::string>::iterator jIterator = g_mapModelTypes.begin(); jIterator != ::g_mapModelTypes.end(); jIterator++)
		{
			if (vecModelsInfo[i].type == jIterator->first){
				if (vecModelsInfo[i].isEnvironment){
					vecModelsToLoad.push_back(CModelLoaderManager::CLoadInfo(jIterator->second));
				}
				else
					vecModelsToLoad.push_back(CModelLoaderManager::CLoadInfo(jIterator->second, 1.0f, true));
			}
		}
	}

	vecModelsToLoad.push_back( CModelLoaderManager::CLoadInfo("sphere_UV_xyz.ply", 1.0f, true) );
	vecModelsToLoad.push_back( CModelLoaderManager::CLoadInfo("Cube.ply", 1.0f, true ) );
	vecModelsToLoad.push_back( CModelLoaderManager::CLoadInfo("CubeUV.ply", 1.0f, true));
	vecModelsToLoad.push_back( CModelLoaderManager::CLoadInfo("floor.ply", 1.0f, true));
	vecModelsToLoad.push_back( CModelLoaderManager::CLoadInfo("1x1_6_Star_2_Sided.ply", 1.0f, true ) );

	if ( ! g_pModelLoader->LoadModels( vecModelsToLoad ) )
	{
		std::cout << "Can't load one or more models. Sorry it didn't work out." << std::endl;
		return -1;
	}

	g_pShaderManager = new CGLShaderManager();
	//LoadShaders();		// Moved from CreateCube
	CShaderDescription uberVertex;
	uberVertex.filename = "assets/shaders/OpenGL.LightTexSkyUber.vertex.glsl";
	uberVertex.name = "UberVertex";

	CShaderDescription uberFragment;
	uberFragment.filename = "assets/shaders/OpenGL.LightTexSkyUber.fragment_texture.glsl";
	uberFragment.name = "UberFragment";

	CShaderProgramDescription uberShader("UberShader", uberVertex, uberFragment );

	if ( !g_pShaderManager->CreateShaderProgramFromFile( uberShader ) )
	{
		std::cout << "Error compiling one or more shaders..." << std::endl;
		std::cout << g_pShaderManager->getLastError() << std::endl;
		std::cout << uberShader.getErrorString() << std::endl;
		return -1;
	}

	::SetUpTextures();


	g_pFactoryMediator = new CFactoryMediator();

	static const float oneDegreeInRadians = static_cast<float>(PI) / 180.0f;

	for (unsigned i = 0; i < vecModelsInfo.size(); i++)
	{
		CPhysicalProp prop;
		prop.position = vecModelsInfo[i].pos;
		prop.setOrientationEulerAngles(CVector3f(oneDegreeInRadians * vecModelsInfo[i].rot.x, oneDegreeInRadians * vecModelsInfo[i].rot.y, oneDegreeInRadians * vecModelsInfo[i].rot.z));
		prop.rotStep = vecModelsInfo[i].rot;

		CMeshDesc mesh(vecModelsInfo[i].file);
		mesh.scale = vecModelsInfo[i].scale;
		mesh.bIsSkybox = vecModelsInfo[i].isSkybox;
		mesh.bIsPlayer = vecModelsInfo[i].isPlayer;
		mesh.modelID = i;
		mesh.blend = vecModelsInfo[i].blend;
		mesh.debugColour = CVector4f(vecModelsInfo[i].col.x, vecModelsInfo[i].col.y, vecModelsInfo[i].col.z, 1.0f);

		if (vecModelsInfo[i].isTransparent){
			mesh.bIsTransparent = true;
			mesh.tranparency = vecModelsInfo[i].transparency;
		}
		else{
			mesh.bIsTransparent = false;
			mesh.tranparency = 1.0f;
		}

		if (vecModelsInfo[i].tex.size() > 0)
		{
			mesh.bHasTexture = true;
			mesh.firstTex = vecModelsInfo[i].firstTex;
			mesh.textureName_0 = vecModelsInfo[i].tex[0][0];
			if (vecModelsInfo[i].tex.size() > 1){
				mesh.textureName_0 = vecModelsInfo[i].tex[0][1];
			}
		}

		if (vecModelsInfo[i].isMaze)
		{
			mesh.bIsMaze = true;
		}

		mesh.bIsParticle = vecModelsInfo[i].isParticle;

		vecModelsInfo[i].ID = g_pFactoryMediator->CreateObjectByType(vecModelsInfo[i].type, prop, mesh);
		if (vecModelsInfo[i].isPlayer)
			::g_Player_ID = vecModelsInfo[i].ID;
		if (vecModelsInfo[i].isSkybox)
			::g_skyBoxID = vecModelsInfo[i].ID;
		if (vecModelsInfo[i].isLightBall)
			::g_lightModelID = vecModelsInfo[i].ID;
		if (vecModelsInfo[i].isParticle)
			::g_ParticleID = vecModelsInfo[i].ID;
		if (vecModelsInfo[i].isMaze)
			::g_vecMazeIDs.push_back(vecModelsInfo[i].ID);
		if (vecModelsInfo[i].isMob)
			::g_mobID.push_back(vecModelsInfo[i].ID);
	}
	
	/*CMeshDesc enviroMesh("NubianComplex2.ply");
	enviroMesh.debugColour = CVector4f(0.75f, 0.5f, 0.3f, 0.0f);
	g_enviroID = g_pFactoryMediator->CreateObjectByType("Object", enviroMesh);*/

	CMeshDesc sphereMesh("sphere_UV_xyz.ply");
	sphereMesh.debugColour = CVector4f(0.75f, 0.5f, 0.3f, 0.0f);
	sphereMesh.bIsTransparent = true;
	//sphereMesh.tranparency = 0.4f;
		CPhysicalProp sphereProps( CVector3f( 0.0f, 5.0f, 0.0f ) );
		::g_lightModelID = g_pFactoryMediator->CreateObjectByType( "Sphere UV", sphereProps, sphereMesh );


	CPhysicalProp playerProps(CVector3f(1.5f, 1.5f, 1.5f));
	/*CMeshDesc playerMesh("bun_zipper_res4_xyz.ply");
	playerMesh.debugColour = CVector4f(0.75f, 0.5f, 0.3f, 0.0f);
	playerMesh.scale = 1.0f;
	::g_Player_ID = g_pFactoryMediator->CreateObjectByType("Player", playerProps, playerMesh);*/

	/*CMeshDesc mobMesh("bun_zipper_res4_xyz.ply");
	mobMesh.debugColour = CVector4f(0.75f, 0.5f, 0.3f, 0.0f);
	mobMesh.scale = 1.0f;
	CPhysicalProp mobProps(CVector3f(0.0f, 5.0f, 4.0f));
	::g_mobID.push_back(g_pFactoryMediator->CreateObjectByType("Mob1", mobProps, mobMesh));
	::g_mobID.push_back(g_pFactoryMediator->CreateObjectByType("Mob2", mobProps, mobMesh));
	::g_mobID.push_back(g_pFactoryMediator->CreateObjectByType("Mob3", mobProps, mobMesh));
	::g_mobID.push_back(g_pFactoryMediator->CreateObjectByType("Mob4", mobProps, mobMesh));*/


	GenerateMaze();

	/*std::vector<CMeshDesc> mazeMesh;
	g_pFactoryMediator->getRenderingPropertiesByID(g_vecMazeIDs[0], mazeMesh);
	CPhysicalProp mazeProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_vecMazeIDs[0], mazeProp);
	for (int i = 0; i < 50; i++){
		for (int j = 0; j < 2; j++){
			for (int k = 0; k < 50; k++){
				if (world[i][j][k] != 0){
					mazeProp.position = CVector3f(i, j, k);
					::g_pFactoryMediator->CreateObjectByType("Maze", mazeProp, mazeMesh[0]);
				}
			}
		}
	}*/

	
	//GenerateAABBWorld();

	// Added October 3, 2014
	g_pCamera = new CCamera();
	// Camera expects an IMediator*, so cast it as that
	g_pCamera->SetMediator( (IMediator*)g_pFactoryMediator );

	g_pCamera->eye.x = 5.0f;		// Centred (left and right)
	g_pCamera->eye.y = 12.0f;		// 2.0 units "above" the "ground"
	g_pCamera->eye.z = -4.0f;		// .0funits0 from "back" the origin    .  1, 2.5, 1.8
	g_pCamera->target.x = 50.0f;		// Centred (left and right)
	g_pCamera->target.y = 350.0f;		// 2.0 units "above" the "ground"
	g_pCamera->target.z = 50.0f;		// 0.0 units "back" from the origin
	g_pCamera->cameraAngleYaw = 3.1f;
	g_pCamera->cameraAnglePitch = -0.9f;

	g_pCamera->up.x = 0.0f;
	g_pCamera->up.y = 1.0f;				// The Y axis is "up and down"

	g_pCamera->m_LEFPLookupMode = CCamera::LERP;

	g_pCamera->setMode_FollowAtDistance( ::g_Player_ID );
	float followSpeed = 30.0f;		// 1.0f whatever per second
	float followMinDistance = 3.0f;
	float followMaxSpeedDisance = 100.0f;
	g_pCamera->setFollowDistances( followMinDistance, followMaxSpeedDisance );
	g_pCamera->setFollowMaxSpeed( followSpeed );

	// A "fly through" camera
	// These numbers are sort of in the direction of the original camera
	g_pCamera->orientation = glm::fquat( 0.0960671529f, 0.972246766f, -0.0900072306f, -0.193443686f );
	glm::normalize( g_pCamera->orientation );
	//g_pCamera->setMode_IndependentFreeLook();


	g_pMouseState = new CMouseState();

	// Set up the basic lighting
	ExitOnGLError("Error setting light values");
	SetupLights();

		//
	if ( !::g_p_LightManager->initShadowMaps( 1, CLightManager::DEFAULT_SHADOW_DEPTH_TEXTURE_SIZE ) )
	{
		std::cout << "Error setting up the shadow textures: " << std::endl;
		std::cout << ::g_p_LightManager->getLastError();
	}

	setUpShadowTexture();

	//***************************************
	g_p_myEmitter = new CParticleEmitter();
	//**************************************
	

	g_p_GameControllerManager = CGameControllerManager::getGameControllerManager();
	g_p_GameController_0 = g_p_GameControllerManager->getController(0);
	if ( ( g_p_GameController_0 != 0 ) && 
		 ( g_p_GameController_0->bIsConnected() ) )
	{
		//g_p_GameController_0->AddVibrationSequence( IGameController::CVibStep::LEFT, 0.5f, 2.0f );
		//g_p_GameController_0->AddVibrationSequence( IGameController::CVibStep::RIGHT, 0.5f, 1.0f );
		std::cout << "Game controller 0 found!" << std::endl;
	}
	else 
	{
		std::cout << "Didn't get an ID for the game controller; is there one plugged in?" << std::endl;
	}

	//CPhysicalProp playerProps;
	::g_pFactoryMediator->getPhysicalPropertiesByID(g_Player_ID, playerProps); 
	g_p_myEmitter->SetLocation(playerProps.position);
	::g_p_myEmitter->GenerateParticles(50, /*number of particles*/
		CVector3f(1.1f, 1.0f, 1.1f), /*Init veloc.*/
		0.1f, /*dist from source */
		5.0f, /*seconds*/
		true);
	::g_p_myEmitter->SetAcceleration(CVector3f(0.0f, 0.0f, -10.0f));



	::g_p_PhysicsThingy = new CPhysicsCalculatron();


	// Added in animation on Sept 19
	::g_simTimer.Reset();
	::g_simTimer.Start();		// Start "counting"

	::g_gameState.currentGameMode = CGameState::GAME_RUNNING;
	
	// FULL SCREEN, Mr. Data!
	//::glutFullScreen();

	ExitOnGLError("Error getting uniform light variables");

	GLuint uberShaderID = ::g_pShaderManager->GetShaderIDFromName("UberShader");
	if (!::g_pShaderManager->UseShaderProgram(uberShaderID))
	{
		std::cout << "Can't switch to shader " << uberShaderID << std::endl;
		return 0;
	}

	std::vector< IGameObjectRender* > vec_pRenderedObjects;
	g_pFactoryMediator->getRenderedObjects(vec_pRenderedObjects, IFactory::EVERYTHING_VISIBLE);
	//SortGameObjects(vec_pRenderedObjects);

	for (int i = 0; i < vec_pRenderedObjects.size(); i++)
	{
		std::vector< CMeshDesc > vecMeshes;
		vec_pRenderedObjects[i]->getMeshDescriptions(vecMeshes);
		for (std::vector<CMeshDesc>::iterator itCurMesh = vecMeshes.begin(); itCurMesh != vecMeshes.end(); itCurMesh++)
		{
			unsigned int texCount = 0;
			for (std::map<unsigned int, std::vector<std::string> >::iterator itr = g_mapTextures.begin(); itr != g_mapTextures.end(); ++itr)
			{
				if (itr->second[0] == "Skybox")
				{
					::g_pShaderManager->SetUniformVar1i(uberShaderID, "skyMapTexture", 0);

				}
				else if (itCurMesh->bHasTexture)
				{
					std::stringstream ss;
					if (texCount < 10)
						ss << "texture_0" << texCount;
					else
						ss << "texture_" << texCount;
					::g_pShaderManager->SetUniformVar1i(uberShaderID, ss.str(), texCount + 1);	// 3rd param is slot
					texCount++;
				}
				ExitOnGLError("ERROR: Could not use the shader program");
			}
		}
	}

	glutMainLoop();

	std::cout << "Shutting down..." << std::endl;

	ShutDown();
  
	exit(EXIT_SUCCESS);
}

void ShutDown(void)
{
	delete ::g_p_PhysicsThingy;
	delete ::g_pCamera;
	delete ::g_pMouseState;
	delete ::g_pFactoryMediator;
	return;
}

std::string GetModeString(void)
{
	std::stringstream ss;

	switch ( ::g_gameState.currentGameMode )
	{
	case CGameState::UNKNOWN:
		break;
	case CGameState::GAME_RUNNING:
		ss << "(game running)";
		break;
	case CGameState::GAME_LOADING:
		ss << "(game loading)";
		break;
	case CGameState::EDITING_LIGHTS:
		ss << "(editing light# " << ::g_gameState.selectedLightID << " )";
		break;
	case CGameState::EDITING_OBJECTS:
		ss << "(editing object# " << ::g_gameState.selectedObjectID << " )";
		break;
	};

	return ss.str();
}


void TimerFunction(int Value)
{
  if (0 != Value) 
  {
	std::stringstream ss;
	ss << WINDOW_TITLE_PREFIX << ": " 
		<< ::g_FrameCount * 4 << " FPS, screen( "
		<< ::g_screenWidth << ", "					// << CurrentWidth << " x "
		<< ::g_screenHeight <<" )  ";					// << CurrentHeight;

	ss << GetModeString();
	
	// Set the "mode" title
	glutSetWindowTitle(ss.str().c_str());
  }

  ::g_FrameCount = 0;	// FrameCount = 0;
  glutTimerFunc(250, TimerFunction, 1);

  return;
}

void SetupLights(void)
{
	g_p_LightManager = new CLightManager();

	g_p_LightManager->setMaxLights(::g_ShaderUniformVariables.MAXLIGHTS);

	g_p_LightManager->CreateBasicOpenGLLights(true);

	g_p_LightManager->GetLightPointer(0)->setLightType(CLight::POINT);
	g_p_LightManager->GetLightPointer(0)->position = CVector3f(50.0f, 30.0f, 50.0f);
	g_p_LightManager->GetLightPointer(0)->bIsEnabled = true;

	return;
}

void SetUpTextures(void)
{
	g_p_TextureManager = new CTextureManager();

	unsigned int totalTextures = 0;
	for (unsigned i = 0; i < vecModelsInfo.size(); i++)
	{
		if (vecModelsInfo[i].isSkybox)
		{
			::g_p_TextureManager->setBasePath("assets/textures/SkyBoxes_by_Michael");
			if (!g_p_TextureManager->CreateCubeTextureFromBMPFiles(vecModelsInfo[i].tex[0][0],
				vecModelsInfo[i].tex[0][1], vecModelsInfo[i].tex[0][2],
				vecModelsInfo[i].tex[0][3], vecModelsInfo[i].tex[0][4],
				vecModelsInfo[i].tex[0][5], vecModelsInfo[i].tex[0][6],
				true, true))
			{
				std::cout << "Didn't load the sky box texture(s):" << std::endl;
				std::cout << ::g_p_TextureManager->getLastError() << std::endl;
			}
			totalTextures++;
			ExitOnGLError("Skymap didn't load.");
		}
		else if (vecModelsInfo[i].tex.size() > 0)
		{
			g_p_TextureManager->setBasePath("assets/textures");
			for (unsigned j = 0; j < vecModelsInfo[i].tex[0].size(); j++)
			{
				if (!g_p_TextureManager->Create2DTextureFromBMPFile(vecModelsInfo[i].tex[0][j], true))
				{
					std::cout << ::g_p_TextureManager->getLastError() << std::endl;
				}
				if (j == 0)
					vecModelsInfo[i].firstTex = totalTextures;
				totalTextures++;
			}
		}
	}

	return;
}

void ReadInModelsInfo(std::string fileLocation)
{
	std::ifstream myFile((fileLocation).c_str());

	//Check if file is open
	if (!myFile.is_open())
	{
		ExitOnGLError("ERROR: Could not open text file");
		return;
	}

	Model tempModel;
	Light tempLight;
	const Model blankModel;
	const Light blankLight;
	std::string tempString;
	bool bKeepReading = true;

	myFile >> tempString;

	//Read in the file
	while (bKeepReading)
	{
		if (tempString == "<Model>")
		{
			while (true)
			{
				myFile >> tempString;

				if (tempString == "<Type>")
				{
					myFile >> tempModel.type;
					myFile >> tempString;
				}
				else if (tempString == "<PlyFile>")
				{
					myFile >> tempModel.file;
					myFile >> tempString;
				}
				else if (tempString == "<Texture>")
				{
					myFile >> tempString;
					tempModel.tex.push_back(std::vector<std::string>(0));
					tempModel.tex[0].push_back(tempString);
					myFile >> tempString;
				}
				else if (tempString == "<Blend>")
				{
					myFile >> (unsigned int)tempModel.blend;
					myFile >> tempString;
				}
				else if (tempString == "<SkyboxTexture>")
				{
					myFile >> tempString;
					if (tempString == "<Name>")
					{
						myFile >> tempString;
						tempModel.tex.push_back(std::vector<std::string>(0));
						tempModel.tex[0].push_back(tempString);
						myFile >> tempString;
					}
					else
					{
						ExitOnGLError("ERROR: Formatting error in model reader.");
						return;
					}

					for (unsigned i = 0; i < 6; i++)
					{
						myFile >> tempString;
						if (tempString == "<Texture>")
						{
							myFile >> tempString;
							tempModel.tex[0].push_back(tempString);
							myFile >> tempString;
						}
						else
						{
							ExitOnGLError("ERROR: Formatting error in model reader.");
							return;
						}
					}
					myFile >> tempString;
				}
				else if (tempString == "<Rotation>")
				{
					myFile >> (float)tempModel.rot.x >> (float)tempModel.rot.y >> (float)tempModel.rot.z;
					myFile >> tempString;
				}
				else if (tempString == "<Colour>")
				{
					myFile >> (float)tempModel.col.x >> (float)tempModel.col.y >> (float)tempModel.col.z;
					myFile >> tempString;
				}
				else if (tempString == "<Scale>")
				{
					myFile >> (float)tempModel.scale;
					myFile >> tempString;
				}
				else if (tempString == "<Transparency>")
				{
					myFile >> (float)tempModel.transparency;
					if (tempModel.transparency < 1.0f)
						tempModel.isTransparent = true;
					myFile >> tempString;
				}
				else if (tempString == "<Position>")
				{
					myFile >> (float)tempModel.pos.x >> (float)tempModel.pos.y >> (float)tempModel.pos.z;
					myFile >> tempString;
				}
				else if (tempString == "<IsPlayer>")
				{
					myFile >> tempString;
					if (tempString == "True" || tempString == "true" || tempString == "TRUE")
						tempModel.isPlayer = true;
					else
						tempModel.isPlayer = false;
					myFile >> tempString;
				}
				else if (tempString == "<IsMob>")
				{
					myFile >> tempString;
					if (tempString == "True" || tempString == "true" || tempString == "TRUE")
						tempModel.isMob = true;
					else
						tempModel.isMob = false;
					myFile >> tempString;
				}
				else if (tempString == "<IsSkybox>")
				{
					myFile >> tempString;
					if (tempString == "True" || tempString == "true" || tempString == "TRUE")
						tempModel.isSkybox = true;
					else
						tempModel.isSkybox = false;
					myFile >> tempString;
				}
				else if (tempString == "<IsEnviro>")
				{
					myFile >> tempString;
					if (tempString == "True" || tempString == "true" || tempString == "TRUE")
						tempModel.isEnvironment = true;
					else
						tempModel.isEnvironment = false;
					myFile >> tempString;
				}
				else if (tempString == "<IsLightBall>")
				{
					myFile >> tempString;
					if (tempString == "True" || tempString == "true" || tempString == "TRUE")
						tempModel.isLightBall = true;
					else
						tempModel.isLightBall = false;
					myFile >> tempString;
				}
				else if (tempString == "<IsParticle>")
				{
					myFile >> tempString;
					if (tempString == "True" || tempString == "true" || tempString == "TRUE")
						tempModel.isParticle = true;
					else
						tempModel.isParticle = false;
					myFile >> tempString;
				}
				else if (tempString == "<IsMaze>")
				{
					myFile >> tempString;
					if (tempString == "True" || tempString == "true" || tempString == "TRUE")
						tempModel.isMaze = true;
					else
						tempModel.isMaze = false;
					myFile >> tempString;
				}
				else if (tempString == "</Model>")
					break;
				else
				{
					ExitOnGLError("ERROR: Formatting error in model section.");
					return;
				}
			}
		}
		else if (tempString == "<Light>")
		{
			while (true)
			{
				myFile >> tempString;

				if (tempString == "<Type>")
				{
					myFile >> tempString;
					if (tempString == "POINT") tempLight.type = CLight::POINT;
					else if (tempString == "SPOT") tempLight.type = CLight::SPOT;
					else if (tempString == "DIRECTIONAL") tempLight.type = CLight::DIRECTIONAL;
					myFile >> tempString;
				}
				else if (tempString == "<Position>")
				{
					myFile >> (float)tempLight.pos.x >> (float)tempLight.pos.y >> (float)tempLight.pos.z;
					myFile >> tempString;
				}
				else if (tempString == "<Colour>")
				{
					myFile >> (float)tempLight.col.x >> (float)tempLight.col.y >> (float)tempLight.col.z;
					myFile >> tempString;
				}
				else if (tempString == "<ConstAtten>")
				{
					myFile >> (float)tempLight.constAtten;
					myFile >> tempString;
				}
				else if (tempString == "<LinAtten>")
				{
					myFile >> (float)tempLight.linAtten;
					myFile >> tempString;
				}
				else if (tempString == "<QuadAtten>")
				{
					myFile >> (float)tempLight.quadAtten;
					myFile >> tempString;
				}
				else if (tempString == "<AttenRange>")
				{
					myFile >> (float)tempLight.attenRangeStart;
					myFile >> (float)tempLight.attenRangeEnd;
					myFile >> tempString;
				}
				else if (tempString == "</Light>")
					break;
				else
				{
					ExitOnGLError("ERROR: Formatting error in model section.");
					return;
				}
			}
		}

		if (tempString == "</Model>")
		{
			vecModelsInfo.push_back(tempModel);
			tempModel = blankModel;
		}
		else if (tempString == "</Light>")
		{
			vecLightsInfo.push_back(tempLight);
			tempLight = blankLight;
		}
		else
		{
			ExitOnGLError("ERROR: Root model formatting error.");
			return;
		}

		myFile >> tempString;
		if (tempString != "<Model>" && tempString != "<Light>")
			bKeepReading = false;
	}
}

bool CreateAllTheSounds(std::wstring &error)
{
	FMOD_RESULT FMODResult;
	std::wstringstream ssErrors;

	std::string fileName = "Sound.txt";
	std::string soundsDir = "assets/Sound/";

	std::ifstream myFile((soundsDir + fileName).c_str());

	//Check if file is open
	if (!myFile.is_open()) return false;

	std::string temp;
	FMOD::Sound* sound;
	bool bKeepReading = true;

	//Read in the file
	while (bKeepReading)
	{
		myFile >> temp;

		if (temp == "#SOUND") //Found a Streaming Sound
		{
			myFile >> temp;
			FMODResult = ::g_FMODInitparams.system->createStream((soundsDir + temp).c_str(), FMOD_DEFAULT, 0, &sound);
			g_Sounds[(soundsDir + temp).c_str()] = sound;
			//FMOD::Channel *channel;
			//g_channels[(soundsDir + temp).c_str()] = channel;

			if (!FMODERRORCHECK(FMODResult, error))
			{
				ssErrors << error << std::endl;
				error = ssErrors.str();
				return false;
			}
		}
		else if (temp == "#END") bKeepReading = false;
	}
	//FMODResult = ::g_FMODInitparams.system->playSound(FMOD_CHANNEL_FREE, g_Sounds[0], false, &(g_channels[0]));
	return true;
}
