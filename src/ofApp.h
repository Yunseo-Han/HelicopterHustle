#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "Octree.h"
#include <glm/gtx/intersect.hpp>
#include "ofxLibwebsockets.h"
#include "PlayerModel.h"
#include "ParticleEmitter.h"


class ofApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y ) {};
    void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h) {};
	void dragEvent(ofDragInfo dragInfo) {};
	void gotMessage(ofMessage msg) {};
	void drawAxis(ofVec3f);
	void initLightingAndMaterials();
	void extractRotationValue(const string& json, const string& key, float& value);
    glm::vec3 getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm);
    bool doPointSelection();
    
    
	// websocket server
	ofxLibwebsockets::Server server;
	bool bSetup;
	vector<string> messages;
	int latency;
	int lastMessage;
	int worstLatency;
	int lastWorst;
	// websocket methods
	void onOpen( ofxLibwebsockets::Event& args );
	void onClose( ofxLibwebsockets::Event& args );
	void onMessage( ofxLibwebsockets::Event& args );
	void onConnect( ofxLibwebsockets::Event& args ){};
	void onIdle( ofxLibwebsockets::Event& args ){};
	void drawConsole();
    
    bool mouseRaySelectWithOctree(ofVec3f &pointRet);
    bool altitudeRaySelectWithOctree(ofVec3f &pointRet);
    

	ofEasyCam easyCam;
	ofCamera followCam;
	ofCamera *theCam;
	ofNode playerNode;

	ofShader particleShader;
	ofShader explosionShader;
	
	ParticleSystem particleSystem;
	vector<ParticleEmitter> bladeTipEmitters;

	ofxAssimpModelLoader mars;
	ofMesh terrainMesh;
	PlayerModel playerModel;
	ofxAssimpModelLoader rotorBladesModel;
    glm::vec3 heldDirection = glm::vec3(0, 0, 0);
	ofLight light;
	Box boundingBox, landerBounds;
	vector<Box> colBoxList;
    vector<int> colPointList;
	bool bLanderSelected = false;
	Octree octree;
	TreeNode selectedNode;
	glm::vec3 mouseDownPos, mouseLastPos;
	bool bInDrag = false;


	ofxIntSlider numLevels;
	ofxToggle timingInfoToggle;
	ofxPanel gui;

	bool bStarted = false;
	bool bAltKeyDown = false;
	bool bCtrlKeyDown = false;
	bool bWireframe = false;
	bool bDisplayPoints = false;
	bool bPointSelected = false;
	bool bHide = false;
	bool pointSelected = false;
	bool bDisplayLeafNodes = false;
	bool bDisplayOctree = false;
	bool bDisplayBBoxes = false;
	bool bLanderLoaded = false;
	bool bTerrainSelected = true;
    bool bDisplayAltitude = true;

	ofVec3f selectedPoint;
	ofVec3f intersectPoint;

	vector<Box> bboxList;

	const float selectionRange = 4.0;
    
    ofVec3f prevPlayerPosition;
    ofVec3f altitudeRayIntersection;
    float altitude;
    bool altitudeExists;
    
    vector<Box> landingPadList;
    // landedArea[1] = true if player landed in landedPadList[1]
    vector<bool> landedAreas;
    
    bool crashed = false;
    bool wonGame = false;
};
