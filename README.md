# ravenna-port
Bachelor's Thesis : computer vision for crane at Ravenna' port

## Compile code
After installing OpenCV library, compile with:

Windows:
```
g++ main.cpp cam.cpp -I C:\opencv\include -L C:\opencv\x64\mingw\lib -lopencv_core455 -lopencv_highgui455 -lopencv_imgproc455 -lopencv_videoio455
```
(assuming the library is installed in C:\ path)

Linux:
```
g++ main.cpp cam.cpp -I /usr/local/include/opencv4 -L /usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio
```
(assuming the library is installed in the default path)
