# ravenna-port
Bachelor's Thesis : object segmentation with binocular vision for crane at Ravenna' port.

Detects objects and calculate also the heights.

Thesis [here](https://github.com/puccj/ravenna-port/blob/main/Analisi%20di%20video%20con%20visione%20binoculare.pdf "Thesis PDF (IT)") (in Italian).



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
