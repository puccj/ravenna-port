#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>  //for imshow
#include <opencv2/videoio.hpp>  
#include <opencv2/imgproc.hpp>  //for conversion to B&W

#include <iostream>
#include <vector>
#include <time.h>

const int FMS_BKGND = 50; //number of frames used to create background

//returns the background from a set of images/frames
cv::Mat getBackground(cv::Mat* images_ptr, int size = FMS_BKGND) {
  std::cout << "Getting background...";
  int rows = images_ptr[0].rows;
  int cols = images_ptr[0].cols;
  
  cv::Mat background(rows, cols, CV_8UC1);  //store the background image

  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      //create an array with the (x,y) pixel of all frames
      uchar currentPixel[size];

      for (int i = 0; i < size; i++) {
        //insert sort: pos is the position where the element will be inserted
        int pos = i-1;
        while(pos>=0 && images_ptr[i].at<uchar>(y,x) < currentPixel[pos]) {
          currentPixel[pos+1] = currentPixel[pos];
          pos--;
        }
        currentPixel[pos+1] = images_ptr[i].at<uchar>(y,x);
      }
      //now currentPixel is a sorted array with (x,y) pixel by all frames.
      //gets the median value and write it to the (x,y) pixel in background image
      background.at<uchar>(y,x) = currentPixel[size/2];
    }
  }
  std::cout << "     Done\n";
  return background;
}

//get background from camera (consecutive frames)
cv::Mat getBackground(cv::VideoCapture cap = cv::VideoCapture(0)) {
  cv::Mat frames[FMS_BKGND];  //stored frames to calculate the bkgnd with

  for (int i = 0; i < FMS_BKGND; i++) {
    cap.read(frames[i]);
    cv::cvtColor(frames[i], frames[i], cv::COLOR_BGR2GRAY);
  }
  return getBackground(frames, FMS_BKGND);
}

//calculate and returns fps of a webcam
int getFps(cv::VideoCapture cap = cv::VideoCapture(0)) {
  std::cout << "Calculating fps...";
  int num_frames = 60; //number of frames to capture
  time_t start, end;
  cv::Mat frame;
  
  time(&start);
  for(int i = 0; i < num_frames; i++)
    cap >> frame;
  time(&end);

  double seconds = difftime (end, start);
  std::cout << "     Done (" << num_frames/seconds << " fps)\n";
  return num_frames / seconds;
}

int main() {
  cv::VideoCapture cap(0);
  int fps = getFps(cap);

  //get and show the background
  cv::Mat bkFrames[FMS_BKGND];
  for (int i = 0; i < FMS_BKGND; i++) {
    cap.read(bkFrames[i]);
    cv::cvtColor(bkFrames[i], bkFrames[i], cv::COLOR_BGR2GRAY);
  }
  cv::Mat background = getBackground(bkFrames, FMS_BKGND);
  cv::imshow("Background", background);
  

  int frameCount = 0;
  int consecutiveFrames = 2;
  cv::Mat list[consecutiveFrames];

  while(true) {
    //read a frame
    cv::Mat thisFrame;
    cap.read(thisFrame);
    
    //save the original frame
    cv::Mat origFrame;
    thisFrame.copyTo(origFrame);

    cv::cvtColor(thisFrame, thisFrame, cv::COLOR_BGR2GRAY);           //convert to grayscale
    
    //update background every n second
    int n = 60;
    if (frameCount % (n*fps) == 0) {
      bkFrames[frameCount/(n*fps) % FMS_BKGND] = thisFrame;
      std::cout << "Added background frame to position: " << (frameCount/(n*fps)) % FMS_BKGND << '\n';

      background = getBackground(bkFrames, FMS_BKGND);
      cv::imshow("Background", background);
    }

    cv::absdiff(thisFrame, background, thisFrame);                    //subtract background from image
    cv::threshold(thisFrame, thisFrame, 50, 255, cv::THRESH_BINARY);  //thresholding to convert to binary
    cv::dilate(thisFrame,thisFrame, cv::Mat(), cv::Point(-1,-1), 2);  //dilate the image (no inside dark regions)

    list[frameCount % consecutiveFrames] = thisFrame;
    frameCount++;

    //skip if list is not already full
    if (frameCount < consecutiveFrames)
      continue;

    //add all frames in list
    cv::Mat sum = list[0];
    for (int i = 1; i < consecutiveFrames; i++)
      sum += list[i];

    //find the contours and draw them
    std::vector<cv::Mat> contours;
    cv::findContours(sum, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::drawContours(origFrame, contours, -1, {255,0,0}, 1);

    for (auto contour : contours) {
      //skip if the area is too little
      if (cv::contourArea(contour) < 500)
        continue;

      //otherwise, draw a rectangle
      cv::Rect r = cv::boundingRect(contour);
      cv::rectangle(origFrame, r, {0,255,0}, 2);
    }

    //out.write(origFrame);
    cv::imshow("Webcam", origFrame);
    int key = cv::waitKey(5);
    if (key == 'b') {
      background = getBackground(cap);
      cv::imshow("Background", background);
    }
    else if (key == 'q')
      break;
  }

  // the camera will be deinitialized automatically in VideoCapture destructor
  return 0;
}