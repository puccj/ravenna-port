#include "cam.h"
//#include "video.h"

#include <iostream>

int main() {
  
  Cam right("sample.mp4");
  Cam left("sample2.mp4");

  right.setOrigin();
  left.setOrigin();

  right.calculateBackground();
  left.calculateBackground(true);
  //right.showBackground();

  //Cam::showTrackbars();
  
  while(left.show(60) && right.show(60));
  //while(left.show());
  //while(right.show());
  
  /*
  Cam webcam(0);
  webcam.setOrigin();
  webcam.calculateBackground();
  webcam.showBackground();
  while(webcam.show());
  */

  return 0;
}