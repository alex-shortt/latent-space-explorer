#include "ofApp.h"

// cover image onto passed image, centered
void coverOnImage(ofImage &child, ofImage &parent){
	// find scale factor
	float scaleX = parent.getWidth() / child.getWidth();
	float scaleY = parent.getHeight() / child.getHeight();
	float scale = scaleX > scaleY ? scaleX : scaleY;
	
	// find new dimensions, apply to child
	float newWidth = child.getWidth() * scale;
	float newHeight = child.getHeight() * scale;
	child.resize(newWidth, newHeight);
	
	// find space offset
	float diffW = (newWidth - parent.getWidth()) / 2.0f;
	float diffH = (newHeight - parent.getHeight()) / 2.0f;
	
	// apply space offset to crop
	child.crop(diffW, diffH, parent.getWidth(), parent.getHeight());
}

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);
	
	// GUI
	gui.setup();
	gui.add(RAW_DEPTH.setup("Raw Depth", false));
	gui.add(TRANSLATE.setup("Translate", {-97.9, 36.7}, {-200, -200}, {200, 200}));
	gui.add(ZOOM.setup("Zoom", 1.62, 0, 3));
	gui.add(DEPTH.setup("Depth (NEAR, FAR)", {-440, 2500}, {-1000, 500}, {1000, 4500}));
	gui.add(DEPTH_TOLERANCE.setup("Depth Tolerance", 0.5f, 0, 1));
		
	// Latent Space data
	latentSpace.setXBounds(-700, 450);
	latentSpace.setYBounds(-200, 150);
	latentSpace.setZBounds(-DEPTH->x, DEPTH->y);
	
	// Fbo / textures
	fbo.allocate(ofGetScreenWidth(), ofGetScreenHeight(), GL_RGBA);
	result.allocate(640, 480, OF_IMAGE_COLOR);
	
	// Kinect setup
	device.setLogLevel(OF_LOG_NOTICE);
	device.setup(0);
	tracker.setup(device);
}

void ofApp::exit()
{
	tracker.exit();
    device.exit();
}

//--------------------------------------------------------------
void ofApp::update()
{
	device.update();
	
	// update latent space based on first user
	size_t numUsers = tracker.getNumUser();
	for(size_t u = 0; u < numUsers; u++) {
		if(u > 1) continue; // only do the first one
		ofxNiTE2::User::Ref user = tracker.getUser(u);
		ofVec3f center = user->getCenterOfMass();
		latentSpace.updatePosition(center);
	}
}

ofVec4f getUserBounds(ofxNiTE2::User::Ref user, float minConfidence){
	ofVec4f bounds;
	
	size_t numJoints = user->getNumJoints();
	for(size_t i = 0; i < numJoints; i++){
		ofxNiTE2::Joint joint = user->getJoint(i);
		float confidence = joint.getPositionConfidence();
		glm::vec3 position = joint.getPosition();
		
		if(confidence > minConfidence){
			ofSetHexColor(0x0000ff);
			ofDrawCircle(position.x, position.y, 5);
		}
		
		if(i == 0){
			bounds = ofVec4f(position.x, position.y, position.x, position.y);
		} else if(confidence > minConfidence) {
			if(position.x < bounds.x) bounds.x = position.x;
			if(position.y < bounds.y) bounds.y = position.y;
			if(position.x > bounds.z) bounds.z = position.x;
			if(position.y > bounds.w) bounds.w = position.y;
		}
	}
	
	return bounds;
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// setup video frame, deptht data
	ofImage depthImage = ofImage(tracker.getPixelsRef(DEPTH->x, DEPTH->y));
	ofPixels depthPixels = depthImage.getPixels();
	
	ofImage latentImage = ofImage(latentSpace.getImage());
	coverOnImage(latentImage, depthImage); // resize latent image to depth image
	ofPixels latentPixels = latentImage.getPixels();
	
	result.setColor(ofColor::black);
	
	// resize/recolor video
	for (int y = 0; y < latentPixels.getHeight(); y++) {
		for (int x = 0; x < latentPixels.getWidth(); x++) {
			
			ofColor c = latentPixels.getColor(x, y);
			ofColor depth = depthPixels.getColor(x, y);
			
			float dist = depth.r / 255.0;
			
			if (RAW_DEPTH){
				result.setColor( x, y , depth);
			} else if(dist < 1 && dist > DEPTH_TOLERANCE){
				result.setColor( x, y , c);
			}
			
		}
	}
	
	result.update();
	
	fbo.begin();
		ofClear(255,255,255, 0);
		
		// get scale measurments
		float scaleX = (float) ofGetScreenWidth()/ (float) result.getWidth();
		float scaleY = (float) ofGetScreenHeight()/ (float) result.getHeight();
		float scale = scaleX > scaleY ? scaleX : scaleY;
		scale *= ZOOM;
		
		// move to center
		ofTranslate(ofGetScreenWidth() / 2, ofGetScreenHeight() / 2);
		ofTranslate(((float) result.getWidth() * scale) / -2.0f, ((float) result.getHeight() * scale) / -2.0f);
	
		// move based on settings
		ofScale(scale);
		ofTranslate(TRANSLATE->x, TRANSLATE->y);
		
		result.mirror(false, true);
		result.draw(0,0);
    fbo.end();

	fbo.draw(0,0);
	
	size_t numUsers = tracker.getNumUser();
		
	for(size_t u = 0; u < numUsers; u++) {
		if(u > 1) continue; // only do the first one
		
		ofxNiTE2::User::Ref user = tracker.getUser(u);
		
		ofVec3f center = user->getCenterOfMass();
		
		ofPushMatrix();
			ofTranslate(center);
		
			ofVec4f bounds = getUserBounds(user, 0.3);
			ofVec2f topLeft = ofVec2f(bounds.x, bounds.y);
			ofVec2f bottomRight = ofVec2f(bounds.z, bounds.w);
			float height = bottomRight.y - topLeft.y;
			float width = bottomRight.x - topLeft.x;
		
			ofSetHexColor(0xff0000);
		
			ofDrawLine(topLeft.x, topLeft.y, bottomRight.x, topLeft.y);
			ofDrawLine(bottomRight.x, topLeft.y, bottomRight.x, bottomRight.y);
			ofDrawLine(bottomRight.x, bottomRight.y, topLeft.x, bottomRight.y);
			ofDrawLine(topLeft.x, bottomRight.y, topLeft.x, topLeft.y);
	//		ofDrawRectangle(center.x + topLeft.x, center.y + topLeft.y, width, height);
		ofPopMatrix();
		
		ofSetHexColor(0x00ff00);
		ofDrawCircle(center, 4);
		
		ofSetHexColor(0xffffff);
	}
	
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
