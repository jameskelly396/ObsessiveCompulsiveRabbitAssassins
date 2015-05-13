/* James kelly
 * March 7 2013
 * CIS*4820
 * Assignment 3
 */

/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>		// Added

#include "global.h"

/* initialize graphics library */
extern void gradphicsInit(int *, char **);

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void getViewOrientation(float *, float *, float *);

/* add cube to display list so it will be drawn */
extern int addDisplayList(int, int, int);

/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void getMobPosition(int, float *, float *, float *, float *);
extern void hideMob(int);
extern void showMob(int);

/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* list and count of polygons to be displayed, set during culling */
extern int displayCount;
/* flag to print out frames per second */
extern int fps;
/* flag to indicate removal of cube the viewer is facing */
extern int dig;
/* flag indicates the program is a client when set = 1 */
extern int netClient;
/* flag indicates the program is a server when set = 1 */
extern int netServer; 

/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/* Maze generating and collision detecting variables*/
int MazeHeight = 1; //how high the maze will be on the Y axis
int MazeHeight2 = 3; //how high the second maze will be on the Y axis
const int MazeSize = 100;



/* Time values */
time_t tStart, tEnd;

float xYellowBox = 55.5, yYellowBox = 7.5, zYellowBox = 55.5;

typedef struct nodeAIPath{
    int x, z;
    struct nodeAIPath *link;
} AIpath;

typedef struct graphNode{
    int nodeNum;
    int xLoc, zLoc;
    int numConnected; //number of connections
    int *connected; //
    AIpath **startPath;
} graph;

AIpath *A1nextStep;
AIpath *A1returnStep;
AIpath *B1nextStep;
AIpath *B1returnStep;
AIpath *C1nextStep;
AIpath *C1returnStep;
AIpath *D1nextStep;
AIpath *D1returnStep;

AIpath *A2nextStep;
AIpath *A2returnStep;
AIpath *B2nextStep;
AIpath *B2returnStep;
AIpath *C2nextStep;
AIpath *C2returnStep;
AIpath *D2nextStep;
AIpath *D2returnStep;

AIpath *cpA1, *cpB1, *cpC1, *cpD1;
AIpath *cpA2, *cpB2, *cpC2, *cpD2;

const static int numOfMobs = 4;
int mobCleanup[numOfMobs] = { 0 };
bool PathFlag[numOfMobs] = { 0 };
static int AIFollowFlag[numOfMobs] = { 0 };
AIpath *cpAIpath[numOfMobs];
AIpath *AInextStep[numOfMobs];
AIpath *AIreturnStep[numOfMobs];

/********* end of extern variable declarations **************/

int playSound(const char* soundName) {

	std::wstring error;
	FMOD::Channel *channel;
	g_FMODResult = g_FMODInitparams.system->playSound(FMOD_CHANNEL_REUSE, g_Sounds[soundName], false, &channel);

	std::wstringstream ssErrors;
	if (!FMODERRORCHECK(g_FMODResult, error))
	{
		ssErrors << error << std::endl;
		error = ssErrors.str();
		std::cout << error.c_str() << std::endl;
		return false;
	}

	return 0;
}

void StartTunnel()
{
	CPhysicalProp playerProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_Player_ID, playerProp);

	int itunnelX = (int)(playerProp.position.x * 1);
	int itunnelZ = (int)(playerProp.position.z * 1);

	if (world[itunnelX][0][itunnelZ] == 2)
		return;

	playerProp.position.y = 0.0f;
	g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProp);

	bIsUnderground = true;
	world[itunnelX][0][itunnelZ] = 2;
	playSound("assets/Sound/Rustle.mp3");
}

void EndTunnel()
{
	CPhysicalProp playerProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_Player_ID, playerProp);

	int itunnelX = (int)(playerProp.position.x * 1);
	int itunnelZ = (int)(playerProp.position.z * 1);

	if (world[itunnelX][1][itunnelZ] != 0 || !bIsUnderground)
		return;

	playerProp.position.y = 1.0f;
	g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProp);

	bIsUnderground = false;
	world[itunnelX][0][itunnelZ] = 2;
}

