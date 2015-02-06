#include "testApp.h"

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void testApp::setup(){
	
    ofEnableSmoothing();
    
    settings.open(ofToDataPath("settings.txt"), ofFile::ReadWrite, false);
    
    ofBuffer settingsBuff = settings.readToBuffer();
    
    vector<string> settingsLines = ofSplitString(ofToString(settingsBuff), "\n");
    
    resetSettings(settingsLines);
    
    //we can now get back a list of devices.
	vector<ofVideoDevice> devices = vidGrabber.listDevices();
	
    for(int i = 0; i < devices.size(); i++){
		cout << devices[i].id << ": " << devices[i].deviceName;
        if( devices[i].bAvailable ){
            cout << endl;
        }else{
            cout << " - unavailable " << endl;
        }
	}
    
	vidGrabber.setDeviceID(0);
	vidGrabber.setDesiredFrameRate(30);
	vidGrabber.initGrabber(camWidth,camHeight);
	
    rawVid.allocate(camWidth, camHeight);
	greyVid.allocate(camWidth, camHeight);
    diff.allocate(camWidth, camHeight);
    
    columnDiff = new int[camWidth];
    rowDiff = new int[camHeight];
    
    for(int c = 0; c < xLength; c++){
        previousXs[c] = camWidth / 2;
    }
    
    ofFont.setGlobalDpi(145);
    ofFont.setSpaceSize(0.6);
    ofFont.loadFont(fontName, 6);
    
	ofSetVerticalSync(true);
    ofSetWindowTitle("The King");
    
    ofSerial().listDevices();

    Arduino.setup(0, 115200);
    Arduino.drain();
    
    lastTimeWeSentData = ofGetElapsedTimeMillis();
}


