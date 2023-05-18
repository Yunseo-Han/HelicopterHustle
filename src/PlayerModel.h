
#include "ofMain.h"
#include "ofxAssimpModelLoader.h"


class PlayerModel : public ofxAssimpModelLoader {
public:
    void integrate() {
        float framerate = ofGetFrameRate();
        if (framerate == 0) return;
        float dt = 1.0f / framerate;


        // Your code
        //
        acceleration = mass == 0 ? glm::vec3(0) : force / mass; // F=ma -> a=F/m
        velocity += acceleration * dt; // dv = a*t
        // velocity *= damping;
        glm::vec3 newPosition = getPosition() + (velocity * dt);


        setPosition(newPosition);
        force = glm::vec3(0);

        // integrating on angular force
        // float angularAcceleration = angularMass == 0 ? 0.0f : angularForce / angularMass;
        // angularVelocity += angularAcceleration * dt;
        // angularVelocity *= damping;
        // rotation += angularVelocity * dt;
    };

	glm::vec3 heading() {
        
    };
    
    void setCollisionBox(){
        ofVec3f min = this->getSceneMin() + pos;
        ofVec3f max = this->getSceneMax() + pos;
        collisionBox = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
    }
	
    void setPosition(glm::vec3 pos) {
        ofxAssimpModelLoader::setPosition(pos.x, pos.y, pos.z);
    }

    glm::vec3 getYawAxis() {
        glm::vec3 axis = glm::vec3(0, 1, 0);
        axis = glm::rotateZ(axis, glm::radians(getRotationAngle(1)));
        axis = glm::rotateX(axis, glm::radians(getRotationAngle(2)));
        axis = glm::rotateY(axis, glm::radians(getRotationAngle(0)));
        return axis;
    }

    void yaw(float deg) {
        setYaw(getRotationAngle(0) + deg);
    }

    void setYaw(float deg) {
        ofxAssimpModelLoader::setRotation(0, deg, 0, 1, 0);
    }

    void setRoll(float deg) {
        ofxAssimpModelLoader::setRotation(1, deg, 0, 0, 1);
    }

    void setPitch(float deg) {
        ofxAssimpModelLoader::setRotation(2, deg, 1, 0, 0);
    }

    float thrust() {
        return maxThrust * throttlePercent;
    }

	glm::vec3 velocity = glm::vec3(0, 0, 0);
	glm::vec3 acceleration = glm::vec3(0, 0, 0);
	glm::vec3 force = glm::vec3(0, 0, 0);

	float mass = 500.0;
	float damping = .99;
	float angularMass = 2.0; // moment of inertia
	float angularVelocity = 0;
	float angularForce = 0;
    
    float maxThrust = 1500;
    float throttlePercent = 0.33;
    
    Box collisionBox;

};