void collisionResponse(float deltaTime) {
	//obtain view points
	CPhysicalProp playerProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_Player_ID, playerProp);
	float curX = playerProp.position.x;
	float curY = playerProp.position.y;
	float curZ = playerProp.position.z;
	float oldX = playerProp.position.x - playerProp.velocity.x * deltaTime;
	float oldY = playerProp.position.y - playerProp.velocity.y * deltaTime;
	float oldZ = playerProp.position.z - playerProp.velocity.z * deltaTime;

	//converts old float points into integers
	int ioldX = (int)(oldX*1);
	int ioldY = (int)(oldY*1);
	int ioldZ = (int)(oldZ*1);

	//converts current negative float points to positive integers
	//boxSize effectively creates an invisible "box" around the player to deal with collision detections
	//its current dimensions are larger than the view point
	//this was done so when players are moving along side a wall, half their vision is not inside the wall
	//to remove this feature set boxSize to 0.0
	float boxSize = 0.2f;

	int i1curX = (int)((curX - boxSize)*1);
	int i1curY = (int)((curY - boxSize)*1);
	int i1curZ = (int)((curZ - boxSize)*1);
	int i2curX = (int)((curX + boxSize)*1);
	int i2curY = (int)((curY + boxSize)*1);
	int i2curZ = (int)((curZ + boxSize)*1);

	//new values will be entered into the final resulting position, if any cur variables are out of range, set the new values to the old ones
	//each coordinate (X,Y, & Z) is handled individually to solve the issue of being "stuck on walls"
	float newX = curX;
	float newY = curY;
	float newZ = curZ;

	//maze wall collision
	if (world[i1curX][ioldY][ioldZ] == 1 || world[i2curX][ioldY][ioldZ] == 1){
		newX = oldX;
	}

	if (world[ioldX][i1curY][ioldZ] == 1 || world[ioldX][i2curY][ioldZ] == 1){
		newY = oldY;
	}

	if (world[ioldX][ioldY][i1curZ] == 1 || world[ioldX][ioldY][i2curZ] == 1){
		newZ = oldZ;
	}

	//outer barrier
	if (curX <= -1.0f || curX >= 99.0f){
		newX = oldX;
	}

	if (curY <= -1.0f || curY >= 2.0f){
		newY = oldY;
	}

	if (curZ <= -1.0f || curZ >= 99.0f){
		newZ = oldZ;
	}

	//sets current view position to a valid position
	playerProp.position = CVector3f(newX, newY, newZ);
	g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProp);

	CGameObject* pGameObject = new CGameObject();
	//CPhysicalProp physicalProps;
	CMeshDesc meshDesc;
	pGameObject->m_physicalProperties.position = CVector3f(newX, newY, newZ);		// *******
	meshDesc.debugColour = CVector4f(0.0f, 1.0f, 0.0f, 1.0f);
	meshDesc.modelName = "sphere_UV_xyz.ply";
	meshDesc.scale = 0.1f;
	meshDesc.bUseDebugColour = true;
	meshDesc.bIsMaze = true;

	pGameObject->addMeshDescription(meshDesc);
	::g_vecPhysDebugObjects.push_back(pGameObject);
}

