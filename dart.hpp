#include "helper_functions.hpp"

/***********************************************************************
Hough Circles Function: Calculates the hough transform for circles and finds
the hough peaks. It also displays the Hough Space.

Parameters: thr = Thresholded image matrix
            Dir = Direction matrix
            minr,maxr = minimum and maximum radius
            potentialCircles = vector where the detected circles will be stored
            no_neighbors = the size of the neighbors that will be taken into consideration               when calculating the hough peaks.
            circleHoughthresh = Threshold used for identifying a peak in Hough space.


Process: Iterates through all pixels where an edge has been detected
i.e. thr has value 255. Sets theta = direction matrix, calculates a, b
and increments the accumulator Acc[b,a,r].Then all the r's are summed
up in each dimension to reduce the 3d accumulator to 2d. Then the 2d hough
space peaks are found by searching for local maxima in neighboring pixels.
Then using the detected x,y peaks, the optimal r is found by searching
the corresponding r accumulator is found the 3D accumulator for the r that
corresponds to the higher acc3d(y,x,r). Finally the x,y,r found are stored in
the potentialCircles.
***********************************************************************/

void hough_circle(Mat thr, Mat dir, int minr, int maxr, vector<Vec3f> &potentialCircles,
  int no_neighbors, int circleHoughthresh, int isConcentric){

  potentialCircles.clear();
  printf("Hough Circle ...\n");
  vector<Vec2f> potentialCircles2d;
  //Initialise accumulator
  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_64FC1);
  const int rows=thr.size[0], cols=thr.size[1], radii=maxr-minr;
  int dims[3] = {rows, cols, radii};
  cv::Mat acc3d = Mat::zeros(3, dims, CV_64FC1);

  //iterate through all pixels
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      // printf("pixel : %d",pixel);
      //if an edge has not been detected go to next pixel
      if ( pixel == 255) {
        //Use gradient direction information as theta (+-0.02)
        double t = dir.at<double>(y,x) ;
        //iterates through all posible radii
        for (int r = 0; r < radii; r++){
            int x0 = (x -(r+minr) * cos(t));
            int y0 = (y -(r+minr) * sin(t));
            // printf("i %d j %d k %d i0%d j0%d",y,x,r,y0,x0);
            //Check if the circle is out of the bounds of the image
            if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
              // increment 3d accumulator
              acc3d.at<double>( y0,x0,r) += 1;
            }
            x0 = (x + (r+minr) * cos(t));
            y0 = (y + (r+minr) * sin(t));
            // printf("i1%d j1%d\n", y0,x0);
            if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
              // increment accumulator
              acc3d.at<double>( y0,x0,r) += 1;
            }
          }
      }
    }
  }
  //accumulator 3d to 2d
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      for (int r = 0; r < maxr - minr ; r++){
        acc2d.at<double>(y,x) += acc3d.at<double>(y,x,r);
      }
    }
  }
  writeToCSV("hough_circle.csv",acc2d);
  if (!isConcentric) imwrite("hough_circle.jpg",acc2d);

  int breakLoop = false;

