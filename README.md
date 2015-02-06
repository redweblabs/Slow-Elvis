# Slow-Elvis
The code and libraries for Slow Elvis, Our lifesized copy of The King that follows you around.

###Running the openFrameworks tracking app

Using openFrameworks ( >= 0.8.0) project creator, generate a new project and then replace the 'src' folder with the 'src' folder from this repo. This should allow you to compile and view the tracking app.

####OS X Mavericks

If you're running Mavericks, you may have problems compiling the code if you're using XCode 6+. If you already have the OS X 10.8 SDK in XCode 6, choose it as your deployment target and it should work fine, otherwise, you'll probably need to download XCode 5.1.1 and compile it from there.

###Running the Arduino Sketch

Just like any other Arduino Sketch, copy the contents of our 'Sketch' folder to wherever you want to run it from on your system. We've created custom versions of the Adafruit MotorShield library that allow us to kill the motors if they hit the edge of our platform. Copy the 'Adafruit_MotorShield_kill' folder into your libraries folder and then upload the sketch. Once the program is run, it will calibrate itself by travelling the length of the track and counting how many steps it takes with the stepper motors to traverse the full length. Elvis will then move to the center of the track and will move relative to his last position.

###Hardware

For Slow Elvis, we built a track using two arcade buttons, an Arudino Uno, a Nema 14 Stepper motor and the Adafruit Motor Shield V2. We don't have a circuit diagram at this time (we assembled the circuit over a year ago) but we will do in the future.