//--------------------------------------------------------------
void testApp::update(){
	
	ofBackground(100,100,100);
    
    vidGrabber.update();
    
    if(vidGrabber.isFrameNew()){
        
        prevFrame = greyVid;
        greyVid = rawVid;
        
        rawVid.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);
        
        //Downstairs
        //Our Camera is upside down
        rawVid.mirror(true, false);
        
        diff.absDiff(prevFrame, greyVid);
		diff.threshold(threshold);
        
    }
    
    if(averagesLength != xLength){
        xLength = averagesLength;
        previousXs = new int[xLength];
        
        for(int c = 0; c < xLength; c++){
            previousXs[c] = camWidth / 2;
        }
        
    }
    
    unsigned char * pixels = diff.getPixels();
    
    //Right, let's sort out the columns;
    for(int x = 0; x < camWidth; x++){
        
        int thisColumnDifference = 0;
        
        for(int y = 0; y < camHeight; y++){
            
            int pos = x + y * camWidth;
            
            if(pixels[pos] == 255 && y > currentTopLimit){
                thisColumnDifference++;
            }
            
        }
        
        columnDiff[x] = thisColumnDifference;
        
        for (int z = 0; z < thisColumnDifference; z++) {
            
            int pos = x + z * camWidth;
            
        }
        
    }
    
    //And now let's sort out the rows;
    for(int y = 0; y < camHeight; y++){
        
        int thisRowDifference = 0;
        
        for(int x = 0; x < camWidth; x++){
            
            int pos = x + y * camWidth;
            
            if(pixels[pos] == 255){
                thisRowDifference++;
            }
            
            
        }
        
        rowDiff[y] = thisRowDifference;
        
    }
    
    for(int x = 0; x < camWidth; x++){
        
        for (int z = 0; z < columnDiff[x]; z++) {
            
            int pos = x + z * camWidth;
            
            pixels[pos] = 255;
            pixels[pos + 1] = 0;
            pixels[pos + 2] = 0;
            
        }
        
    }
    
    for(int y = 0; y < camHeight; y++){
        
        for (int z = 0; z < rowDiff[y]; z++) {
            
            int pos = z + y * camWidth;
            
            pixels[pos] = 255;
            pixels[pos + 1] = 0;
            pixels[pos + 2] = 0;
            
        }
        
    }
    
    for(int i = 2; i < camWidth - 2; i++) {
        columnDiff[i] = (columnDiff[i - 1] + columnDiff[i - 2] + columnDiff[i] + columnDiff[i + 1] + columnDiff[i + 2]) / 5;
    }
    
    for(int j = 2; j < camHeight - 2; j++) {
        rowDiff[j] = (rowDiff[j - 1] + rowDiff[j - 2] + rowDiff[j] + rowDiff[j + 1] + rowDiff[j + 2]) / 5;
    }
    
    int currentLeftIndex = 0;
    
    for(int l = 0; l < camWidth; l++){
        
        if(columnDiff[l] > personThreshold && columnDiff[l] > columnDiff[l + 1] && columnDiff[l] > columnDiff[currentLeftIndex]){
            currentLeftIndex = l;
        }
        
    }
    
    leftIndex = currentLeftIndex;
    
    int currentRightIndex = 0;
    
    for(int r = camWidth - 1; r > leftIndex + 1; r--){
        
        if(columnDiff[r] > personThreshold && columnDiff[r] > columnDiff[r - 1] && columnDiff[r] > columnDiff[currentRightIndex]){
            currentRightIndex = r;
        }
        
    }
    
    rightIndex = currentRightIndex;
    
    int currentTopIndex = 0;
    
    for(int t = 0; t < camHeight - 1; t++){
        
        if(rowDiff[t] > rowDiff[t + 1] && rowDiff[t] > rowDiff[currentTopIndex]){
            currentTopIndex = t;
        }
        
    }
    
    topIndex = currentTopIndex;
    
    int currentBottomIndex = 0;
    
    for(int b = camHeight - 1; b > topIndex + 1; b--){
        
        if(rowDiff[b] > rowDiff[b - 1] && rowDiff[b] > rowDiff[currentBottomIndex]){
            currentBottomIndex = b;
        }
    
    }
    
    bottomIndex = currentBottomIndex;
    
    if(rightIndex - leftIndex > minPersonSize){
        havePerson = true;
        
        midX = leftIndex + ((rightIndex - leftIndex) / 2);
        
        int total = 0;
        
        midX = calculateAverageX(midX);
        
        if(midX < currentLeftLimit){
            midX = currentLeftLimit;
        } else if (midX > camWidth - currentRightLimit){
            midX = (camWidth - currentRightLimit) - 1;
        }
        
        midY = topIndex + ((bottomIndex - topIndex) / 2);
        
        midY = calculateAverageY(midY);
        
        if(midY < currentTopLimit){
            midY = currentTopLimit;
        } else if(midY > camHeight - currentBottomLimit){
            midY = (camHeight - currentBottomLimit) - 1;
        }
        
    } else {
        havePerson = false;
    }
    
    if(queenX < midX){
    
        if(midX - queenX < queenSpeed){
            queenX += midX - queenX;
        } else {
            queenX += queenSpeed;
        }
    
    } else if(queenX > midX){
        if(queenX - midX > queenSpeed){
            queenX -= queenX - midX;
        } else {
            queenX -= queenSpeed;
        }
    }
    
    if(queenX + (queenWidth / 2) > camWidth - currentRightLimit){
        queenX = (camWidth - currentRightLimit) - (queenWidth / 2);
    } else if(queenX - (queenWidth / 2) < currentLeftLimit){
        queenX = currentLeftLimit + (queenWidth / 2);
    }
    
    motorX = ((midX  - currentLeftLimit) / ((camWidth - currentRightLimit)  - currentLeftLimit)) * 100;
    
    unsigned char buf[4] = {'0', '0', '0', '\n'};
    
    int value = motorX;
    int idx = 2;
    
    while (value > 0) {
        int digit = value % 10;
        
        int i = digit;
        char c = i + '0';
        
        buf[idx] = c;
        
        idx--;
        
        value /= 10;
        
    }
    
    if(Arduino.isInitialized() && ofGetElapsedTimeMillis() - lastTimeWeSentData > 500){
        Arduino.flush(true, false);
        Arduino.writeBytes(&buf[0], 4);
        lastTimeWeSentData = ofGetElapsedTimeMillis();
    }
    
    
}//Ends update()