//Starting from the minr to avoid out of the bounds circles
  for (int y = minr ; y < rows-minr; y++){
    for (int x = minr ; x < cols-minr; x++){
      breakLoop = false;
      int peak = acc2d.at<double>(y,x);
      if (peak > circleHoughthresh) {
        //checking in neighbors (offset)
        for( int m = -no_neighbors; m <= no_neighbors; m++ ){
          if (breakLoop) break;
          for( int n = -no_neighbors; n <= no_neighbors; n++ ){
            int nei_x = n + x ;
            int nei_y = m + y;
            //Continue if it's the middle pixel, neighbors are out of bounds
            if ((m == 0 && n == 0) || nei_x < 0 || nei_x >= cols || nei_y < 0 || nei_y >= rows)
              continue;
            else{

              //If a neighbor has a larger value,it's not a local max, break double loop
              if (acc2d.at<double>(nei_y,nei_x) > peak){
                breakLoop = true;
                break;
              }
            }
          }
        }
        if (breakLoop) continue;
        //If it's a local maxima push into the vector
        else {
          potentialCircles2d.push_back(Vec2f(y,x));
        }
      }
    }
  }
  double max,min;
  minMaxLoc(acc2d, &min,&max);
  convert(acc2d,acc2d,min,max);
  int maxr_value, maxr_index;
  printf("Hough circle found %d potential maxima\n",potentialCircles2d.size());

  //Loop through the 2d peaks and find the optimal radius for each circle
  for (int i = 0; i < potentialCircles2d.size(); i++){
    maxr_value = -1;
    maxr_index = -1;
    for (int r = 0; r < radii; r++){
      double acc3d_value = acc3d.at<double>(potentialCircles2d[i][0], potentialCircles2d[i][1],r);
      if ( acc3d_value > maxr_value ){
        maxr_value = acc3d_value;
        maxr_index = r+minr;
      }
    }
    potentialCircles.push_back(Vec3f(potentialCircles2d[i][1],potentialCircles2d[i][0],maxr_index));
  }
}


/***********************************************************************
Function that calculates the gradient direction of an image using the
gradients in the x and y direction given by sobel. The result is stored in
dir.
***********************************************************************/

void gradient_direction(Mat &dx,Mat &dy, Mat &dir){
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      double pointx = dx.at<double>(y,x);
      double pointy = dy.at<double>(y,x);
      if (pointx == 0) dir.at<double>(y,x) = 0;
      else dir.at<double>(y,x) = atan(pointy/pointx) ;
    }
  }
  writeToCSV("dir.csv",dir);
  printf("Finished Directioning\n" );
}

/***********************************************************************
Function that calculates the gradient magnitude of an image using the
gradients in the x and y direction given by sobel. The result is stored in
mag.
***********************************************************************/

void gradient_magnitude(Mat &dx,Mat &dy, Mat &mag){
  Mat mag_non_normalised = Mat::zeros(dx.rows,dx.cols,CV_64FC1);
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      double pointx = dx.at<double>(y,x);
      double pointy = dy.at<double>(y,x);
      mag_non_normalised.at<double>(y,x) = sqrt(pointx*pointx +pointy*pointy) ;
    }
  }
  writeToCSV("mag_non_normalised.csv",mag_non_normalised);
  normalize(mag_non_normalised,mag,0, 255, NORM_MINMAX, CV_8UC1);
  printf("Finished Magnituding\n" );
}

/***********************************************************************
Hough Line Function: Calculates the hough transform for lines and finds
the hough peaks. It also displays the Hough Space.

Parameters: thr = Thresholded image matrix
            potentialLines = vector that stores the detected lines (rho,theta)
            no_neighbors = the size of the neighbors that will be taken into consideration               when calculating the hough peaks.
            Houghthresh = Threshold used for identifying a peak in Hough space.

Process: Thresholds magnitude image according to the thresh variable Houghthresh.
Iterates through all pixels where an edge has been detected i.e. has value 255.
Votes for every line with theta = -p/2 to p/2. Calculates r and increments the
accumulator Acc[r,t]. The image is resized 512x512. Finds local maxima by
comparing points with neighbors. Stores this local maxima in potentialLines.
***********************************************************************/

