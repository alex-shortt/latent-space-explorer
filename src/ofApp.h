#pragma once

#include "ofMain.h"
#include "ofxNI2.h"
#include "ofxNiTE2.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void exit();
	
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
		// Kinect
		ofxNI2::Device device;
		ofxNiTE2::UserTracker tracker;
		
		// Video
		ofVideoPlayer wrapper;
	
		// fbo
		ofFbo fbo;

		ofImage result;
		ofImage texture;
};