CVector3f collisionResponse2(CVector3f oldPosition, CVector3f newPosition) {
	//obtain player location where the wish to move
	CPhysicalProp playerProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_Player_ID, playerProp);
	float curX = newPosition.x;
	float curY = newPosition.y;
	float curZ = newPosition.z;
	float oldX = oldPosition.x;
	float oldY = oldPosition.y;
	float oldZ = oldPosition.z;

	//converts old float points into integers

	int ioldX = (int)(oldX * 1);
	int ioldY = (int)(oldY * 1);
	int ioldZ = (int)(oldZ * 1);

	//converts current negative float points to positive integers
	//boxSize effectively creates an invisible "box" around the player to deal with collision detections
	//its current dimensions are larger than the view point
	//this was done so when players are moving along side a wall, half their vision is not inside the wall
	//to remove this feature set boxSize to 0.0
	float boxSize = 0.1f;

	int i1curX = (int)((curX - boxSize) * 1);
	int i1curY = (int)((curY - boxSize) * 1);
	int i1curZ = (int)((curZ - boxSize) * 1);
	int i2curX = (int)((curX + boxSize) * 1);
	int i2curY = (int)((curY + boxSize) * 1);
	int i2curZ = (int)((curZ + boxSize) * 1);

	//new values will be entered into the final resulting position, if any cur variables are out of range, set the new values to the old ones
	//each coordinate (X,Y, & Z) is handled individually to solve the issue of being "stuck on walls"
	float newX = curX;
	float newY = curY;
	float newZ = curZ;

	//maze wall collision
	if (world[i1curX][ioldY][ioldZ] == 1 || world[i2curX][ioldY][ioldZ] == 1){
		newX = oldX;
	}

	if (world[ioldX][i1curY][ioldZ] == 1 || world[ioldX][i2curY][ioldZ] == 1){
		newY = oldY;
	}

	if (world[ioldX][ioldY][i1curZ] == 1 || world[ioldX][ioldY][i2curZ] == 1){
		newZ = oldZ;
	}

	if (world[i1curX][ioldY][ioldZ] == 2 || world[i2curX][ioldY][ioldZ] == 2){
		newX = oldX;
	}

	if (world[ioldX][i1curY][ioldZ] == 2 || world[ioldX][i2curY][ioldZ] == 2){
		newY = oldY;
	}

	if (world[ioldX][ioldY][i1curZ] == 2 || world[ioldX][ioldY][i2curZ] == 2){
		newZ = oldZ;
	}

	//outer barrier
	if (curX <= -1.0f || curX >= 99.0f){
		newX = oldX;
	}

	if (curY <= -1.0f || curY >= 2.0f){
		newY = oldY;
	}

	if (curZ <= -1.0f || curZ >= 99.0f){
		newZ = oldZ;
	}


	CGameObject* pGameObject = new CGameObject();
	CMeshDesc meshDesc;
	pGameObject->m_physicalProperties.position = CVector3f(newX, newY, newZ);		// *******
	meshDesc.debugColour = CVector4f(0.0f, 1.0f, 0.0f, 1.0f);
	meshDesc.modelName = "sphere_UV_xyz.ply";
	meshDesc.scale = 0.1f;
	meshDesc.bUseDebugColour = true;
	meshDesc.bIsMaze = true;

	pGameObject->addMeshDescription(meshDesc);
	::g_vecPhysDebugObjects.push_back(pGameObject);

	//sets current view position to a valid position
	return CVector3f(newX, newY, newZ);
}

void initializeMaze(int height, int color){
	//fills the maze area with 1s to then be filled out by depthFirstSearchFunc
	int i, j;
	for(i = 0; i < MazeSize; i++) {
		for (j = 0; j < MazeSize; j++) {
			world[i][height][j] = color;
		}
	}	
}

void mazeCopy(int maze[MazeSize][MazeSize], int ycord){
    int i, j;
	for (i = 0; i < MazeSize; i++)
		for (j = 0; j < MazeSize; j++)
            maze[i][j] = world[i][ycord][j];
}

