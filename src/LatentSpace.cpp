//
//  LatentSpace.cpp
//  latent-space-explorer
//
//  Created by Alex Shortt on 3/15/20.
//

#include "LatentSpace.hpp"

LatentSpace::LatentSpace(string directory) {
	outDir = directory;
	position = ofVec3f(0.0f, 0.0f, 0.0f);
	xBounds = ofVec2f(0, 1);
	yBounds = ofVec2f(0, 1);
	zBounds = ofVec2f(0, 1);
	
	image.allocate(200, 200, OF_IMAGE_COLOR);
	image.load(outDir + "/0.0-0.0-0.0.jpg");
}

void LatentSpace::updatePosition(ofVec3f pos) {
	updatePosition(pos.x, pos.y, pos.z);
}

string LatentSpace::getImagePath(){
	string x_string = ofToString(position.x, 1);
	string y_string = ofToString(position.y, 1);
	string z_string = ofToString(position.z, 1);
	
	string coords = x_string + "-" + y_string + "-" + z_string;
	return outDir + "/" + coords + ".jpg";
}

void LatentSpace::updatePosition(float x, float y, float z){
	// get values between 0 and 1
	float x_perc = ofClamp(ofMap(x, xBounds.x, xBounds.y, 0, 1), 0, 1);
	float y_perc = ofClamp(ofMap(y, yBounds.x, yBounds.y, 0, 1), 0, 1);
	float z_perc = ofClamp(ofMap(z, zBounds.x, zBounds.y, 0, 1), 0, 1);
	
	// round to 0.x precision
	x_perc = roundf(x_perc * 10) / 10;
	y_perc = roundf(y_perc * 10) / 10;
	z_perc = roundf(z_perc * 10) / 10;
	
	// save new pos with modified coordinates
	ofVec3f newPos = ofVec3f(x_perc, y_perc, z_perc);
	
	if(newPos.distance(position) > 0){
		position.set(newPos);
		string path = getImagePath();
		image.load(path);
		image.update();
	}
}

void drawIndicator(float x, float y, float val){
	float barWidth = 200;
	float barHeight = 20;
	float indicatorWidth = 6;
	
	ofSetHexColor(0xff0000);
	ofDrawRectangle(x, y, barWidth, barHeight);
	ofSetHexColor(0xffffff);
	ofDrawRectangle(x + (val * barWidth) - (indicatorWidth / 2.0) , y, indicatorWidth, barHeight);
}

void LatentSpace::drawDebug(){
	ofPushStyle();
	
	// draw xyz values
	drawIndicator(20, 20, position.x);
	drawIndicator(20, 50, position.y);
	drawIndicator(20, 80, position.z);
	
	// draw image path
	ofSetHexColor(0xff0000);
	ofDrawBitmapString(getImagePath(), 20, 120);
	
	ofPopStyle();
}
