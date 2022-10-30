#include "cam.h"

int main() {
  Cam prova("sample.mp4");
  prova.calculateBackground();
  Cam::showTrackbars();
  
  //prova.setOrigin();
  //prova.setScale();
  
  while (true) {
    
    if (!prova.process(false, true, true))
      break;

    Fit3d result = prova.fit(true);
    
    if (!prova.show(30))
      break;


    //TO DO : calculate height using 2 Fit3d results
  }

  return 0;
}