AIpath* dfsAIPath(int maze[MazeSize][MazeSize], int curX, int curZ, int lastX, int lastZ){
    int randDirTemp[4];
	int randDir[4];
	int count = 0;
	int i,r = 0;
	
	for (i=0; i < 4; i++){ //fills the arrays with {0,1,2,3} with each number representing a direction
		randDirTemp[i] = i;
		randDir[i] = i;
	}
	for (i=0; i < 4; i++){ //randomly rearrange the order of directions
		r = rand()%4;
		if(randDirTemp[r]<0)
			--i;
		else{
			randDir[count] = randDirTemp[r];
			count++;
			randDirTemp[r] =-1;
		}
	}
    
    maze[curX][curZ]=1;
    if(curX == lastX && curZ == lastZ){
		AIpath *nextStep = new AIpath();
        //AIpath *nextStep = malloc(sizeof(AIpath));
        nextStep->link = NULL;
        nextStep->x = lastX;
        nextStep->z = lastZ;
        return nextStep;
    }
    else {
        for (i=0; i < 4; i++){ //goes through and checks each possible direction in the array
            switch (randDir[i]) {
                case 0:
                    if(maze[curX-1][curZ]==0){
						//std::cout << "curX=" << curX-1 << "(-1) curZ=" << curZ << " lastX=" << lastX << " lastZ=" << lastZ << std::endl;
                        AIpath *nextStep = dfsAIPath(maze, curX-1, curZ, lastX, lastZ);
                        if (nextStep!=NULL) {
							AIpath *toReturn = new AIpath();
							//AIpath *toReturn=malloc(sizeof(AIpath));
                            toReturn->link = nextStep;
                            toReturn->x = curX;
                            toReturn->z = curZ;
                            return toReturn;
                        }
                    }
                    continue;
                case 1:
                    if(maze[curX+1][curZ]==0){
						//std::cout << "curX=" << curX + 1 << "(+1) curZ=" << curZ << " lastX=" << lastX << " lastZ=" << lastZ << std::endl;
                        AIpath *nextStep = dfsAIPath(maze, curX+1, curZ, lastX, lastZ);
                        if (nextStep!=NULL) {
							AIpath *toReturn = new AIpath();
							//AIpath *toReturn=malloc(sizeof(AIpath));
                            toReturn->link = nextStep;
                            toReturn->x = curX;
                            toReturn->z = curZ;
                            return toReturn;
                        }
                    }
                    continue;
                case 2:
                    if(maze[curX][curZ-1]==0){
						//std::cout << "curX=" << curX << " curZ=" << curZ << "(-1) lastX=" << lastX << " lastZ=" << lastZ << std::endl;
                        AIpath *nextStep = dfsAIPath(maze, curX, curZ-1, lastX, lastZ);
                        if (nextStep!=NULL) {
							AIpath *toReturn = new AIpath();
							//AIpath *toReturn=malloc(sizeof(AIpath));
                            toReturn->link = nextStep;
                            toReturn->x = curX;
                            toReturn->z = curZ;
                            return toReturn;
                        }
                    }
                    continue;
                case 3:
                    if(maze[curX][curZ+1]==0){
						//std::cout << "curX=" << curX << " curZ=" << curZ << "(+1) lastX=" << lastX << " lastZ=" << lastZ << std::endl;
                        AIpath *nextStep = dfsAIPath(maze, curX, curZ+1, lastX, lastZ);
                        if (nextStep!=NULL) {
							AIpath *toReturn = new AIpath();
							//AIpath *toReturn=malloc(sizeof(AIpath));
                            toReturn->link = nextStep;
                            toReturn->x = curX;
                            toReturn->z = curZ;
                            return toReturn;
                        }
                    }
            }
        }
    }
    return NULL;
}


void depthFirstSearchFunc(int row, int col, int height, int drillFlag){
	//generates a random direction to travel
	int randDirTemp[4];
	int randDir[4];
	int count = 0;
	int hallwayLen = 2; //must be a positive even number less than the maze size
	int i,j,r;
	
	for (i=0; i < 4; i++){ //fills the arrays with {0,1,2,3} with each number representing a direction
		randDirTemp[i] = i;
		randDir[i] = i;
	}
	for (i=0; i < 4; i++){ //randomly rearrange the order of directions
		r = rand()%4;
		if(randDirTemp[r]<0)
			--i;
		else{
			randDir[count] = randDirTemp[r];
			count++;
			randDirTemp[r] =-1;
		}
	}
    
	for (i=0; i < 4; i++){ //goes through and checks each possible direction in the array
		switch (randDir[i]) {
			case 0: //move x value up
				if(row-hallwayLen <= 0) //unable to drill: Out of barrier range
					continue;
				if(world[row-hallwayLen][height][col] != 0){ //path is drillable
					for(j=1; j<= hallwayLen; j++){ //fills the path with 0's. Path's length is based on hallwayLen variable
						world[row-j][height][col] = 0;
					}
					depthFirstSearchFunc(row-hallwayLen, col, height, drillFlag); //recursively calls this function to then start at the new branch
				}
				continue;
			case 1: //move x value down
				if(row+hallwayLen >= MazeSize-1)
					continue;
				if(world[row+hallwayLen][height][col] != 0){
					for(j=1; j<= hallwayLen; j++){
						world[row+j][height][col] = 0;
					}
					depthFirstSearchFunc(row+hallwayLen, col, height, drillFlag);
				}
				continue;
			case 2: //move z value up (left)
				if(col-hallwayLen <= 0)
					continue;
				if(world[row][height][col-hallwayLen] != 0){
					for(j=1; j<= hallwayLen; j++){
						world[row][height][col-j] = 0;
					}
					depthFirstSearchFunc(row, col-hallwayLen, height, drillFlag);
				}
				continue;
			case 3: //move z value down (right)
				if(col+hallwayLen >= MazeSize-1)
					continue;
				if(world[row][height][col+hallwayLen] !=0){
					for(j=1; j<= hallwayLen; j++){
						world[row][height][col+j] = 0;
					}
					depthFirstSearchFunc(row, col+hallwayLen, height, drillFlag);
				}
				continue;
		}
	}
}

