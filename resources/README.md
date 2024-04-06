# Add new banana contour reference

To add a new banana contour reference ([reference-contours.yml](../resources/reference-contours.yml)),
the following code can be added in line 36 ([lib.cpp](../src/lib.cpp)):
````cpp
cv::FileStorage fs("resources/reference-contours.yml", cv::FileStorage::WRITE);
fs << "banana" << contours;
fs.release();
````
