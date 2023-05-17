#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 9092;
	options.bUseSSL = true; // you'll have to manually accept this self-signed cert if 'true'!
    bSetup = server.setup( options );
    
    // this adds your app as a listener for the server
    server.addListener(this);
    

	easyCam.setDistance(10);
	easyCam.setNearClip(.1);
	easyCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	easyCam.disableMouseInput();

	followCam.setPosition(0, 25, 0);
	followCam.lookAt(glm::vec3(0, 0, 0));
	followCam.setNearClip(.1);
	followCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format

    theCam = &easyCam;

	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

//	string marsObjPath = "geo/scaledPlane.obj";
    mars.loadModel("geo/mars-low-5x-v2.obj");
    // mars.loadModel("geo/mars-low-5x-v2.obj", true); // TODO: should we set optimize to true?
//	mars.loadModel(marsObjPath);
	mars.setScaleNormalization(false);


	terrainMesh = mars.getMesh(0);

    // position the player node
    playerNode.setGlobalPosition(0, 0, 0);
	// playerNode.pan(180);

    // load helicopter models
    playerModel.loadModel("geo/helicopter.obj");
    bLanderLoaded = true;

    playerModel.setScaleNormalization(false);
    playerModel.setPosition(playerNode.getGlobalPosition());
    // playerModel.setRotation(playerNode.getOrientationEuler());
    // playerModel.turnAround(); // turn it around
    
    cout << "number of meshes in Terrain: " << mars.getNumMeshes() << endl;
    cout << "number of meshes in playerModel: " << playerModel.getNumMeshes() << endl;
    
//    TODO: REVIEW
    bboxList.clear();
    for (int i = 0; i < playerModel.getMeshCount(); i++) {
        bboxList.push_back(Octree::meshBounds(playerModel.getMesh(i)));
    }
    
    glm::vec3 min = playerModel.getSceneMin();
    glm::vec3 max = playerModel.getSceneMax();
    landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));


    followCam.setParent(playerNode);
    followCam.setPosition(0, 5, 10);
    followCam.lookAt(playerNode);
	
	cout << "Building octree..." << endl;
	ofResetElapsedTimeCounter();
	octree.create(mars.getMesh(0), 20);
	cout << "Time to build octree (ms): " << ofGetElapsedTimeMicros() / 1000.0 << endl;

	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(timingInfoToggle.setup("Log Timing Info", false));
	bHide = false;
}