void addRooms(int height){
	int i, j, k;
	
	int numOfRooms = (rand()%5) +10; //creates a random number of rooms between 10 and 14 to add
	
	for(k = 0; k < numOfRooms; k++){ //for each room, randomly generate the length and width of each room and where in the maze they appear
		int rowLen = (rand()%7)+5;
		int colLen = (rand()%7)+5;
		
		int ycord = rand()%(MazeSize-rowLen-2)+1; 
		int xcord = rand()%(MazeSize-colLen-2)+1;
		
		//fills room into the maze
		for(i = ycord; i<=ycord+rowLen; i++){
			for(j = xcord; j<=xcord+colLen; j++){
				world[i][height][j] = 0;
			}
		}
	}	
}

float getDistance(CVector3f position1, float x2, float y2, float z2) {
	float distX, distY, distZ;
	distX = x2 - position1.x;
	distY = y2 - position1.y;
	distZ = z2 - position1.z;
	distX = distX * distX;
	distY = distY * distY;
	distZ = distZ * distZ;

	return sqrtf(distX + distY + distZ);
}

void followMob(int mobNum){
	float curX, curY, curZ;
	CPhysicalProp playerProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_Player_ID, playerProp);
	curX = playerProp.position.x;
	curY = playerProp.position.y;
	curZ = playerProp.position.z;


	CPhysicalProp mobProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_mobID[mobNum], mobProp);
	//follows the player
	if (getDistance(playerProp.position, mobProp.position.x, mobProp.position.y, mobProp.position.z) < 5){
		float rangeToMove = 0.02f;
		if (mobProp.position.x + 0.1f < curX)
			mobProp.position.x = mobProp.position.x + rangeToMove;
		else if (mobProp.position.x - 0.1f > curX)
			mobProp.position.x = mobProp.position.x - rangeToMove;

		if (mobProp.position.z + 0.3f < curZ)
			mobProp.position.z = mobProp.position.z + rangeToMove;
		else if (mobProp.position.z - 0.3f > curZ)
			mobProp.position.z = mobProp.position.z - rangeToMove;

		//mobry = atan2f(curX - mobx, curZ - mobz)*(180 / PI);

		g_pFactoryMediator->setPhysicalPropertiesByID(g_mobID[mobNum], mobProp);
	}
	else //player is too far away
	{
		AIFollowFlag[mobNum] = 0;
		playSound("assets/Sound/BuzzFadeOut.mp3");
	}
}

