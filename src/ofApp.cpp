
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   < Your Name goes Here >
//  Date: <date of last version>


#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
    bWireframe = false;
    bDisplayPoints = false;
    bAltKeyDown = false;
    bCtrlKeyDown = false;
    bLanderLoaded = false;
    bTerrainSelected = true;
    //	ofSetWindowShape(1024, 768);
    cam.setDistance(10);
    cam.setNearClip(.1);
    cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    ofSetVerticalSync(true);
    cam.disableMouseInput();
    ofEnableSmoothing();
    ofEnableDepthTest();
    
    // setup rudimentary lighting
    //
    initLightingAndMaterials();
    
//    mars.loadModel("geo/mars-low-5x-v2.obj");
//    mars.loadModel("geo/moon-houdini.obj");
    mars.loadModel("geo/triangleSubdividedCity.obj");
    mars.setScaleNormalization(false);
    
    // create sliders for testing
    //
    gui.setup();
    gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
    gui.add(bTimingInfo.setup("Timing Info", true));
    bHide = false;
    
    cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;
    
    //  Create Octree for testing.
    //
    float creationStart = ofGetElapsedTimeMillis();
    octree.create(mars.getMesh(0), 20);
    if (bTimingInfo) {
        cout << "Octree creation miliseconds elapsed: " << ofGetElapsedTimeMillis() - creationStart << endl;
    }
    
    testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));
    octree.colors.push_back(ofColor::hotPink);
    octree.colors.push_back(ofColor::yellowGreen);
    octree.colors.push_back(ofColor::cyan);
    octree.colors.push_back(ofColor::green);
    octree.colors.push_back(ofColor::skyBlue);
    octree.colors.push_back(ofColor::red);
    octree.colors.push_back(ofColor::peachPuff);
    octree.colors.push_back(ofColor::purple);
    octree.colors.push_back(ofColor::midnightBlue);
    octree.colors.push_back(ofColor::seaGreen);
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {	
}
//--------------------------------------------------------------
void ofApp::draw() {
    
    ofBackground(ofColor::black);
    
    glDepthMask(false);
    if (!bHide) gui.draw();
    glDepthMask(true);
    
    cam.begin();
    ofPushMatrix();
    if (bWireframe) {                    // wireframe mode  (include axis)
        ofDisableLighting();
        ofSetColor(ofColor::slateGray);
        mars.drawWireframe();
        if (bLanderLoaded) {
            lander.drawWireframe();
            if (!bTerrainSelected) drawAxis(lander.getPosition());
        }
        if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
    }
    else {
        ofEnableLighting();              // shaded mode
        mars.drawFaces();
        ofMesh mesh;
        if (bLanderLoaded) {
            lander.drawFaces();
            if (!bTerrainSelected) drawAxis(lander.getPosition());
            if (bDisplayBBoxes) {
                ofNoFill();
                ofSetColor(ofColor::white);
                for (int i = 0; i < lander.getNumMeshes(); i++) {
                    ofPushMatrix();
                    ofMultMatrix(lander.getModelMatrix());
                    ofRotate(-90, 1, 0, 0);
                    Octree::drawBox(bboxList[i]);
                    ofPopMatrix();
                }
            }
            
            if (bLanderSelected) {
                
                ofVec3f min = lander.getSceneMin() + lander.getPosition();
                ofVec3f max = lander.getSceneMax() + lander.getPosition();
                
                Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
                ofSetColor(ofColor::white);
                Octree::drawBox(bounds);
                
                // draw colliding boxes
                //
                ofSetColor(ofColor::lightBlue);
                for (int i = 0; i < colBoxList.size(); i++) {
                    Octree::drawBox(colBoxList[i]);
                }
            }
        }
    }
    if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
    
    
    
    if (bDisplayPoints) {                // display points as an option
        glPointSize(3);
        ofSetColor(ofColor::green);
        mars.drawVertices();
    }
    
    // highlight selected point (draw sphere around selected point)
    //
    if (bPointSelected) {
        ofSetColor(ofColor::blue);
        ofDrawSphere(selectedPoint, .1);
    }
    
    
    // recursively draw octree
    //
    ofDisableLighting();
    int level = 0;
    //	ofNoFill();
    
    if (bDisplayLeafNodes) {
        octree.drawLeafNodes(octree.root);
        cout << "num leaf: " << octree.numLeaf << endl;
    }
    else if (bDisplayOctree) {
        ofNoFill();
        ofSetColor(ofColor::white);
        octree.draw(numLevels, 0);
    }
    
    // if point selected, draw a sphere
    //
    if (pointSelected) {
        ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
        ofVec3f d = p - cam.getPosition();
        ofSetColor(ofColor::lightGreen);
        ofDrawSphere(p, .02 * d.length());
    }
    ofNoFill();
    octree.draw(octree.root, numLevels, 0);
    
    ofPopMatrix();
    cam.end();
}


// 
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
        case 'B':
        case 'b':
            bDisplayBBoxes = !bDisplayBBoxes;
            break;
        case 'C':
        case 'c':
            if (cam.getMouseInputEnabled()) cam.disableMouseInput();
            else cam.enableMouseInput();
            break;
        case 'F':
        case 'f':
            ofToggleFullscreen();
            break;
        case 'H':
        case 'h':
            break;
        case 'L':
        case 'l':
            bDisplayLeafNodes = !bDisplayLeafNodes;
            break;
        case 'O':
        case 'o':
            bDisplayOctree = !bDisplayOctree;
            break;
        case 'r':
            cam.reset();
            break;
        case 's':
            savePicture();
            break;
        case 't':
            setCameraTarget();
            break;
        case 'u':
            break;
        case 'v':
            togglePointsDisplay();
            break;
        case 'V':
            break;
        case 'w':
            toggleWireframeMode();
            break;
        case OF_KEY_ALT:
            cam.enableMouseInput();
            bAltKeyDown = true;
            break;
        case OF_KEY_CONTROL:
            bCtrlKeyDown = true;
            break;
        case OF_KEY_SHIFT:
            break;
        case OF_KEY_DEL:
            break;
        default:
            break;
    }
}

