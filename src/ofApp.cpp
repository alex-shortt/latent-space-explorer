#include "ofApp.h"

#include "ofxNI2.h"

ofEasyCam cam;

size_t SCREEN_W;
size_t SCREEN_H;

bool showDebugging = true;
int RAW_DEPTH = false;
int TRANSLATE_Y = 20;
int TRANSLATE_X = 0;
float ZOOM = 1.05;
float TOLERANCE = 0.7;
int NEAR = 0;
int FAR = 2500;

ofVec3f latentPos;

//--------------------------------------------------------------
void ofApp::setup()
{
	SCREEN_W = ofGetScreenWidth();
	SCREEN_H = ofGetScreenHeight();
	
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);
	
	fbo.allocate((int) SCREEN_W, (int) SCREEN_H, GL_RGBA);
	
	texture.allocate(200, 200, OF_IMAGE_COLOR);
	texture.load("latent/0.0-0.0-0.0.jpg");
	
	wrapper.load("movies/waves.mp4");
	wrapper.play();
	
	result.allocate(wrapper.getWidth(), wrapper.getHeight(), OF_IMAGE_COLOR);

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
		
	size_t numUsers = tracker.getNumUser();
		
	for(size_t u = 0; u < numUsers; u++) {
		// only do the first one
		if(u > 1) {
			continue;
		}
		
		ofxNiTE2::User::Ref user = tracker.getUser(u);
		
		ofVec3f center = user->getCenterOfMass();
		
		float x_perc = ofClamp(ofMap(center.x, -700, 400, 0, 1), 0, 1);
		float y_perc = ofClamp(ofMap(center.y, -700, 400, 0, 1), 0, 1);
		float z_perc = ofClamp(ofMap(center.z, -FAR, NEAR, 0, 1), 0, 1);
		
		x_perc = roundf(x_perc * 10) / 10;
		y_perc = roundf(y_perc * 10) / 10;
		z_perc = roundf(z_perc * 10) / 10;
		
		string x_string = ofToString(x_perc, 1);
		string y_string = ofToString(y_perc, 1);
		string z_string = ofToString(z_perc, 1);
		
		if(latentPos.distance(ofVec3f(x_perc, y_perc, z_perc)) > 0){
			latentPos.set(x_perc, y_perc, z_perc);
			texture.load("latent/" + x_string + "-" + y_string + "-" + z_string + ".jpg");
			texture.update();
		}
		
	}
	
//	texture.load("latent/0.0-0.0-0.0.jpg");
	wrapper.update();
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
	ofPixels framePixels = texture.getPixels();
	ofImage *depthImage = new ofImage(tracker.getPixelsRef(NEAR, FAR));
	depthImage->resize((int) framePixels.getWidth(), (int) framePixels.getHeight());
	ofPixels depthPixels = depthImage->getPixels();
	delete depthImage;
	
	result.setColor(ofColor::black);
	
	bool depthReady = depthPixels.size() * 3 == framePixels.size();
	
	// resize/recolor video
	if(depthReady) {
		for (int y = 0; y < framePixels.getHeight(); y++) {
			for (int x = 0; x < framePixels.getWidth(); x++) {
				
				ofColor c = framePixels.getColor(x, y);
				ofColor depth = depthPixels.getColor(x, y);
				
				float dist = depth.r / 255.0;
				
				if (RAW_DEPTH){
					result.setColor( x, y , depth);
				} else if(dist < 1 && dist > TOLERANCE){
					result.setColor( x, y , c);
				}
				
			}
		}
	}
	
	result.update();
	
	fbo.begin();
		ofClear(255,255,255, 0);
		
		// get scale measurments
		float scaleX = (float) SCREEN_W / (float) result.getWidth();
		float scaleY = (float) SCREEN_H / (float) result.getHeight();
		float scale = scaleX > scaleY ? scaleX : scaleY;
		scale *= ZOOM;
		
		// move to center
		ofTranslate(ofGetScreenWidth() / 2, ofGetScreenHeight() / 2);
		ofTranslate(((float) result.getWidth() * scale) / -2.0f, ((float) result.getHeight() * scale) / -2.0f);
	
		// move based on settings
		ofScale(scale);
		ofTranslate(TRANSLATE_X, TRANSLATE_Y);
	
		result.draw(0,0);
    fbo.end();

	fbo.draw(0,0);
	
	size_t numUsers = tracker.getNumUser();
		
	for(size_t u = 0; u < numUsers; u++) {
		// only do the first one
		if(u > 1) {
			continue;
		}
		
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
	
	if (showDebugging){
		tracker.draw();
//		ofDrawBitmapString("X Bounds: " + ofToString(centerMinX) + ", " + ofToString(centerMaxX), 20, ofGetHeight() - 210);
//		ofDrawBitmapString("Y Bounds: " + ofToString(centerMinY) + ", " + ofToString(centerMaxY), 20, ofGetHeight() - 200);
//		ofDrawBitmapString("Z Bounds: " + ofToString(centerMinZ) + ", " + ofToString(centerMaxZ), 20, ofGetHeight() - 180);
		ofDrawBitmapString("RAW_DEPTH (e): " + ofToString(RAW_DEPTH), 20, ofGetHeight() - 160);
		ofDrawBitmapString("TRANSLATE_Y (r+/f-): " + ofToString(TRANSLATE_Y), 20, ofGetHeight() - 140);
		ofDrawBitmapString("TRANSLATE_X (t+/g-): " + ofToString(TRANSLATE_X), 20, ofGetHeight() - 120);
		ofDrawBitmapString("ZOOM (y+/h-): " + ofToString(ZOOM), 20, ofGetHeight() - 100);
		ofDrawBitmapString("TOLERANCE (u+/j-): " + ofToString(TOLERANCE), 20, ofGetHeight() - 80);
		ofDrawBitmapString("NEAR (i+/k-): " + ofToString(NEAR), 20, ofGetHeight() - 60);
		ofDrawBitmapString("FAR (o+/l-): " + ofToString(FAR), 20, ofGetHeight() - 40);
		ofDrawBitmapString("Tracker FPS: "+ofToString(tracker.getFrameRate()),20,ofGetHeight()-20);
		ofDrawBitmapString("Application FPS: "+ofToString(ofGetFrameRate()),20,ofGetHeight());
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
	if (key == 'd') {
		showDebugging = !showDebugging;
	} else if (key == 'i') {
		NEAR += 100;
	} else if (key == 'k') {
		NEAR -= 100;
	} else if (key == 'o') {
		FAR += 100;
	} else if (key == 'l') {
		FAR -= 100;
	} else if (key == 'u') {
	   TOLERANCE += 0.05;
	} else if (key == 'j') {
	   TOLERANCE -= 0.05;
	} else if (key == 'y') {
		ZOOM += 0.05;
	}  else if (key == 'h') {
		ZOOM -= 0.05;
    } else if (key == 't') {
		TRANSLATE_X += 20;
	} else if (key == 'g') {
		TRANSLATE_X -= 20;
	} else if (key == 'r') {
		TRANSLATE_Y += 20;
	} else if (key == 'f') {
		TRANSLATE_Y -= 20;
	} else if (key == 'e') {
		RAW_DEPTH = !RAW_DEPTH;
	}
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
