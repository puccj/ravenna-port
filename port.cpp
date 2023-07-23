//#include "cam.h"
#include <fstream>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

// read the configuration file to get the username and password
std::string getRtsp() {
  std::string username, password, ip;
  std::ifstream configFile("config.txt");
  if (!configFile.is_open())
    return "error";
  
  std::string line;
  while (std::getline(configFile, line)) {
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      if (key == "username")
        username = value;
      else if (key == "password")
        password = value;
      else if (key == "ip")
        ip = value;
    }
  }
  configFile.close();

  std::cout << "Username = " << username << '\n';
  std::cout << "Password = " << password << '\n';
  std::cout << "IP = " << ip << '\n';
  
  // create a VideoCapture object to open the RTSP stream
  return "rtsp://" + username + ":" + password + "@" + ip + "/axis-media/media.amp?videocodec=h264&resolution=640x480";
}

int main() {
  std::string path = getRtsp();
  if (path == "error") {
    std::cout << "Error in file opening\n";
    return 1;
  }
  
  cv::VideoCapture cap(getRtsp());
  cv::Mat frame;
  while (true) {
    for (int i = 0; i < 100; ++i) { //try 100 times before giving up
      cap.read(frame);
      if (!frame.empty())
        break;
    }

    if (frame.empty()) {
      std::cout << "Can't load frame\n";
      break;
    }

    cv::imshow("Cam", frame);
    int k = cv::waitKey(5);
    if (k == 'q')
      break;
  }

  return 0; 
}