void testApp::draw(){
	ofSetHexColor(0xffffff);
    
    diff.draw(0, 0);
    
    ofSetLineWidth(1);
    
    if(havePerson){
        
        ofSetColor(255, 0, 0);
        ofLine(leftIndex, 0, leftIndex, camHeight);
        
        ofSetColor(255, 255, 0);
        ofLine(rightIndex, 0, rightIndex, camHeight);
        
        ofSetColor(155,0,200);
        ofLine(0, topIndex, camWidth, topIndex);
        
        ofSetColor(155,155,200);
        ofLine(0, bottomIndex, camWidth, bottomIndex);
        	
    }
    
    //Where the averaged middleX and middleY values are
    ofSetColor(0, 255, 0);
    ofLine(midX, 0, midX, camHeight); // The middle X
    ofLine(0, midY, camWidth, midY); // Thy middlest y
    
    //Where the Queen is now
    ofSetColor(255, 0, 255, 150);
    ofRect(queenX - (queenWidth / 2), 0, queenWidth, camHeight);
    ofSetColor(255, 0, 255);
    ofLine(queenX, 0, queenX, camHeight);
    
    //Left, Right, Top, Bottom Limit
    ofSetColor(255,0,0,100);
    ofRect(0, 0, currentLeftLimit, camHeight);
    ofRect(camWidth - currentRightLimit, 0, camWidth, camHeight);
    ofRect(0, 0, camWidth, currentTopLimit);
    ofRect(0, camHeight - currentBottomLimit, camWidth, camHeight);
    
    ofSetColor(255, 0, 0, 255);
    ofLine(currentLeftLimit, 0, currentLeftLimit, camHeight);
    ofLine(camWidth - currentRightLimit, 0, camWidth - currentRightLimit, camHeight);
    ofLine(0, currentTopLimit, camWidth, currentTopLimit);
    ofLine(0, camHeight - currentBottomLimit, camWidth, camHeight - currentBottomLimit);
    
    ofSetColor(0, 0, 0);
    
    drawGUI();
    
}



void testApp::drawGUI(){
    
    ofRect(camWidth, 0, camWidth + 200, camHeight);
    
    //Draw backgrounds
    ofSetColor(39, 70, 94);
    
    //Thickness of Queen
    ofRect(CAMWIDTH + 10, 10, sliderWidth, sliderHeight);
    //Left offset
    ofRect(CAMWIDTH + 10, 40, sliderWidth, sliderHeight);
    //Right offset
    ofRect(CAMWIDTH + 10, 70, sliderWidth, sliderHeight);
    //Top Offset
    ofRect(CAMWIDTH + 10, 100, sliderWidth, sliderHeight);
    //Bottom Offset
    ofRect(CAMWIDTH + 10, 130, sliderWidth, sliderHeight);
    //Averages
    ofRect(CAMWIDTH + 10, 160, sliderWidth, sliderHeight);
    //Reset
    ofRect(CAMWIDTH + 10, (CAMHEIGHT - sliderHeight) - 10, sliderWidth, sliderHeight);
    
    //Draw active foreground
    ofSetColor(70, 127, 171);
    
    //Current Queen thickness
    ofRect(CAMWIDTH + 10, 10, queenPercent, sliderHeight);
    //Current left offset
    ofRect(CAMWIDTH + 10, 40, leftPercent, sliderHeight);
    //Current right offset
    ofRect(CAMWIDTH + 10, 70, rightPercent, sliderHeight);
    //Current top offset;
    ofRect(CAMWIDTH + 10, 100, topPercent, sliderHeight);
    //Current bottom offset
    ofRect(CAMWIDTH + 10, 130, bottomPercent, sliderHeight);
    //Current averages length
    ofRect(CAMWIDTH + 10, 160, averagesPercent, sliderHeight);
    
    //Draw current values
    ofSetColor(255, 255, 255);
    
    //Queen
    ofFont.drawString(ofToString(queenWidth), CAMWIDTH + 15, 24);
    ofFont.drawString("Queen Thickness", CAMWIDTH + 10 + sliderWidth + 4, 24);
    //Left
    ofFont.drawString(ofToString(currentLeftLimit), CAMWIDTH + 15, 54);
    ofFont.drawString("Left limit", CAMWIDTH + 10 + sliderWidth + 4, 54);
    //Right
    ofFont.drawString(ofToString(currentRightLimit), CAMWIDTH + 15, 84);
    ofFont.drawString("Right limit", CAMWIDTH + 10 + sliderWidth + 4, 84);
    //Top
    ofFont.drawString(ofToString(currentTopLimit), CAMWIDTH + 15, 114);
    ofFont.drawString("Top limit", CAMWIDTH + 10 + sliderWidth + 4, 114);
    //Bottom
    ofFont.drawString(ofToString(currentBottomLimit), CAMWIDTH + 15, 144);
    ofFont.drawString("Bottom limit", CAMWIDTH + 10 + sliderWidth + 4, 144);
    //Averages
    ofFont.drawString(ofToString(averagesLength), CAMWIDTH + 15, 174);
    ofFont.drawString("Previous averages", CAMWIDTH + 10 + sliderWidth + 4, 174);
    //Reset to defaults
    ofFont.drawString("Reset values", CAMWIDTH + 14, (CAMHEIGHT - sliderHeight) + 4);
    
    
}

