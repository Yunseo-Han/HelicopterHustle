#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    // Load sprites
    if (bgSkyImage.load("image/sky.png")) {
        cout << "loaded sky" << endl;
    }
    
    // lights
    ofEnableLighting();
    keyLight.setup();
    keyLight.enable();
    keyLight.setAreaLight(1, 1);
    keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
    keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    keyLight.setSpecularColor(ofFloatColor(1, 1, 1));

    keyLight.rotate(45, ofVec3f(0, 1, 0));
    keyLight.rotate(-45, ofVec3f(1, 0, 0));
    keyLight.setPosition(300, 300, 150);

    fillLight.setup();
    fillLight.enable();
    fillLight.setSpotlight();
    fillLight.setScale(1);
//1    fillLight.setSpotlightCutOff(1000);
    fillLight.setAttenuation(2, 1, 1);
    fillLight.setAmbientColor(ofFloatColor(1, 1, 1));
    fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
    fillLight.rotate(-20, ofVec3f(1, 0, 0));
    fillLight.rotate(-45, ofVec3f(0, 1, 0));
    fillLight.setPosition(-157, 200, 300);

    rimLight.setup();
    rimLight.enable();
    rimLight.setSpotlight();
    rimLight.setScale(1);
//    rimLight.setSpotlightCutOff(30);
    rimLight.setAttenuation(1, 1, 1);
    rimLight.setAmbientColor(ofFloatColor(1, 1, 1));
    rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
    rimLight.rotate(180, ofVec3f(0, 1, 0));
    rimLight.setPosition(62, 100, -407);
    
    

    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 9092;
	options.bUseSSL = true; // you'll have to manually accept this self-signed cert if 'true'!
    bSetup = server.setup( options );
    
    // this adds your app as a listener for the server
    server.addListener(this);
    
	// position the player node
    playerNode.setGlobalPosition(0, 80, 0);

	easyCam.setGlobalPosition(playerNode.getGlobalPosition() + glm::vec3(5, 5, -20));
	easyCam.setTarget(playerNode);	
	easyCam.setDistance(10);
	easyCam.setNearClip(.1);
	easyCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	easyCam.disableMouseInput();

	skyCam.setGlobalPosition(playerNode.getGlobalPosition() + glm::vec3(60, 110, -160));
	skyCam.setTarget(playerNode);	
	skyCam.setNearClip(.1);
	skyCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	skyCam.disableMouseInput();

	followCam.setNearClip(.1);
	followCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	followCam.setParent(playerNode);
    followCam.setPosition(0, 5, 10);
    followCam.lookAt(playerNode);

    theCam = &easyCam;

	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();

	// setup rudimentary lighting 
	//
//	initLightingAndMaterials();

//	string marsObjPath = "geo/scaledPlane.obj";
    mars.loadModel("geo/RemeshedNoLandingPads.obj");
    // mars.loadModel("geo/mars-low-5x-v2.obj", true); // TODO: should we set optimize to true?
	mars.setScaleNormalization(false);


	terrainMesh = mars.getMesh(0);

	motorSound.load("sounds/motor_sound.mp3");
	motorSound.setLoop(true);
	motorSound.setVolume(.1);
    motorSound.play();

	popSound.load("sounds/pop_sound.mp3");

	collisionSound.load("sounds/collision_sound.mp3");

    destructionSound.load("sounds/destruction_sound.mp3");
	destructionSound.setVolume(0.7);

    // load helicopter models
    playerModel.loadModel("geo/helicopterbody.obj");
    bLanderLoaded = true;

    playerModel.setScaleNormalization(false);
    playerModel.setPosition(playerNode.getGlobalPosition());
    
	rotorBladesModel.loadModel("geo/rotorblades.obj");
	rotorBladesModel.setScaleNormalization(false);
    rotorBladesModel.setPosition(playerNode.getGlobalPosition().x, playerNode.getGlobalPosition().y, playerNode.getGlobalPosition().z);
    
	glm::vec3 offset(2.6, 0.8, 0);
	for (int i = 0; i < 4; ++i) {
		ParticleEmitter tipEmitter(&particleSystem);
		tipEmitter.init();
		tipEmitter.radius = 0.1;
		tipEmitter.setRate(200);
		tipEmitter.setLifespan(1);
		tipEmitter.setParticleRadius(0.01);
		tipEmitter.setVelocity(glm::vec3(0, -0.01, 0));
		tipEmitter.start();
		bladeTipEmitters.push_back(tipEmitter);
	}

	explosionEmitter.init();
	explosionEmitter.setOneShot(true);
	explosionEmitter.setLifespan(10);
	explosionEmitter.setVelocity(ofVec3f(3));
	explosionEmitter.setEmitterType(EmitterType::RadialEmitter);
	explosionEmitter.setGroupSize(400);

    cout << "number of meshes in Terrain: " << mars.getNumMeshes() << endl;
    cout << "number of meshes in playerModel: " << playerModel.getNumMeshes() << endl;
    