//--------------------------------------------------------------
void ofApp::update(){
	playerModel.yaw(-heldDirection.x);
	playerNode.panDeg(-heldDirection.x);
	
	if (!bStarted) return;

	glm::vec3 thrustForce = heldDirection.y * 10 * playerModel.getYawAxis();
	playerModel.force += thrustForce;

	glm::vec3 gravityForce(0, -1.5, 0);
	playerModel.force += gravityForce;

	playerModel.integrate();

	playerNode.setPosition(playerModel.getPosition());
    
    ofVec3f min = playerModel.getSceneMin() + playerModel.getPosition();
    ofVec3f max = playerModel.getSceneMax() + playerModel.getPosition();
    
    Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
    
    // collision
    colBoxList.clear();
    colPointList.clear();
    if (octree.intersect(bounds, octree.root, colBoxList, colPointList)) {
        // start with a vertical impulse because normalizing a zero-length vector causes big problems
	    glm::vec3 impulseNormal(0, 1, 0); 

        for (int i=0; i<colPointList.size(); i++) {
            impulseNormal += terrainMesh.getNormal(colPointList[i]);
        }

        impulseNormal = glm::normalize(impulseNormal);

        playerModel.force += impulseNormal;
   }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(ofColor::black);

	theCam->begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			playerModel.drawWireframe();
			if (!bTerrainSelected) drawAxis(playerModel.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;

		if (bLanderLoaded) {
			playerModel.drawFaces();

			if (!bTerrainSelected) drawAxis(playerModel.getPosition());

			if (bDisplayBBoxes) {
				ofNoFill();
                
				ofSetColor(ofColor::white);
				for (int i = 0; i < playerModel.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(playerModel.getModelMatrix());
//					ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
                

				ofVec3f min = playerModel.getSceneMin() + playerModel.getPosition();
				ofVec3f max = playerModel.getSceneMax() + playerModel.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofNoFill();
				ofSetColor(ofColor::green);
				Octree::drawBox(bounds);

//                ofSetcolor(ofColor::white)
				ofSetColor(ofColor::teal); // draw colliding boxes
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}

	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	if (bDisplayPoints) { // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	if (bPointSelected) { // highlight selected point (draw sphere around selected point)
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}
	
	// ofDisableLighting(); // recursively draw octree
	// int level = 0;

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
    }
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	ofPopMatrix();
	theCam->end();

	glDepthMask(false);
        if (!bHide) gui.draw();
        
        ofSetColor(ofColor::white);

		if (!bStarted) ofDrawBitmapString("Press space to start", 400, 400);

        string serverStatus = bSetup ? "WebSocket server setup at " + ofToString(server.getPort()) : "WebSocket setup failed :(";
        ofDrawBitmapString(serverStatus, 750, 740);

		ofDrawBitmapString("latency: " + ofToString(latency), 750, 760);
		ofDrawBitmapString("worst: " + ofToString(worstLatency), 860, 760);

        drawConsole();
	glDepthMask(true);
}

void ofApp::drawConsole() {
    int x = 20;
    int y = 100;
    
    ofSetColor(0,150,0);
    ofDrawBitmapString("Console", x, 80);
    
    ofSetColor(255);
    for (int i = messages.size() -1; i >= 0; i-- ){
        ofDrawBitmapString( messages[i], x, y );
        y += 20;
    }
    while (messages.size() > 20) messages.erase( messages.begin() );
}

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {
	switch (key) {
	case '1':
        theCam = &easyCam;
        break;
    case '2':
        theCam = &followCam;
        break;
    case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
        easyCam.getMouseInputEnabled() ? easyCam.disableMouseInput() : easyCam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
        bHide = !bHide;
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
    case 'R':
	case 'r':
		easyCam.reset();
		break;
    case 'V':
	case 'v':
		bDisplayPoints = !bDisplayPoints;
		break;
     case 'W':
	 case 'w':
	 	bWireframe = !bWireframe;
	 	break;
	case OF_KEY_ALT:
		easyCam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case ' ':
		bStarted = true;
		break;
	}

	if (heldDirection.y == 0) {
        if (key == OF_KEY_UP) heldDirection.y = 1;
        else if (key == OF_KEY_DOWN) heldDirection.y = -1;
    }
    if (heldDirection.x == 0) {
        if (key == OF_KEY_RIGHT) heldDirection.x = 1;
        else if (key == OF_KEY_LEFT) heldDirection.x = -1;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    switch (key) {
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_ALT:
        bAltKeyDown = false;
        easyCam.disableMouseInput();
		break;
	default:
		break;
	}

	if (key == OF_KEY_UP || key == OF_KEY_DOWN) heldDirection.y = 0;
    else if (key == OF_KEY_RIGHT || key == OF_KEY_LEFT) heldDirection.x = 0;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bInDrag = false;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor

// void ofApp::dragEvent(ofDragInfo dragInfo){
//     if (!playerModel.loadModel(dragInfo.files[0])) return;
//     bLanderLoaded = true;
//     playerModel.setScaleNormalization(false);
//     playerModel.ofxAssimpModelLoader::setPosition(0, 0, 0);
//     cout << "number of meshes: " << playerModel.getNumMeshes() << endl;
//     bboxList.clear();
//     for (int i = 0; i < playerModel.getMeshCount(); i++) {
//         bboxList.push_back(Octree::meshBounds(playerModel.getMesh(i)));
//     }
//
//     glm::vec3 origin = followCam.getPosition();
//     glm::vec3 camAxis = followCam.getZAxis();
//     glm::vec3 mouseWorld = followCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
//     glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
//     float distance;
//
//     bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
//     if (!hit) return;
//
//     // find the point of intersection on the plane using the distance
//     // We use the parameteric line or vector representation of a line to compute
//     //
//     // p' = p + s * dir;
//     //
//     glm::vec3 intersectPoint = origin + distance * mouseDir;
//
//     // Now position the lander's origin at that intersection point
//     //
//     glm::vec3 min = playerModel.getSceneMin();
//     glm::vec3 max = playerModel.getSceneMax();
//     float offset = (max.y - min.y) / 2.0;
//     playerModel.ofxAssimpModelLoader::setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);
//     playerNode.setGlobalPosition(playerModel.getPosition());
//
//     landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
//
//     followCam.setParent(playerNode);
// }

//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

/*  WEBSOCKET SERVER METHODS  */
//--------------------------------------------------------------
void ofApp::onOpen( ofxLibwebsockets::Event& args ){
    cout << "New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName() << endl;
    messages.push_back("New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName() );
}

//--------------------------------------------------------------
void ofApp::onClose( ofxLibwebsockets::Event& args ){
    cout << "Connection closed." << endl;
    messages.push_back("Connection closed");
}

//--------------------------------------------------------------
void ofApp::onMessage( ofxLibwebsockets::Event& args ){
    // cout << "Received message: " << args.message << endl;

	latency = (ofGetElapsedTimeMicros() / 1000.0f) - lastMessage;
	if (latency > worstLatency || ofGetElapsedTimeMillis() - lastWorst > 1000.0f) {
		worstLatency = latency;
		lastWorst = ofGetElapsedTimeMillis();
	}
    
    if (!bLanderLoaded) return;

    // https://developer.mozilla.org/en-US/docs/Web/API/Device_orientation_events/Orientation_and_motion_data_explained
    // Extract the rotation values from the JSON object
    float beta = 0.0f, gamma = 0.0f;
    extractRotationValue(args.message, "beta", beta);
    extractRotationValue(args.message, "gamma", gamma);

	playerModel.setRoll(-gamma);
	playerModel.setPitch(beta);

	lastMessage = ofGetElapsedTimeMicros() / 1000.0f;
}


void ofApp::extractRotationValue(const string& json, const string& key, float& value) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos != string::npos) {
        pos = json.find(":", pos);
        size_t valueStart = json.find_first_not_of(" \t\n\r\f\b", pos + 1);
        size_t valueEnd = json.find_first_of(",}\n", valueStart);
        string valueStr = json.substr(valueStart, valueEnd - valueStart);
        value = stof(valueStr);
    }
}
