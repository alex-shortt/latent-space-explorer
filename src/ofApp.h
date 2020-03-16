#pragma once

#include "ofMain.h"
#include "ofxNI2.h"
#include "ofxNiTE2.h"
#include "ofxGui.h"
#include "LatentSpace.hpp"

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
	
		// GUI for params
		ofxPanel gui;
	
		ofxToggle RAW_DEPTH;
		ofxVec2Slider TRANSLATE;
		ofxFloatSlider ZOOM;
		ofxFloatSlider DEPTH_TOLERANCE; //0.7
		ofxVec2Slider DEPTH; //0, 2500
	
		// Kinect
		ofxNI2::Device device;
		ofxNiTE2::UserTracker tracker;
		
		// Latent space data
		LatentSpace latentSpace = LatentSpace("latent");
	
		// fbo
		ofFbo fbo;
		ofImage result;
};