int testApp::calculateAverageX(int nextX){
    
    int finalX = 0;
    
    for(int v = 0; v < sizeof(previousXs); v++){
        previousXs[v] = previousXs[v + 1];
    }
    
    previousXs[sizeof(previousXs)] = nextX;
    
    for(int w = 0; w < sizeof(previousXs); w++){
        finalX += previousXs[w];
    }
    
    return finalX / sizeof(previousXs);
    
}

int testApp::calculateAverageY(int nextY){
    
    int finalY = 0;
    
    for(int v = 0; v < sizeof(previousYs); v++){
        previousYs[v] = previousYs[v + 1];
    }
    
    previousYs[sizeof(previousYs)] = nextY;
    
    for(int w = 0; w < sizeof(previousYs); w++){
        finalY += previousYs[w];
    }
    
    return finalY / sizeof(previousYs);
    
}

void testApp::resetSettings(vector<string> &settings){
    
    for(int f = 0; f < settings.size() - 1; f += 1){
        
        string thisSetting = ofToString(settings[f]);
        vector<string> keyValue = ofSplitString(thisSetting, ",");
        
        string key = ofToString(keyValue[0]);
        int val = ofToInt(keyValue[1]);
        
        if(key == "QUEEN"){
            queenPercent = val;
            queenWidth = ((queenMax / 100) * val) + queenMin;
        }
        
        if(key == "LEFT"){
            leftPercent = val;
            currentLeftLimit = ((maxLeftLimit / 100) * val) + minLeftLimit;
        }
        
        if(key == "RIGHT"){
            rightPercent = val;
            currentRightLimit = ((maxRightLimit / 100) * val) + minRightLimit;
        }
        
        if(key == "TOP"){
            topPercent = val;
            currentTopLimit = ((maxTopLimit / 100) * val) + minTopLimit;
        }
        
        if(key == "BOTTOM"){
            bottomPercent = val;
            currentBottomLimit = ((maxBottomLimit / 100) * val) + minBottomLimit;
        }
        
        if(key == "AVERAGES"){
            averagesPercent = val;
            averagesLength = ((maxAveragesLength / 100) * val) + minAveragesLength;
        }
        
    }
    
}

void testApp::resetSettingsToDefault(){
    
    queenPercent = 25;
    queenWidth = ((queenMax / 100) * queenPercent) + queenMin;
    
    leftPercent = 0;
    currentLeftLimit = ((maxLeftLimit / 100) * leftPercent) + minLeftLimit;
    
    rightPercent = 0;
    currentRightLimit = ((maxRightLimit / 100) * rightPercent) + minRightLimit;
    
    topPercent = 0;
    currentTopLimit = ((maxTopLimit / 100) * topPercent) + minTopLimit;
    
    bottomPercent = 0;
    currentBottomLimit = ((maxBottomLimit / 100) * bottomPercent) + minBottomLimit;
    
    averagesPercent = 40;
    averagesLength = ((maxAveragesLength / 100) * averagesPercent) + minAveragesLength;
    
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	
}