void ofApp::toggleWireframeMode() {
    bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
    bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
    bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {
    
    switch (key) {
            
        case OF_KEY_ALT:
            cam.disableMouseInput();
            bAltKeyDown = false;
            break;
        case OF_KEY_CONTROL:
            bCtrlKeyDown = false;
            break;
        case OF_KEY_SHIFT:
            break;
        default:
            break;
            
    }
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
    
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    
    // if moving camera, don't allow mouse interaction
    //
    if (cam.getMouseInputEnabled()) return;
    
    // if moving camera, don't allow mouse interaction
    //
    if (cam.getMouseInputEnabled()) return;
    
    // if rover is loaded, test for selection
    //
    if (bLanderLoaded) {
        glm::vec3 origin = cam.getPosition();
        glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
        glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
        
        ofVec3f min = lander.getSceneMin() + lander.getPosition();
        ofVec3f max = lander.getSceneMax() + lander.getPosition();
        
        Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
        bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
        if (hit) {
            bLanderSelected = true;
            mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
            mouseLastPos = mouseDownPos;
            bInDrag = true;
        }
        else {
            bLanderSelected = false;
        }
    }
    else {
        ofVec3f p;
        raySelectWithOctree(p);
    }
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
    ofVec3f mouse(mouseX, mouseY);
    ofVec3f rayPoint = cam.screenToWorld(mouse);
    ofVec3f rayDir = rayPoint - cam.getPosition();
    rayDir.normalize();
    Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
                  Vector3(rayDir.x, rayDir.y, rayDir.z));
    
    float searchStart = ofGetElapsedTimef();
    pointSelected = octree.intersect(ray, octree.root, selectedNode);
    if (bTimingInfo) {
        cout << "Ray intersect search miliseconds elapsed: " << ofGetElapsedTimef()/1000 << endl;
    }
    
    if (pointSelected) {
        pointRet = octree.mesh.getVertex(selectedNode.points[0]);
    }
    return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    
    // if moving camera, don't allow mouse interaction
    //
    if (cam.getMouseInputEnabled()) return;
    
    if (bInDrag) {
        
        glm::vec3 landerPos = lander.getPosition();
        
        glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
        glm::vec3 delta = mousePos - mouseLastPos;
        
        landerPos += delta;
        lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
        mouseLastPos = mousePos;
        
        ofVec3f min = lander.getSceneMin() + lander.getPosition();
        ofVec3f max = lander.getSceneMax() + lander.getPosition();
        
        Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
        
        colBoxList.clear();
        octree.intersect(bounds, octree.root, colBoxList);
        
        
        /*if (bounds.overlap(testBox)) {
         cout << "overlap" << endl;
         }
         else {
         cout << "OK" << endl;
         }*/
        
        
    }
    else {
        ofVec3f p;
        raySelectWithOctree(p);
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {
    
}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
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

void ofApp::savePicture() {
    ofImage picture;
    picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
    picture.save("screenshot.png");
    cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {
    
    ofVec3f point;
    mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
    if (lander.loadModel(dragInfo.files[0])) {
        lander.setScaleNormalization(false);
        //		lander.setScale(.1, .1, .1);
        //	lander.setPosition(point.x, point.y, point.z);
        lander.setPosition(1, 1, 0);
        
        bLanderLoaded = true;
        for (int i = 0; i < lander.getMeshCount(); i++) {
            bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
        }
        
        cout << "Mesh Count: " << lander.getMeshCount() << endl;
    }
    else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
    ofVec2f mouse(mouseX, mouseY);
    ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
    ofVec3f rayDir = rayPoint - cam.getPosition();
    rayDir.normalize();
    return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
    if (lander.loadModel(dragInfo.files[0])) {
        bLanderLoaded = true;
        lander.setScaleNormalization(false);
        lander.setPosition(0, 0, 0);
        cout << "number of meshes: " << lander.getNumMeshes() << endl;
        bboxList.clear();
        for (int i = 0; i < lander.getMeshCount(); i++) {
            bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
        }
        
        //		lander.setRotation(1, 180, 1, 0, 0);
        
        // We want to drag and drop a 3D object in space so that the model appears
        // under the mouse pointer where you drop it !
        //
        // Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
        // once we find the point of intersection, we can position the lander/lander
        // at that location.
        //
        
        // Setup our rays
        //
        glm::vec3 origin = cam.getPosition();
        glm::vec3 camAxis = cam.getZAxis();
        glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
        glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
        float distance;
        
        bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
        if (hit) {
            // find the point of intersection on the plane using the distance
            // We use the parameteric line or vector representation of a line to compute
            //
            // p' = p + s * dir;
            //
            glm::vec3 intersectPoint = origin + distance * mouseDir;
            
            // Now position the lander's origin at that intersection point
            //
            glm::vec3 min = lander.getSceneMin();
            glm::vec3 max = lander.getSceneMax();
            float offset = (max.y - min.y) / 2.0;
            lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);
            
            // set up bounding box for lander while we are at it
            //
            landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
        }
    }
    
    
}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
    // Setup our rays
    //
    glm::vec3 origin = cam.getPosition();
    glm::vec3 camAxis = cam.getZAxis();
    glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
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
