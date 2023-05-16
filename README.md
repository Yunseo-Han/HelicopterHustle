# Helicopter Hustle

This helicopter landing game uses your phone as a joystick!

It is built with OpenFrameworks and uses WebSockets to send your phone's orientation data received from the Device Orientation API to the game server to control the helicopter's pitch and roll.


## How to run

* Clone this repo into your OpenFrameworks/apps/myApps directory
* Copy the addon ofxLibwebsockets into your OpenFrameworks/addons directory
* Open the game in your IDE and build/run, or alternatively, open a terminal and run the commands `make` followed by `make RunRelease`
* If you receive an alert about opening connections, accept it
* At this point, the game should be running but waiting on you to press the spacebar to start the game and you should be able to use the arrow keys to turn the helicopter


## Connect your phone as a joystick

* From your phone's web browser, navigate to the url `https://{your-ip-here}:9092`
    * To find your IP on mac you can open the network system settings, click WiFi, and then details, and your IP should be at the bottom of that page.
* Click the button to start sending your phone's orientation data, you should see the helicopter pitch and roll in sync with your phone!
