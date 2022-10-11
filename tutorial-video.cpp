#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include <random>
#include <iostream>

const int FMS_BKGND = 50;

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

//returns the background from a video
cv::Mat getBackground(std::string path) {
  //TO DO: check if the file has a video extension
  cv::VideoCapture cap(path);

  //take 50 random frames
  cv::Mat frames[FMS_BKGND];
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, cap.get(cv::CAP_PROP_FRAME_COUNT)-1);
  for (int i = 0; i < FMS_BKGND; i++) {
    int rand = dist(rd);
    cap.set(cv::CAP_PROP_POS_FRAMES, rand); //set the frame id to read that particular frame
    cap.read(frames[i]);  //read that frame
    cv::cvtColor(frames[i], frames[i], cv::COLOR_BGR2GRAY); //convert it in B&W
  }

  //calculate the background with those frames
  return getBackground(frames);
}

int main() {
  std::string path = "./video/video_1.mp4";
  cv::Mat background = getBackground(path);
  
  cv::VideoCapture cap(path);
  int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  int fps = cap.get(cv::CAP_PROP_FPS);
  std::string outputName = "Saved.mp4";

  cv::VideoWriter out(outputName, cv::VideoWriter::fourcc('m','p','4','v'), fps, cv::Size(width,height));

  int frameCount = 0;
  int consecutiveFrames = 2;
  cv::Mat list[consecutiveFrames];
  while(cap.isOpened()) {
    //read a frame
    cv::Mat thisFrame;
    cap.read(thisFrame);

    if (thisFrame.empty())
      break;

    //save the original frame
    cv::Mat origFrame;
    thisFrame.copyTo(origFrame);

    //convert to grayscale
    cv::cvtColor(thisFrame, thisFrame, cv::COLOR_BGR2GRAY);
    
    //subtract background from image
    cv::absdiff(thisFrame, background, thisFrame);

    //thresholding to convert to binary
    cv::threshold(thisFrame, thisFrame, 50, 255, cv::THRESH_BINARY);

    //dilate the image (no inside dark regions)
    cv::dilate(thisFrame,thisFrame, cv::Mat(), cv::Point(-1,-1), 2);

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
      //if the area is too little continue through the loop
      if (cv::contourArea(contour) < 500)
        continue;

      //otherwise, draw a rectangle
      cv::Rect r = cv::boundingRect(contour);
      cv::rectangle(origFrame, r, {0,255,0}, 2);
    }

    //cv::imshow("Detected objects", origFrame)
  
    //out.write(origFrame);
    cv::imshow("Ciao", origFrame);
    cv::waitKey(1000/fps);
  }
  
  /* Superfluo
  cap.release();
  cv::destroyAllWindows();
  */
  
  return 0;
}