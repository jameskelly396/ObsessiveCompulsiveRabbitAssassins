#include "global.h"

#include "CPhysics/Physics.h"

#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
//#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

void AddClosestPointDebugSpheres(void);

static const float g_MAXIMUM_TIME_STEP = 0.1f;		// 10th of a second or 100ms

void drawCube(int i, int j, int k) {
	CGameObject* pGameObject = new CGameObject();

	//CMeshDesc meshDesc;
	//pGameObject->m_physicalProperties.position = CVector3f(i, j, k);		// *******
	//if (world[i][j][k] == 1)
	//	meshDesc.debugColour = CVector4f(0.0f, 0.6f, 0.0f, 1.0f);
	//else
	//	meshDesc.debugColour = CVector4f(205.0f / 255.0f, 133.0f / 255.0f, 63.0f / 255.0f, 1.0f);
	//meshDesc.modelName = "Cube.ply";
	//meshDesc.scale = 1.0f;
	//meshDesc.bUseDebugColour = true;
	//meshDesc.bIsMaze = true;

	//pGameObject->addMeshDescription(meshDesc);
	//::g_vecPhysDebugObjects.push_back(pGameObject);


	std::vector<CMeshDesc> mazeMesh;
	if (world[i][j][k] == 1){
		int r = (i + k) % 3;
		g_pFactoryMediator->getRenderingPropertiesByID(g_vecMazeIDs[r], mazeMesh);
	}
	else if (world[i][j][k] == 2)
		g_pFactoryMediator->getRenderingPropertiesByID(g_vecMazeIDs[3], mazeMesh);
	else if (world[i][j][k] == 3)
		g_pFactoryMediator->getRenderingPropertiesByID(g_vecMazeIDs[4], mazeMesh);
	mazeMesh[0].bIsParticle = true;
	pGameObject->m_physicalProperties.position = CVector3f(i, j, k);
	pGameObject->addMeshDescription(mazeMesh[0]);
	::g_vecPhysDebugObjects.push_back(pGameObject);
}


static const float g_RESET_BG_MUSIC = 90.0f;  //20 seconds
static float countdownTime = 0.0f; //counts down to 0 if player doesn't make it to check point in time

