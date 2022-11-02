#include "cam.h"

#include <iostream>

int main() {

  int craneHeight = 600;
  int camDistance = 100;

  Cam right("sample.mp4");
  Cam left("sample2.mp4");
  right.calculateBackground();
  left.calculateBackground();
  Cam::showTrackbars();
  
  //right.setOrigin();
  //right.setScale();
  
  while (true) {
    
    if (!right.process(false, true, true))
      break;
    if (!left.process(false, true, true))
      break;

    right.fit(true);
    left.fit(true);
    
    cv::Point posR = right.position(true);
    right.putText(std::to_string(posR.x) + ", " + std::to_string(posR.y));
    
    cv::Point posL = left.position(true);
    left.putText(std::to_string(posL.x) + ", " + std::to_string(posL.y));

    if (!right.show(30))
      break;
    if (!left.show(30))
      break;

    //calculate height:
    double diff = abs(posR.x - posL.x);
    double height = diff * craneHeight / (camDistance + diff);

    std::cout << "Height: " << height << '\n';

  }

  return 0;
}