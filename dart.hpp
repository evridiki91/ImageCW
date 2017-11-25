#include "helper_functions.hpp"

/***********************************************************************
Function that calculates the hough transform for circles and displays
its Hough Space.
Inputs: Source = Magnitude matrix
        Dir    = Direction matrix
        minr,maxr = minimum and maximum radius
Process: Thresholds magnitude image according to the thresh variable defined
in constants.hpp. Iterates through all pixels where an edge has been detected
i.e. has value 255. Sets theta = direction matrix.(+-0.02). Calculates a,b
and increments the accumulator Acc[b,a,r].For visualisation purposes
all the r's are summed up in each dimension to reduce the 3d accumulator
to 2d
***********************************************************************/

void hough_circle(Mat &thr, Mat &dir, int minr, int maxr ){

  printf("inside hough\n");
  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_64F);
  const int rows=thr.size[0], cols=thr.size[1], radii=maxr-minr;
  int dims[3] = {rows, cols, radii};
  cv::Mat acc3d = Mat::zeros(3, dims, CV_64F);
  //iterate through all pixels
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      //if an edge has not been detected go to next pixel
      if ( pixel == 0) {
        continue;
      }
      //if an edge has been detected go on to increment the accumulator
      else{
        //Use gradient direction information as theta (+-0.02)
        double t = dir.at<double>(y,x);
          //iterate through all posible radii
        for (int r = minr; r < maxr; r++){
            int x0 = round(x -r * cos(t));
            int y0 = round(y -r * sin(t));
            if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
              // increment accumulator
              acc3d.at<double>( y0,x0,r-minr) += 1;
            }
            x0 = round(x + r * cos(t));
            y0 = round(y + r * sin(t));
            if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
              // increment accumulator
              acc3d.at<double>( y0,x0,r-minr) += 1;
            }
          }
      }
    }
  }
  //accumulator 3d to 2d for visualisation purposes
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      for (int r = minr; r < maxr; r++){
        acc2d.at<double>(y,x) += acc3d.at<double>(y,x,r);
      }
      acc2d.at<double>(y,x) *= 50;
    }
  }
  writeToCSV("hough_circle.csv",acc2d);
  // convert(acc2d,acc2d,);
  // log_mat(acc2d,acc2d);
  imwrite("hough_circle.jpg",acc2d);
}


/***********************************************************************
Function that calculates the gradient direction of an image using the
gradients in the x and y direction given by sobel. The result is stored in
dir.
***********************************************************************/

void gradient_direction(Mat &dx,Mat &dy, Mat &dir){
  printf("Directioning\n" );
  dir.create(dx.size(), dx.type() );
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      double pointx = dx.at<double>(y,x);
      double pointy = dy.at<double>(y,x);
      dir.at<double>(y,x) = atan2(pointy,pointx) ;   // - (CV_PI/2)
    }
  }
  printf("Finished Directioning\n" );
}

/***********************************************************************
Function that calculates the gradient magnitude of an image using the
gradients in the x and y direction given by sobel. The result is stored in
mag.
***********************************************************************/

void gradient_magnitude(Mat &dx,Mat &dy, Mat &mag){
  printf("Magnituding\n" );
  mag.create(dx.size(), dx.type() );
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      double pointx = dx.at<double>(y,x);
      double pointy = dy.at<double>(y,x);
      mag.at<double>(y,x) = sqrt(pointx*pointx +pointy*pointy) ;
    }
  }
  printf("Finished Magnituding\n" );
}




void hough_line(Mat &thr, Mat &dir, int hough_threshold ){
  printf("inside hough");
  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_64F);
  const int rows=thr.size[0], cols=thr.size[1];
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      if ( pixel == 0) {
        continue;
      }
      else{
        float t = dir.at<float>(y,x);
        for (float i = -0.1; i <= 1.1; i += 0.1){
          int r = round(x*cos(t) + y*sin(t));
          acc2d.at<float>(r,t) +=1;
        }
      }
    }
  }
  writeToCSV("hough_circle.csv",acc2d);
  // convert(acc2d,acc2d);
  // writeToCSV("hough_circlenorm.csv",acc2d);
  // imwrite("hough_circle.jpg",acc2d);
}

/***********************************************************************
Calculates the overlapping percentage of two rectangles.
It finds the rightmost and bottom coordinates of the overlap rectangle
by finding the maximum of the two rectangles respective coordinates.
We do the same for the y direction.
NOTE: Rect a must be the reference. The overlapping percentage will
be compared to this.
***********************************************************************/

float overlapRectanglePerc(Rect a, Rect b){
	//finding the intersecting left, top, right, bottom coordinates
	int left = max(a.x,b.x);
	int top = max(a.y,b.y);
	int right =  min(a.x+a.width,b.x+b.width);
	int bottom = min(a.y+a.height,b.y+b.height);
	int width = right - left;
	int height = bottom - top;

	float overlappingArea = width*height;
	float originalArea = (a.width)*(a.height);
	float perc = overlappingArea/originalArea;
	return perc*100;
}
