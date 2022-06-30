#include "cam.h"
#include <iostream>

int main() {
  Cam webcam(0);

  webcam.calculateBackground();
  webcam.showBackground();
  webcam.showTrackbars();
  
  while(webcam.show()); 

  return 0;
}