static float joysticMovement = 0.0f;
static float joysticMovement2 = 0.0f;
void IdleFunction(void)
{
	// Update the "simulation" at this point.
	// 1. Get elapsed time
	g_simTimer.Stop();
	float deltaTime = ::g_simTimer.GetElapsedSeconds();
	// Is the number TOO big (we're in a break point or something?)
	if ( deltaTime > g_MAXIMUM_TIME_STEP )
	{	// Yup, so clamp it to a new value
		deltaTime = g_MAXIMUM_TIME_STEP;
	}
	g_simTimer.Start();

	countdownTime += deltaTime;
	if (g_RESET_BG_MUSIC < countdownTime)
	{
		countdownTime = 0;
		std::wstring error;
		FMOD::Channel *channel;
		g_FMODResult = g_FMODInitparams.system->playSound(FMOD_CHANNEL_REUSE, g_Sounds["assets/Sound/Lab.mp3"], false, &channel);

		std::wstringstream ssErrors;
		if (!FMODERRORCHECK(g_FMODResult, error))
		{
			ssErrors << error << std::endl;
			error = ssErrors.str();
			return;
		}
	}

	//CVector3f playerLocation;
	::g_p_GameController_0->Update( deltaTime );

	g_FMODInitparams.system->update();

	// Set the light at the same location as the player object...
	unsigned int objectID;
	if (::g_gameState.currentGameMode == CGameState::EDITING_OBJECTS)
		objectID = vecModelsInfo[g_gameState.selectedObjectID].ID;
	else
		objectID = ::g_Player_ID;
	static CPhysicalProp playerProps;
	::g_pFactoryMediator->getPhysicalPropertiesByID(objectID, playerProps);


	static std::vector< IGameObjectPhysics* > vec_pGameObjects;
	g_pFactoryMediator->getPhysicsObjects( vec_pGameObjects );

	::g_p_PhysicsThingy->simulationStep( vec_pGameObjects, deltaTime );

	for ( int index = 0; index != static_cast<int>( vec_pGameObjects.size() ); index++ )
	{
		vec_pGameObjects[index]->Update(deltaTime);
	}

	if (::g_pCamera->getCameraState() == CCamera::FOLLOW_TARGET_AT_DISTANCE_CHASE_PLANE){
		g_pCamera->eye.x = playerProps.position.x;
		g_pCamera->eye.y = playerProps.position.y + 20;
		g_pCamera->eye.z = playerProps.position.z - 5;
		g_pCamera->target = playerProps.position;
	}

	// Added October 3, 2014
	//::g_pCamera->Update( deltaTime );
		// Adjust camera based on the Right stick
	float rightJoyX = ::g_p_GameController_0->getRightJoystickXAxis();
	float rightJoyY = ::g_p_GameController_0->getRightJoystickYAxis();
	static const float lookSpeedScale = 0.01f;
	::g_pCamera->AdjustYaw( rightJoyX * lookSpeedScale );
	::g_pCamera->AdjustPitch( -rightJoyY * lookSpeedScale );
	// Move camera forward with "A"
	if ( ::g_p_GameController_0->bIsAButtonDown() )
	{
		::g_pCamera->MoveForwardBackward( deltaTime );
	}

	g_p_myEmitter->SetLocation( playerProps.position );
	g_p_myEmitter->Update( deltaTime );

	// Add a sphere wherever there is a particle...
	for ( int index = 0; index != ::g_vecPhysDebugObjects.size(); index++ )
	{
		delete g_vecPhysDebugObjects[index];
	}
	::g_vecPhysDebugObjects.clear();

	int icurX = (int)(playerProps.position.x);
	int icurZ = (int)(playerProps.position.z);
	/*if (bIsUnderground)
		world[icurX][0][icurZ] = 2;*/
	int viewRange = 10;
	if (icurX < viewRange) icurX = viewRange;
	if (icurZ < viewRange) icurZ = viewRange;
	if (icurX > 99 - viewRange) icurX = 99 - viewRange;
	if (icurZ > 99 - viewRange) icurZ = 99 - viewRange;


	for (int i = icurX - viewRange; i < icurX + viewRange; i++){
		for (int j = 0; j < 2; j++){
			for (int k = icurZ - viewRange; k < icurZ + viewRange; k++){
				if (world[i][j][k] != 0){
					drawCube(i, j, k);
				}
			}
		}
	}
	updateMazeRunners();
	collisionResponse(deltaTime);

	if (drawParticles){
		// Ask for all the "alive" particles
		std::vector< CParticleEmitter::CParticle > vecParticles;
		g_p_myEmitter->GetParticles(vecParticles);

		for (int index = 0; index != vecParticles.size(); index++)
		{
			std::vector< CMeshDesc > vecMeshDescs;
			g_pFactoryMediator->getRenderingPropertiesByID(g_ParticleID, vecMeshDescs);

			CGameObject* pGameObject = new CGameObject();
			pGameObject->m_physicalProperties.position = vecParticles[index].position;
			CMeshDesc meshDesc;
			meshDesc = vecMeshDescs[0];

			//meshDesc.modelName = "1x1_6_Star_2_Sided.ply";
			meshDesc.scale = 1.0f;
			meshDesc.tranparency = 0.5f;
			meshDesc.bIsTransparent = true;
			meshDesc.debugColour = CVector4f(1.0f, 1.0f, 1.0f, 1.0f);
			// Add it 
			pGameObject->addMeshDescription(meshDesc);
			::g_vecPhysDebugObjects.push_back(pGameObject);
		}
	}
	// *************************************

	//if (drawHitbox)
		//TriangleAABBCollision(deltaTime);

	// Place light #0 near the viper
	::g_p_LightManager->GetLightPointer( 0 )->position 
			= CVector3f( playerProps.position.x, playerProps.position.y + 50.0f, playerProps.position.z - 20.0f );	

	// ******************** 
	// Keep skybox around camera
	static CPhysicalProp skyboxProps;
	::g_pFactoryMediator->getPhysicalPropertiesByID( ::g_skyBoxID, skyboxProps );
	skyboxProps.position = ::g_pCamera->eye;
	::g_pFactoryMediator->setPhysicalPropertiesByID( ::g_skyBoxID, skyboxProps );

	float thrust = (25.0f * ::g_p_GameController_0->getRightTrigger() ) - ( 2.5f * ::g_p_GameController_0->getLeftTrigger() );
	CVector4f directedVel = CVector4f( 0.0f, 0.0f, ( thrust * deltaTime ), 0.0f );
	::g_pFactoryMediator->SendMessageToObject( ::g_Player_ID, 0, CMessage( CNameValuePair( "SetDirectedVelocity", directedVel ) ) );


	icurX = (int)(playerProps.position.x);
	icurZ = (int)(playerProps.position.z);
	float changeInPosition = 1.0f;
	float changeInVelocity = 0.1f;
	float leftJoyY = ::g_p_GameController_0->getLeftJoystickYAxis();
	joysticMovement += deltaTime;
	bool bCanMovePlayer = false;
	float walkingRange = 0.8f;
	if (::g_p_GameController_0->bIsGamePadUp() || leftJoyY > 0.0f )	{
		if (joysticMovement > 0.3f && ((leftJoyY > 0.1f && leftJoyY < walkingRange) || ::g_p_GameController_0->bIsGamePadUp()))
		{
			joysticMovement = 0.0f;
			bCanMovePlayer = true;
		}
		else if (leftJoyY > walkingRange)
		{
			joysticMovement = 0.0f;
			bCanMovePlayer = true;
		}
		if (bCanMovePlayer){
			if (bIsUnderground)
				world[icurX][0][icurZ] = 2;
			CVector3f newPosition(playerProps.position.x, playerProps.position.y, playerProps.position.z + changeInPosition);
			playerProps.position = collisionResponse2(playerProps.position, newPosition);
			playerProps.velocity = CVector3f(0.0f);
			g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProps);
		}
	}
	else if (::g_p_GameController_0->bIsGamePadDown() || leftJoyY < -0.0f) {
		if (joysticMovement > 0.3f && ((leftJoyY < 0.1f && leftJoyY > -walkingRange) || ::g_p_GameController_0->bIsGamePadDown()))
		{
			joysticMovement = 0.0f;
			bCanMovePlayer = true;
		}
		else if (leftJoyY < -walkingRange)
		{
			joysticMovement = 0.0f;
			bCanMovePlayer = true;
		}
		if (bCanMovePlayer){
			if (bIsUnderground)
				world[icurX][0][icurZ] = 2;
			CVector3f newPosition(playerProps.position.x, playerProps.position.y, playerProps.position.z - changeInPosition);
			playerProps.position = collisionResponse2(playerProps.position, newPosition);
			playerProps.velocity = CVector3f(0.0f);
			g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProps);
		}
	}

	joysticMovement2 += deltaTime;
	bCanMovePlayer = false;
	float leftJoyX = ::g_p_GameController_0->getLeftJoystickXAxis();
	if (::g_p_GameController_0->bIsGamePadLeft() || leftJoyX < -0.1f){
		if (joysticMovement2 > 0.3f && ((leftJoyX < -0.1f && leftJoyX > -walkingRange) || ::g_p_GameController_0->bIsGamePadLeft()))
		{
			joysticMovement2 = 0.0f;
			bCanMovePlayer = true;
		}
		else if (leftJoyX < -walkingRange)
		{
			joysticMovement2 = 0.0f;
			bCanMovePlayer = true;
		}
		if (bCanMovePlayer){
			if (bIsUnderground)
				world[icurX][0][icurZ] = 2;
			CVector3f newPosition(playerProps.position.x + changeInPosition, playerProps.position.y, playerProps.position.z);
			playerProps.position = collisionResponse2(playerProps.position, newPosition);
			playerProps.velocity = CVector3f(0.0f);
			g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProps);
		}
	}
	else if (::g_p_GameController_0->bIsGamePadRight() || leftJoyX > 0.1f)	{
		if (joysticMovement2 > 0.3f && ((leftJoyX > 0.1f && leftJoyX < walkingRange) || ::g_p_GameController_0->bIsGamePadRight()))
		{
			joysticMovement2 = 0.0f;
			bCanMovePlayer = true;
			//std::cout << "Moving slightly to the right." << std::endl;
		}
		else if (leftJoyX > walkingRange)
		{
			joysticMovement2 = 0.0f;
			bCanMovePlayer = true;
		}
		if (bCanMovePlayer){
			if (bIsUnderground)
				world[icurX][0][icurZ] = 2;
			CVector3f newPosition(playerProps.position.x - changeInPosition, playerProps.position.y, playerProps.position.z);
			playerProps.position = collisionResponse2(playerProps.position, newPosition);
			playerProps.velocity = CVector3f(0.0f);
			g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProps);
		}
	}

	if (::g_p_GameController_0->bIsAButtonDown())
	{
		StartTunnel();
	}
	if (::g_p_GameController_0->bIsBButtonDown())
	{
		EndTunnel();	
	}

	if (::g_p_GameController_0->bIsStartButtonDown())
	{
		playerProps.position.x = 1.5f;
		playerProps.position.y = 1.5f;
		playerProps.position.z = 1.5f;
		g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProps);
		std::wstring error;
		FMOD::Channel *channel;
		g_FMODResult = g_FMODInitparams.system->playSound(FMOD_CHANNEL_REUSE, g_Sounds["assets/Sound/Lab.mp3"], false, &channel);

		std::wstringstream ssErrors;
		if (!FMODERRORCHECK(g_FMODResult, error))
		{
			ssErrors << error << std::endl;
			error = ssErrors.str();
			return;
		}
	}
	if ( ::g_gameState.bPrintViperPhysicsStats )
	{
		CPhysicalProp viperPhysProps;
		::g_pFactoryMediator->getPhysicalPropertiesByID( ::g_Player_ID, viperPhysProps );
		std::cout << "Viper thrust = " << viperPhysProps.directedVelocity.z 
			<< " accel = " << viperPhysProps.directedAcceleration.z 
			<< " vel( " << viperPhysProps.velocity.x << ", " 
						<< viperPhysProps.velocity.y << ", " 
						<< viperPhysProps.velocity.z << ")"
			<< " pos( " << viperPhysProps.position.x << ", "
						<< viperPhysProps.position.y << ", "
						<< viperPhysProps.position.z << ") " << std::endl;
	}

	glutPostRedisplay();
	return;
}