#include "dart.hpp"

void writeToCSV(string filename, Mat m)
{
   ofstream file;
   file.open(filename.c_str());
   file<< cv::format(m, cv::Formatter::FMT_CSV) << std::endl;
   file.close();
}

int overlap(Rect a, Rect b){
  if ((a.x+a.width) <= b.x ||a.x >= ( b.x+b.width )|| a.y >= ( b.y+b.height )|| (a.y + a.height) <= b.y){
    return 0;
  }
  return 1;
}

void createRectFromDiagonal(Point a, Point b){
	printf("Rect(%d,%d,%d,%d);\n", a.x,a.y,b.x-a.x,b.y-a.y);
}

//creating the overlapping rectangle
//NOTE: Rect a must be the true face
double overlapRectanglePerc(Rect a, Rect b){
	//finding the intersecting left, top, right, bottom coordinates
	int left = max(a.x,b.x);
	int top = max(a.y,b.y);
	int right =  min(a.x+a.width,b.x+b.width);
	int bottom = min(a.y+a.height,b.y+b.height);
	int width = right - left;
	int height = bottom - top;

	double overlappingArea = width*height;
	double originalArea = (a.width)*(a.height);
	double perc = overlappingArea/originalArea;
	return perc*100;
}

double f1score(double detected, double trueDetected, double correct){
	double precision = correct/detected;
	printf("precision %f\n",precision);
	double recall = correct / trueDetected;
	printf("recall %f\n",recall);
	return 2*(precision*recall)/(precision + recall);
}


int main( int argc, const char** argv )
{
  // 1. Read Input Image
	Mat frame = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	// 2. Load the Strong Classifier in a structure called `Cascade'
	if( !cascade.load( cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

	// 3. Detect darts and Display Result
	detectAndDisplay( frame );
	// 4. Save Result Image
	imwrite( "detected.jpg", frame );

	return 0;
}

void magnit(Mat &dx, Mat &dy, Mat &out){
  out.create(dx.size(), dx.type());
  printf("Magnituding\n");
  for (int y = 0; y < dx.size[0]; y++){
      for (int x = 0; x < dx.size[1]; x++){
        int pointx = dx.at<uchar>(y,x);
        int pointy = dy.at<uchar>(y,x);
        out.at<uchar>(y,x) = sqrt((pointx*pointx)+(pointy*pointy));
        if (out.at<uchar>(y,x) > 255) out.at<uchar>(y,x) = 255;
      }
  }
}

void direc(Mat &dx, Mat &dy, Mat &out){
  printf("Directioning\n" );
  out.create(dx.size(), dx.type() );
  for (int y = 0; y < dx.size[0]; y++){
      for (int x = 0; x < dx.size[1]; x++){
        int pointx = dx.at<uchar>(y,x);
        int pointy = dy.at<uchar>(y,x);
        out.at<uchar>(y,x) = atan2(pointy,pointx)* 180 /M_PI ;
      }
  }
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
	std::vector<Rect> darts;
	Mat frame_gray, blurred, gradx, grady, mag, dir;

	// 1. Prepare Image by turning it into Grayscale and normalising lighting
	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );
  //applying gaussian blur to get rid of noise
  GaussianBlur( frame_gray, blurred , Size(3,3), 0, 0, BORDER_DEFAULT );
  //finding gradient for x
  Sobel( blurred, gradx, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
  ///finding gradient for y
  Sobel( blurred, grady, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
  //Scales, calculates absolute values, and converts the result to 8-bit.
  convertScaleAbs( gradx, gradx );
  convertScaleAbs( grady, grady );
  magnit(gradx,grady,mag);
  direc(gradx,grady,dir);
  // normalize(dir,dir,0,255,NORM_MINMAX);
  imwrite("magnitude.jpg",mag);
  imwrite("direction.jpg",dir);

  // writeToCSV("gradx.csv",gradx);
  // writeToCSV("direction.csv",dir);

	// // 2. Perform Viola-Jones Object Detection
	// cascade.detectMultiScale( frame_gray, darts, 1.1, 1, 0|CV_HAAR_SCALE_IMAGE, Size(50, 50), Size(500,500) );
  //
	// //counter for correctly recognised darts
	// int counter = 0;
	// //draw rectangle around detected darts
	// for( int i = 0; i < darts.size(); i++ ){
	// 	rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 0, 255, 0 ), 2);
	// }

  //
	// //variable for storing the true face coordinates
	// vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));
	// for (int j = 0; j<truedarts.size(); j++ ){
	// 	rectangle(frame, Point(truedarts[j].x, truedarts[j].y), Point(truedarts[j].x + truedarts[j].width, truedarts[j].y + truedarts[j].height), Scalar( 255, 0, 0 ), 2);
	// 	for( int i = 0; i < darts.size(); i++ ){
	// 		//if they do not overlap then go to next iteration
	// 		if (!overlap(truedarts[j],darts[i])){
	// 			continue; }
	// 		else {
	// 			//getting the % of overlap
	// 			double percentage = overlapRectanglePerc(truedarts[j],darts[i]);
	// 			printf("percentage of overlap  %f%%\n",percentage );
	// 			if (percentage > 70){
	// 				//increment the counter for correctly recognised darts and go to next face
	// 				counter++;
	// 				break; }
	// 		}
	// 	}
	// }
	// printf("Found %d darts out of %d\n",counter,truedarts.size());
  // double f1 = f1score(darts.size(),truedarts.size(),counter);
  // printf("f1score is %lf\n",f1);

}