/*** update() ***/
/* background process, it is called when there are no other events */
/* -used to control animations and perform calculations while the  */
/*  system is running */
/* -gravity must also implemented here, duplicate collisionResponse */
void updateMazeRunners() {
	/* sample animation for the test world, don't remove this code */
	/* -demo of animating mobs */
	
	static int initializePathList = 0;
	//first time update is called initialize the path lists
	if (initializePathList == 0) {
		initializePathList = 1;
		cpAIpath[0] = AInextStep[0];
		cpAIpath[1] = AInextStep[1];
		cpAIpath[2] = AInextStep[2];
		cpAIpath[3] = AInextStep[3];
	}

	CPhysicalProp playerProp;
	g_pFactoryMediator->getPhysicalPropertiesByID(g_Player_ID, playerProp);
	int icurX = (int)(playerProp.position.x);
	int icurY = (int)(playerProp.position.y);
	int icurZ = (int)(playerProp.position.z);

	//travelDelay is used to slow down the mobs movement through the maze
	static int travelDelay = 0;
	int travelDelayCap = 30;
	travelDelay++;
	if (travelDelay == travelDelayCap + 1) {
		travelDelay = 0;
	}

	int cleanupTime = 10;
	float mobHeight = 1.0f;
	for (int mobNum = 0; mobNum < numOfMobs; mobNum++){
		CPhysicalProp mobProp;
		g_pFactoryMediator->getPhysicalPropertiesByID(g_mobID[mobNum], mobProp);
		int imobX = (int)(mobProp.position.x);
		int imobZ = (int)(mobProp.position.z);

		if (imobX == icurX && icurY == 1 && imobZ == icurZ) //player has collided with mob
		{
			if (AIFollowFlag[mobNum] == 0) //player attacks while mob is cleaning. Player kills mob
			{
				AIFollowFlag[mobNum] = 2; //can no longer be addressed
				mobProp.position.x = 99.0f;
				mobProp.position.y = 1.0f;
				mobProp.position.z = 99.0f;
				g_pFactoryMediator->setPhysicalPropertiesByID(g_mobID[mobNum], mobProp);
				playSound("assets/Sound/Pain.mp3");
			}
			else if (AIFollowFlag[mobNum] == 1) //player get's killed. Game restarts.
			{
				AIFollowFlag[0] = 0;
				AIFollowFlag[1] = 0;
				AIFollowFlag[2] = 0;
				AIFollowFlag[3] = 0;
				playerProp.position.x = 1.5f;
				playerProp.position.y = 1.5f;
				playerProp.position.z = 1.5f;
				g_pFactoryMediator->setPhysicalPropertiesByID(g_Player_ID, playerProp);
				initializeMaze(MazeHeight - 1, 3);
				playSound("assets/Sound/hahaha.mp3");
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
		}

		if (AIFollowFlag[mobNum] == 0 && travelDelay == travelDelayCap) {
			if (cpAIpath[mobNum] != NULL && cpAIpath[mobNum]->link != NULL) {
				if ((world[imobX][0][imobZ] == 2 && mobCleanup[mobNum] == cleanupTime) || world[imobX][0][imobZ] == 3) {
					mobProp.position.x = cpAIpath[mobNum]->x + 0.5f;
					mobProp.position.y = mobHeight;
					mobProp.position.z = cpAIpath[mobNum]->z + 0.5f;
					
					static const float oneDegreeInRadians = static_cast<float>(PI) / 180.0f;
					float rotation = atan2f(cpAIpath[mobNum]->link->x - cpAIpath[mobNum]->x, cpAIpath[mobNum]->link->z - cpAIpath[mobNum]->z)*(180 / PI);
					//std::cout << rotation << std::endl;
					mobProp.setOrientationEulerAngles(CVector3f(oneDegreeInRadians * 0.0f, oneDegreeInRadians * rotation, oneDegreeInRadians * 0.0f));
					g_pFactoryMediator->setPhysicalPropertiesByID(g_mobID[mobNum], mobProp);
					cpAIpath[mobNum] = cpAIpath[mobNum]->link;
					if (world[imobX][0][imobZ] == 2 && mobCleanup[mobNum] == cleanupTime){
						mobCleanup[mobNum] = 0;
						world[imobX][0][imobZ] = 3;
					}
				}
				else
					mobCleanup[mobNum]++; //cleans the dug up ground beneath it
			}
			else {
				//has reached end of path, will now travel in opposite direction
				if (PathFlag[mobNum] == 0) {
					cpAIpath[mobNum] = AIreturnStep[mobNum];
					PathFlag[mobNum] = 1;
				}
				//has returned to initial starting point, resets for another loop
				else if (PathFlag[mobNum] == 1){
					cpAIpath[mobNum] = AInextStep[mobNum];
					PathFlag[mobNum] = 0;
				}
			}
		}
		else if (AIFollowFlag[mobNum] == 1) //mob will now pursue player
			followMob(mobNum);


		if (cpAIpath[mobNum] != NULL && cpAIpath[mobNum]->link != NULL && AIFollowFlag[mobNum] == 0) {
			//checks the distance from player to bunny
			if (getDistance(playerProp.position, cpAIpath[mobNum]->link->x, 0.5f, cpAIpath[mobNum]->link->z) < 1.0f
				|| getDistance(playerProp.position, cpAIpath[mobNum]->x, 0.5f, cpAIpath[mobNum]->z) < 1.0f)
			{
				AIFollowFlag[mobNum] = 1;
				playSound("assets/Sound/BuzzFadeIn.mp3");
			}
		}

	}	
}



//determines and builds the travelling paths for each mob
void buildMartixPaths(AIpath **nextStep, AIpath **returnStep, int mazeLevel, int startPointX, int startPointZ, int optionX1, int optionZ1, int optionX2, int optionZ2, int optionX3, int optionZ3){
    int randDirTemp[3];
	int randDir[3];
	int count = 0;
	int i,r;
	
	for (i=0; i < 3; i++){ //fills the arrays with {0,1,2} with each number representing a option
		randDirTemp[i] = i;
		randDir[i] = i;
	}
	for (i=0; i < 3; i++){ //randomly rearrange the order of directions
		r = rand()%3;
		if(randDirTemp[r]<0)
			--i;
		else{
			randDir[count] = randDirTemp[r];
			count++;
			randDirTemp[r] =-1;
		}
	}
    
	int maze[MazeSize][MazeSize];
    mazeCopy(maze, mazeLevel);
    
    switch (randDir[0]) {
        case 0: //option 1
            printf("points(%d,%d) travel to (%d,%d)\n", startPointX, startPointZ, optionX1, optionZ1);
            *nextStep = dfsAIPath(maze, startPointX, startPointZ, optionX1, optionZ1);
            mazeCopy(maze, mazeLevel);
            *returnStep = dfsAIPath(maze, optionX1, optionZ1, startPointX, startPointZ);
            break;
        case 1: //option 2
            printf("points(%d,%d) travel to (%d,%d)\n", startPointX, startPointZ, optionX2, optionZ2);
            *nextStep = dfsAIPath(maze, startPointX, startPointZ, optionX2, optionZ2);
            mazeCopy(maze, mazeLevel);
            *returnStep = dfsAIPath(maze, optionX2, optionZ2, startPointX, startPointZ);
            break;
        case 2: //option 3
            printf("points(%d,%d) travel to (%d,%d)\n", startPointX, startPointZ, optionX3, optionZ3);
            *nextStep = dfsAIPath(maze, startPointX, startPointZ, optionX3, optionZ3);
            mazeCopy(maze, mazeLevel);
            *returnStep = dfsAIPath(maze, optionX3, optionZ3, startPointX, startPointZ);
            break;
        default:
            break;
    }
}


int GenerateMaze()
{
	srand((unsigned)time(NULL));
    tStart=clock();
	int i, j, k;
	/* the first part of this if statement builds a sample */
	/* world which will be used for testing */
	/* DO NOT remove this code. */
	/* Put your code in the else statment below */
	
    /* initialize world to empty */
	for(i=0; i<100; i++)
		for(j=0; j<50; j++)
			for(k=0; k<100; k++)
				world[i][j][k] = 0;
        
		/* your code to build the world goes here */
		int startPointX = 51;
		int startPointY = 51;
		initializeMaze(MazeHeight - 1, 3);
		//generates a Maze using a Depth First Search Algorithm
		initializeMaze(MazeHeight, 1);
		depthFirstSearchFunc(startPointX, startPointY, MazeHeight, 0);
        
		
        //******** Assignment 3 section ********
        //randomly generates different points on in the maze, each favoring a particular corner region
        int AX = (rand()%24)*2+1;
        int AZ = (rand()%24)*2+1;
        int BX = (rand()%24)*2+1;
        int BZ = (rand()%24)*2+51;
        int CX = (rand()%24)*2+51;
        int CZ = (rand()%24)*2+1;
        int DX = (rand()%24)*2+51;
        int DZ = (rand()%24)*2+51;

		/*AX = 1;
		AZ = 1;
		BX = 1;
		BZ = 99;
		CX = 99;
		CZ = 1;
		DX = 99;
		DZ = 99;*/
        
        printf("A coordinates (X=%d,Z=%d)\nB coordinates (X=%d,Z=%d)\n", AX, AZ, BX, BZ);
        printf("C coordinates (X=%d,Z=%d)\nD coordinates (X=%d,Z=%d)\n", CX, CZ, DX, DZ);
        
        
        printf("For lower Level maze\n");
        //A pathing for maze 1, will be assigned to find point B, C, or D
        buildMartixPaths(&AInextStep[0], &AIreturnStep[0], MazeHeight, AX, AZ, BX, BZ, CX, CZ, DX, DZ);
        
        //B pathing for maze 1
		buildMartixPaths(&AInextStep[1], &AIreturnStep[1], MazeHeight, BX, BZ, AX, AZ, CX, CZ, DX, DZ);
        
        //C pathing for maze 1
		buildMartixPaths(&AInextStep[2], &AIreturnStep[2], MazeHeight, CX, CZ, AX, AZ, BX, BZ, DX, DZ);
        
        //D pathing for maze 1
		buildMartixPaths(&AInextStep[3], &AIreturnStep[3], MazeHeight, DX, DZ, AX, AZ, BX, BZ, CX, CZ);
        
        //********* end of Assignment 3 section *********
        
                
		//addRooms(MazeHeight2);
		addRooms(MazeHeight);
                

        //creates entrance
        
	return 0; 
}