//    TODO: REVIEW
    bboxList.clear();
    for (int i = 0; i < playerModel.getMeshCount(); i++) {
        bboxList.push_back(Octree::meshBounds(playerModel.getMesh(i)));
    }
    
    // set up landing pads
    landingPadList.clear();
    landingPadList.push_back(Box(Vector3(-26, 69, -25), Vector3(26, 70, 26)));
    landingPadList.push_back(Box(Vector3(16, 153, -210), Vector3(88, 154, -140)));
    landingPadList.push_back(Box(Vector3(-209, 8, 150), Vector3(-150, 9, 205)));
    
    landedAreas.clear();
    landedAreas.push_back(false);
    landedAreas.push_back(false);
    landedAreas.push_back(false);
    
    
    playerModel.setCollisionBox();
    
	
	cout << "Building octree..." << endl;
	ofResetElapsedTimeCounter();
	octree.create(mars.getMesh(0), 23);
	cout << "Time to build octree (ms): " << ofGetElapsedTimeMicros() / 1000.0 << endl;

	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	bHide = false;

	particleShader.load("shaders/particle");
	cout << (particleShader.isLoaded() ? "particleShader loaded" : "particleShader not loaded") << endl;
	explosionShader.load("shaders/explosion");
}

//--------------------------------------------------------------
void ofApp::update(){
    wonGame = landedAreas[0] && landedAreas[1] && landedAreas[2];
    
	playerModel.yaw(-heldDirection.x);
	playerNode.panDeg(-heldDirection.x);

	particleSystem.update();
	explosionEmitter.sys->update();
    explosionEmitter.sys->integrate();

	ofPoint pos = playerModel.getPosition();
	playerNode.setPosition(pos);
	easyCam.setTarget(playerNode);	
	skyCam.setTarget(playerNode);

	// orient rotor blades in correct position with helicopter body
	rotorBladesModel.setPosition(pos.x, pos.y, pos.z);
	rotorBladesModel.setRotation(0, playerModel.getRotationAngle(0), 0, 1, 0);
	rotorBladesModel.setRotation(1, playerModel.getRotationAngle(1), 0, 0, 1);
	rotorBladesModel.setRotation(2, playerModel.getRotationAngle(2), 1, 0, 0);

	// rotate blades based on throttle percent
	rotorBladesModel.setRotation(3, rotorBladesModel.getRotationAngle(3) + (10 * playerModel.throttlePercent), 0, 1, 0);


	for (int i = 0; i < bladeTipEmitters.size(); ++i) {
		// orient blade tip emitters along with player rotation and rotor spin
		glm::vec3 offset = glm::vec3(2.6, 0.8, 0); // point to first blade tip
		offset = glm::rotateY(offset, glm::radians(90.0f * i)); // rotate to current tip

		glm::mat4 matrix = rotorBladesModel.getModelMatrix();
		glm::vec3 tipPosition = matrix * glm::vec4(offset, 1.0f);

		bladeTipEmitters[i].setPosition(tipPosition);
		
		bladeTipEmitters[i].setRate(300 * playerModel.throttlePercent);
		bladeTipEmitters[i].update();
	}

    // playerMovement
    if (!bInDrag) {
        playerModel.throttlePercent += heldDirection.y / 500;
        playerModel.throttlePercent = ofClamp(playerModel.throttlePercent, 0, 1);

	// only rotational movement is allowed while game is not started
	if (!bStarted) return;
        
    if (crashed) {
		explosionEmitter.update();
		return;
	}
        
    if (wonGame) return;

	motorSound.setVolume(playerModel.throttlePercent);

	glm::vec3 thrustForce = playerModel.thrust() * playerModel.getYawAxis();
	playerModel.force += thrustForce;

        float dragCoef = -3;
        glm::vec3 dragForce = dragCoef * glm::length2(playerModel.velocity) * playerModel.velocity;
        playerModel.force += dragForce;

        glm::vec3 gravityForce(0, -playerModel.mass, 0);
        playerModel.force += gravityForce;

        playerModel.integrate();
        
        ofPoint pos = playerModel.getPosition();
        playerNode.setPosition(pos);
        rotorBladesModel.setPosition(pos.x, pos.y, pos.z);
        rotorBladesModel.setRotation(0, playerModel.getRotationAngle(0), 0, 1, 0);
        rotorBladesModel.setRotation(1, playerModel.getRotationAngle(1), 0, 0, 1);
        rotorBladesModel.setRotation(2, playerModel.getRotationAngle(2), 1, 0, 0);
        rotorBladesModel.setRotation(3, rotorBladesModel.getRotationAngle(3) + (10 * playerModel.throttlePercent), 0, 1, 0);
    }
	
	// ofPoint pos = playerModel.getPosition();
	// playerNode.setPosition(pos);
	// rotorBladesModel.setPosition(pos.x, pos.y, pos.z);
	// rotorBladesModel.setRotation(0, playerModel.getRotationAngle(0), 0, 1, 0);
	// rotorBladesModel.setRotation(1, playerModel.getRotationAngle(1), 0, 0, 1);
	// rotorBladesModel.setRotation(2, playerModel.getRotationAngle(2), 1, 0, 0);
	// rotorBladesModel.setRotation(3, rotorBladesModel.getRotationAngle(3) + (10 * playerModel.throttlePercent), 0, 1, 0);
    
    // finding altitude of lander from terrain
    altitudeExists = altitudeRaySelectWithOctree(altitudeRayIntersection);
    ofVec3f altitudeRay = playerModel.getPosition() - altitudeRayIntersection;
    altitude = glm::length(glm::vec3(altitudeRay.x, altitudeRay.y, altitudeRay.z));
    
    
    
    // collision
    playerModel.setCollisionBox();
    colBoxList.clear();
    colPointList.clear();
    if (octree.intersect(playerModel.collisionBox, octree.root, colBoxList, colPointList)) {
        // Landing area check
        for (int i=0; i<landingPadList.size(); i++) {
            if (landingPadList[i].overlap(playerModel.collisionBox)) {
                // check player velocity
                if (playerModel.velocity.y < -4) {
                    cout << playerModel.velocity.y << endl;
                    crashed = true;
					destructionSound.play();

					explosionEmitter.setPosition(playerModel.getPosition());
					explosionEmitter.start();
					explosionEmitter.update();
					explosionEmitter.sys->update();
                }
                
                landedAreas[i] = true;
            }
        }
        
        // adjust player position to non-colliding state
        playerModel.setPosition(prevPlayerPosition);
        
        // calculate the surface normal
        // start with a vertical impulse because normalizing a zero-length vector causes big problems
        glm::vec3 surfaceNormal(0, 1, 0);
        for (int i=0; i<colPointList.size(); i++) {
            surfaceNormal += terrainMesh.getNormal(colPointList[i]);
        }
        surfaceNormal = glm::normalize(surfaceNormal);
        
		float landingSpeed = glm::length2(playerModel.velocity);
		cout << "landing speed: " << landingSpeed << endl;

        collisionSound.setVolume(ofMap(landingSpeed, 0, 30, 0, 0.7));
		collisionSound.play();

        
        // old collision response??
//        // restitution is 0.9
//        glm::vec3 collisionResponse = (1 + 0.9) * (glm::dot(-playerModel.velocity, surfaceNormal) * surfaceNormal) * ofGetFrameRate();
//        playerModel.force = collisionResponse;
        
        
        
		// playerModel.setPosition(playerModel.getPosition() + surfaceNormal * 0.01);
		playerModel.velocity *= -0.9;

		float restitution = 15000;
        glm::vec3 collisionResponse = (1 + restitution) * (glm::dot(-playerModel.velocity, surfaceNormal) * surfaceNormal);

        playerModel.force += collisionResponse;
        
        // Landing area check
        for (int i=0; i<landingPadList.size(); i++) {
            if (landingPadList[i].overlap(playerModel.collisionBox)) {
				if (!landedAreas[i]) popSound.play();
                landedAreas[i] = true;
            }
        }
    } else {
        prevPlayerPosition = playerModel.getPosition();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    glDepthMask(false);
    ofBackground(ofColor::white);
    ofSetColor(ofColor::white);
    bgSkyImage.draw(0, 0, ofGetWidth(), ofGetHeight());
    glDepthMask(true);
    
    glDepthMask(false);
        if (!bHide) gui.draw();
        
        ofSetColor(ofColor::white);

        if (!bStarted) ofDrawBitmapString("Press space to start", 400, 400);
    
    if (crashed) {
        ofDrawBitmapString("CRASHED", 400, 400);
    }
    
    if (wonGame) {
        ofDrawBitmapString("SUCCESSFUL LANDING IN ALL AREAS", 400, 400);
    }

        string serverStatus = bSetup ? "WebSocket server setup at " + ofToString(server.getPort()) : "WebSocket setup failed :(";
        ofDrawBitmapString(serverStatus, 750, 740);

        ofDrawBitmapString("latency: " + ofToString(latency), 750, 760);
        ofDrawBitmapString("worst: " + ofToString(worstLatency), 860, 760);

        drawConsole();

        ofSetColor(ofColor::black);
        ofDrawRectangle(50, 660, 5, 100);
        ofDrawRectangle(60, 660, 5, 100);

        ofSetColor(ofColor::silver);
        ofDrawRectangle(35, 755 - (100 * playerModel.throttlePercent), 45, 10);
    
    ofSetColor(ofColor::green);
    
    if (bDisplayAltitude) {
        ofDrawBitmapString("altitude: " + (altitudeExists? ofToString(altitude) : "unknown"), 80, 760);
    }

    glDepthMask(true);

	theCam->begin();
    
	ofPushMatrix();
    
    keyLight.draw();
    fillLight.draw();
    rimLight.draw();
    
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
//        ofEnableAlphaBlending();
		mars.drawFaces();
		ofMesh mesh;
        
        

		if (bLanderLoaded) {
			playerModel.drawFaces();
			rotorBladesModel.drawFaces();
            
			ofSetColor(ofColor::gray);
			for (int i = 0; i < bladeTipEmitters.size(); ++i) bladeTipEmitters[i].draw();

			ofEnableBlendMode(OF_BLENDMODE_ADD);
				particleShader.begin();
					for (int i = 0; i < particleSystem.particles.size(); i++) {
						float age = particleSystem.particles[i].age();
						particleShader.setAttribute1f(1, age);
						ofSetColor(255, 255, 255, ofMap(age, 0, 1, 255 * playerModel.throttlePercent, 0));
						particleSystem.particles[i].draw();
					}
				particleShader.end();
			ofDisableBlendMode();

			if (crashed) {
				ofEnableBlendMode(OF_BLENDMODE_ADD);
					explosionShader.begin();
						ofSetColor(ofColor::red);
						explosionEmitter.sys->draw();
					explosionShader.end();
				ofDisableBlendMode();
			}
            
            
            ofNoFill();
            ofSetColor(ofColor::white);
            // draw landing pads
            for (int i = 0; i < landingPadList.size(); i++) {
                if (landedAreas[i]) {
                    ofSetColor(ofColor::teal);
                } else {
                    ofSetColor(ofColor::red);
                }
                Octree::drawBox(landingPadList[i]);
            }
            
            ofFill();

			if (!bTerrainSelected) drawAxis(playerModel.getPosition());
            
            if (bDisplayAltitude) {
                ofNoFill();
                ofSetColor(ofColor::green);
                ofDrawLine(playerModel.getPosition(), altitudeRayIntersection);
            }
            
            ofFill();

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
                
				ofNoFill();
				ofSetColor(ofColor::green);
				Octree::drawBox(playerModel.collisionBox);

//                ofSetcolor(ofColor::white)
				ofSetColor(ofColor::teal);
                
                // draw colliding boxes
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}

				ofFill();
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
	case '3':
        theCam = &skyCam;
        break;
    case 'A':
    case 'a':
        bDisplayAltitude = !bDisplayAltitude;
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
	case OF_KEY_ESC:
		ofExit();
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
void ofApp::mousePressed(int x, int y, int button) {
    
    // if moving camera, don't allow mouse interaction
    //
    if (easyCam.getMouseInputEnabled()) return;
    
    // if moving camera, don't allow mouse interaction
    //
    if (easyCam.getMouseInputEnabled()) return;
    
    // if rover is loaded, test for selection
    //
    if (bLanderLoaded) {
        // selecting lander with mouse
        glm::vec3 origin = easyCam.getPosition();
        glm::vec3 mouseWorld = easyCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
        glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
        
        
        bool hit = playerModel.collisionBox.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
        if (hit) {
            bLanderSelected = true;
            // TODO: RE-IMPLEMENT DRAGGING
            mouseDownPos = getMousePointOnPlane(playerModel.getPosition(), easyCam.getZAxis());
            mouseLastPos = mouseDownPos;
            bInDrag = true;
        }
        else {
            bLanderSelected = false;
        }
    }
}

bool ofApp::mouseRaySelectWithOctree(ofVec3f &pointRet) {
    ofVec3f mouse(mouseX, mouseY);
    ofVec3f rayPoint = easyCam.screenToWorld(mouse);
    ofVec3f rayDir = rayPoint - easyCam.getPosition();
    rayDir.normalize();
    Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
                  Vector3(rayDir.x, rayDir.y, rayDir.z));
    
    float searchStart = ofGetElapsedTimef();
    pointSelected = octree.intersect(ray, octree.root, selectedNode);
    
    if (pointSelected) {
        pointRet = octree.mesh.getVertex(selectedNode.points[0]);
    }
    return pointSelected;
}

bool ofApp::altitudeRaySelectWithOctree(ofVec3f &pointRet) {
    ofVec3f rayPoint = playerModel.getPosition();
    ofVec3f rayDir = ofVec3f(0, -1, 0);
    rayDir.normalize();
    Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
                  Vector3(rayDir.x, rayDir.y, rayDir.z));
    
    pointSelected = octree.intersect(ray, octree.root, selectedNode);
    
    if (pointSelected) {
        pointRet = octree.mesh.getVertex(selectedNode.points[0]);
    }
    return pointSelected;
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

void ofApp::mouseDragged(int x, int y, int button) {
    
    // if moving camera, don't allow mouse interaction
    //
    if (easyCam.getMouseInputEnabled()) return;
    
    if (bInDrag) {
        
        glm::vec3 landerPos = playerModel.getPosition();
        
        glm::vec3 mousePos = getMousePointOnPlane(landerPos, easyCam.getZAxis());
        glm::vec3 delta = mousePos - mouseLastPos;
        
        landerPos += delta;
        playerModel.setPosition(landerPos);
        mouseLastPos = mousePos;
        
        playerModel.setCollisionBox();
        
        colBoxList.clear();
        octree.intersect(playerModel.collisionBox, octree.root, colBoxList);
        
        
        /*if (bounds.overlap(testBox)) {
         cout << "overlap" << endl;
         }
         else {
         cout << "OK" << endl;
         }*/
        
        
    }
    else {
        ofVec3f p;
        mouseRaySelectWithOctree(p);
    }
}


//  intersect the mouse ray with the plane normal to the camera
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
    // Setup our rays
    //
    glm::vec3 origin = easyCam.getPosition();
    glm::vec3 camAxis = easyCam.getZAxis();
    glm::vec3 mouseWorld = easyCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
    glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
    float distance;
    
    bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);
    
    if (hit) {
        // find the point of intersection on the plane using the distance
        // We use the parameteric line or vector representation of a line to compute
        //
        // p' = p + s * dir;
        //
        glm::vec3 intersectPoint = origin + distance * mouseDir;
        
        return intersectPoint;
    }
    else return glm::vec3(0, 0, 0);
}

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