//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
    if(currentSlider != ""){
        
        int percent = x - (camWidth + 10);
        
        if(percent <= 100 && percent >= 0){
            
            if(currentSlider == "QUEEN"){
                
                queenPercent = percent;
                queenWidth = ((queenMax / 100) * percent) + queenMin;
                
                if(queenWidth > queenMax){
                    queenWidth = queenMax;
                }
                
                if(queenWidth < queenMin){
                    queenWidth = queenMin;
                }
                
            }
            
            if(currentSlider == "LEFT"){
                
                leftPercent = percent;
                currentLeftLimit = ((maxLeftLimit / 100) * percent) + minLeftLimit;
                
                if(currentLeftLimit > maxLeftLimit){
                    currentLeftLimit = maxLeftLimit;
                }
                
                if(currentLeftLimit < minLeftLimit){
                    currentLeftLimit = minLeftLimit;
                }
                
            }
            
            if(currentSlider == "RIGHT"){
                
                rightPercent = percent;
                currentRightLimit = ((maxRightLimit / 100) * percent) + minRightLimit;
                
                if(currentRightLimit > maxRightLimit){
                    currentRightLimit = maxRightLimit;
                }
                
                if(currentRightLimit < minRightLimit){
                    currentRightLimit = minRightLimit;
                }
                
            }
            
            if(currentSlider == "TOP"){
                
                topPercent = percent;
                currentTopLimit = ((maxTopLimit / 100) * percent) + minTopLimit;
                
                if(currentTopLimit > maxTopLimit){
                    currentTopLimit = maxTopLimit;
                }
                
                if(currentTopLimit < minTopLimit){
                    currentTopLimit = minTopLimit;
                }
                
            }
            
            if(currentSlider == "BOTTOM"){
                
                bottomPercent = percent;
                currentBottomLimit = ((maxBottomLimit / 100) * percent) + minBottomLimit;
                
                if(currentBottomLimit > maxBottomLimit){
                    currentBottomLimit = maxBottomLimit;
                }
                
                if(currentBottomLimit < minBottomLimit){
                    currentBottomLimit = minBottomLimit;
                }
                
            }
            
            if(currentSlider == "AVERAGES"){
                
                averagesPercent = percent;
                averagesLength = ((maxAveragesLength / 100) * percent) + minAveragesLength;
                
                if(averagesLength > maxAveragesLength){
                    averagesLength = maxAveragesLength;
                }
                
                if(averagesLength < minAveragesLength){
                    averagesLength = minAveragesLength;
                }
                
            }
            
        }
        
        
    }
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
    //cout << x;
	
    if (x > CAMWIDTH + 10 && x < CAMWIDTH + 10 + sliderWidth) {
        //Right, We're in the right area for there to be a slider
        //Now we need to determine which one it is.
        
        if(y > 10 && y < 10 + sliderHeight){
            currentSlider = "QUEEN";
        }
        
        if(y > 40 && y < 40 + sliderHeight){
            currentSlider = "LEFT";
        }
        
        if(y > 70 && y < 70 + sliderHeight){
            currentSlider = "RIGHT";
        }
        
        if(y > 100 && y < 100 + sliderHeight){
            currentSlider = "TOP";
        }
        
        if(y > 130 && y < 130 + sliderHeight){
            currentSlider = "BOTTOM";
        }
        
        if(y > 160 && y < 160 + sliderHeight){
            currentSlider = "AVERAGES";
        }
        
        if(y > 450 && y < 450 + sliderHeight){
            resetSettingsToDefault();
        }
        
    }
    
}

void testApp::exit(){
    
    ofBuffer outputSettings;
    
    outputSettings.append("QUEEN," + ofToString(queenPercent) + "\n");
    outputSettings.append("LEFT," + ofToString(currentLeftLimit) + "\n");
    outputSettings.append("RIGHT," + ofToString(currentRightLimit) + "\n");
    outputSettings.append("TOP," + ofToString(currentTopLimit) + "\n");
    outputSettings.append("BOTTOM," + ofToString(currentBottomLimit) + "\n");
    outputSettings.append("AVERAGES," + ofToString(averagesPercent) + "\n");
    
    ofBufferToFile("settings.txt", outputSettings);
    
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
    currentSlider = "";
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}
