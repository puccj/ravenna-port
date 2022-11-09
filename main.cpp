#include "cam.h"

#include <iostream>

int main() {
  int craneHeight = 2225;
  int camDistance = 723;

  Cam right("sample.mp4");
  Cam left("sample2.mp4");
  right.calculateBackground();
  left.calculateBackground();
  Cam::showTrackbars();
  
  right.setOrigin();
  left.setOrigin();
  right.setScale();
  left.setScale();
  //set again origin to move it on top-left corner
  right.setOrigin();
  left.setOrigin();
  
  while (true) {
    if (!right.process(false, true, true))
      break;
    if (!left.process(false, true, true))
      break;

    right.fit(true);
    left.fit(true);
   
    double distR = right.distance(true);
    double distL = left.distance(true);

    if (!right.show(15))
      break;
    if (!left.show(15))
      break;

    //calculate height:
    double diff = abs(distR - distL);
    double height = diff * craneHeight / (camDistance + diff);

    std::cout << "Height: " << height << '\n';
  }

  return 0;
}