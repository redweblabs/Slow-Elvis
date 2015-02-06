#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofPixels.h"
#include "ofxLibface.h"
#include "ofxXmlSettings.h"
#include "ofxUI.h"
#include "ofUtils.h"

#define CAMWIDTH 720
#define CAMHEIGHT 480

class testApp : public ofBaseApp{
	
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
    int  calculateAverageX(int nextX);
    int  calculateAverageY(int nextY);
    void drawGUI();
    void resetSettings(vector<string> &settings);
    void resetSettingsToDefault();
    
    ofVideoGrabber 		vidGrabber;
    int 				camWidth = CAMWIDTH;
    int 				camHeight = CAMHEIGHT;
    
    int* columnDiff;
    int* rowDiff;
    
    int **p2DArray;
    
    ofFile settings;

    ofxCv::ObjectFinder finder;
    ofxCv::ObjectFinder faceFinder;
    ofxCv::ObjectFinder shouldersFinder;
    ofxCv::ObjectFinder bodyFinder;
    
    ofxCvColorImage			rawVid;
    unsigned char *         hist;
    ofTexture               histTexture;
    ofxCvGrayscaleImage 	prevFrame;
    ofxCvGrayscaleImage 	greyVid;
    ofxCvGrayscaleImage 	diff;
    
    ofTrueTypeFont ofFont;
    string fontName = "opensans.ttf";
    
    int threshold = 35;
    int personThreshold = 10;
    int minPersonSize = 5;
    int movementThreshold = 40; //How fast I think a person can move
    
    bool havePerson = false;
    
    int leftIndex = 0;
    int lastLeftIndex = leftIndex;
    
    int rightIndex = 0;
    int lastRightIndex = rightIndex;
    
    int topIndex = 0;
    int lastTopIndex = topIndex;
    
    int bottomIndex = 0;
    int lastBottomIndex = 0;
    
    float midX = camWidth / 2;
    int xLength = 40;
    int *previousXs = new int[xLength];
//  int idx = 0;
    
    int midY = camHeight / 2;
    int yLength = 40;
    int *previousYs = new int[yLength];
    
    bool invert = false;
    
    int queenX = camWidth / 2;
    int queenSpeed = 10;
    int queenWidth = 50;
    
    //GUI Variables
    
    int sliderWidth = 100;
    int sliderHeight = 20;
    
    int queenPercent = 25;//((queenWidth / queenMax) * 100);
    int queenMin = 10;
    int queenMax = 200;
    
    int leftPercent = 0;
    int minLeftLimit = 0;
    int currentLeftLimit = 0;
    int maxLeftLimit = camWidth / 2;
    
    int rightPercent = 0;
    int minRightLimit = 0;
    int currentRightLimit = 0;
    int maxRightLimit = camWidth / 2;
    
    int topPercent = 0;
    int minTopLimit = 0;
    int currentTopLimit = 0;
    int maxTopLimit = camHeight / 2;
    
    int bottomPercent = 0;
    int minBottomLimit = 0;
    int currentBottomLimit = 0;
    int maxBottomLimit = camHeight / 2;
    
    int averagesPercent = 40;
    int minAveragesLength = 1;
    int averagesLength = xLength;
    int maxAveragesLength = 300;
    
    int motorX = 50;
    
    string currentSlider = "";
    
    long lastTimeWeSentData = 0;
    
    ofSerial Arduino;
    
};