void hough_line(Mat &thr, vector<Point2f> &potentialLines, int no_neighbors,
 int Houghthresh){

  printf("Hough Line ...\n");
  const int rows=thr.size[0], cols=thr.size[1];
  int diagonal = floor(sqrt(rows*rows + cols*cols));
  int degree_range = 90;
  float resolution = 1;

  Mat acc2d = Mat::zeros(diagonal*2, degree_range*2/resolution,CV_64FC1);
  Mat acc2d_norm = Mat::zeros(diagonal*2, degree_range*2/resolution,CV_64FC1);
  Mat acc2d_display;

  //iterating through all x and y
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      //if an edge has not been detected continue to next iteration
      if ( pixel == 0) {
        continue;
      }
      //an edge has been detected
      else{
        for (float t = -90; t < 90; t+=resolution){
          int r = round(x*cosd(t) + y*sind(t));
          //scaling theta so that the indexes are not decimal or negative
          int t_index = (t+90)*(1/resolution) ;
          //checking for out of bounds condition and incrementing accumulator
          if(r >= -diagonal && t_index >=0 && t_index < degree_range*2/resolution && r <  diagonal) {
            acc2d.at<double>( r+diagonal,t_index) +=1;
          }
        }
      }
    }
  }
  //scaling values from 0 to 255 for visualisation and resizing image to be square
  double max,min;
  minMaxLoc(acc2d, &min,&max);
  convert(acc2d,acc2d_norm,0,max);
  resize(acc2d_norm,acc2d_display, Size(512, 512)) ;

  writeToCSV("acc2d.csv",acc2d);
  writeToCSV("acc2d_norm.csv",acc2d_norm);

  //variable for breaking out of double loop
  int breakLoop = false;

  /**** finding local maxima ****/

  //iterating through all r and theta
  for (int r = 0; r < 2*diagonal; r++) {
    for (float t = -90 ; t <90; t+=resolution){
      breakLoop = false;
      int t_index = (t+90)*(1/resolution);
      int peak = acc2d.at<double>(r,t_index);
      //if accumulator is greater than Houghthresh then it is a peak
      if (peak > Houghthresh) {
        //checking in neighbors for larger values
        for( int m = -no_neighbors; m <= no_neighbors; m++ ){
          if (breakLoop) break;
          for( int n = -no_neighbors; n <= no_neighbors; n++ ){
            int nei_x = m + r ;
            int nei_y = n + t_index;
            //Costraints for neighbors and continue if m and n = 0
            if ((m == 0 && n == 0) || nei_x < 0 || nei_x >= 2*diagonal || nei_y < 0 || nei_y >= degree_range*2/resolution)
              continue;
            else{
              //local maxima check, if accumulator is greater than the peak
              // then it's not local maxima and break the double loop
              if (acc2d.at<double>(nei_x,nei_y) > peak){
                breakLoop = true;
                break;
              }
            }
          }
        }
        if (breakLoop) continue;
        //it's a local maxima and add it
        else potentialLines.push_back(Point(r,t_index));
      }
    }
  }
  //rescaling back to the original values
  for ( int index = 0; index < potentialLines.size(); index++){
    potentialLines[index].x -= diagonal;
    potentialLines[index].y = (potentialLines[index].y*resolution)-90;
  }
  imwrite("hough_line.jpg",acc2d_display);
  printf(" Potential lines = %d\n",potentialLines.size());
}


/***********************************************************************
Calculates the overlapping percentage of two rectangles.
It finds the rightmost and bottom coordinates of the overlap rectangle
by finding the maximum of the two rectangles respective coordinates.
We do the same for the y direction.
NOTE: Rect a must be the reference. The overlapping percentage will
be compared to this.
***********************************************************************/

int overlapRectanglePerc(Rect a, Rect b){
	//finding the intersecting left, top, right, bottom coordinates
	int left = max(a.x,b.x);
	int top = max(a.y,b.y);
	int right =  min(a.x+a.width,b.x+b.width);
	int bottom = min(a.y+a.height,b.y+b.height);
  //calculates the width and height of the overlaping rectangle
	int width = right - left;
	int height = bottom - top;
  printf("Overlap dimensions l %d r %d t %d b %d w %d h %d \n", left,right,top,bottom,width, height);
  //calculates the overlapping area and the reference(original) area
  float overlappingArea = width*height;
	float originalArea = (a.width)*(a.height);
	float perc = overlappingArea/originalArea;
	return perc*100;
}
