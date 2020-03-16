//
//  LatentSpace.hpp
//  latent-space-explorer
//
//  Created by Alex Shortt on 3/15/20.
//

#ifndef LatentSpace_hpp
#define LatentSpace_hpp

#include "ofMain.h"
#include <stdio.h>

class LatentSpace {
	public:
		LatentSpace(string dir);
		ofImage getImage(){ return image; }
		void updatePosition(float x, float y, float z);
		void updatePosition(ofVec3f pos);
		void setXBounds(float min, float max) { xBounds = ofVec2f(min, max); }
		void setYBounds(float min, float max) { yBounds = ofVec2f(min, max); }
		void setZBounds(float min, float max) { zBounds = ofVec2f(min, max); }
	
	private:
		// bounds
		ofVec2f xBounds;
		ofVec2f yBounds;
		ofVec2f zBounds;
		
		ofVec3f position;
		ofImage image;
		string outDir;
};

#endif /* LatentSpace_hpp */
