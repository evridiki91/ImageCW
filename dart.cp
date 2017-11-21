#include "dart.hpp"

void convert(Mat &mag){
  double scale_min,scale_max;
  minMaxLoc(mag,&scale_min,&scale_max);
  mag.convertTo(mag,CV_8UC1,255/scale_max,0);
  printf("Converting done\n");
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
	std::vector<Rect> darts;
	Mat frame_gray, blurred, gradx, grady, mag;
	// 1. Prepare Image by turning it into Grayscale and normalising lighting
	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );
  //applying gaussian blur to get rid of noise
  GaussianBlur( frame_gray, blurred , Size(3,3), 0, 0, BORDER_DEFAULT );
  // finding gradient for x and y
  Sobel( blurred, gradx, ddepth, 1, 0);
  Sobel( blurred, grady, ddepth, 0, 1);
  magnitude(gradx,grady,mag);

  Mat dir = gradient_direction(gradx,grady);
  printf("direction done");
  writeToCSV("dir.csv",dir);
  writeToCSV("mag.csv",mag);
  convert(mag);
  imwrite("magnitude.jpg",mag);

  int maxr = int (hypot(mag.size[0], mag.size[1]))/2; //hypotenous = diagonal of image
  int minr = 0;
  Mat thr;
  threshold(mag,thr, thresh, maxValue, THRESH_BINARY);
  imwrite("threshold.jpg",thr);
  printf("threshold done ...");
  printf("hough transform ...");
  hough_circle(thr, minr, maxr, dir);
  printf("hough transform done");
  writeToCSV("direction.csv",dir);
  vector<Vec3f> circles;

  // Apply the Hough Transform to find the circles
  // HoughCircles( blurred, circles, CV_HOUGH_GRADIENT, 1, blurred.rows/8, 200, 100, 25, 0 );
	// // 2. Perform Viola-Jones Object Detection
	// cascade.detectMultiScale( frame_gray, darts, 1.1, 1, 0|CV_HAAR_SCALE_IMAGE, Size(50, 50), Size(500,500) );
  // //iterate through all circles and all rectangles and find
  // //which ones centers are the closest.
  //
  // // if (darts.size() == 0) {
  // // //ignore
  // // }
  // // if (circles.size() == 0) {
  // // // take VJ detections
  // // }
  // // else{
  // //   double max_distance = (hypot(frame.size[0], frame.size[1]));
  // //   Mat score_mat = Mat::zeros(circles.size(),darts.size(),CV_64F);
  // //   for( int i = 0; i < circles.size(); i++ ){
  // //     Point hough_center(cvRound(circles[i][0]), cvRound(circles[i][1]));
  // //     int radius = cvRound(circles[i][2]);
  // //     Rect rect_circle = createRectFromDiagonal(Point(hough_center.x -radius,hough_center.y -radius), Point(hough_center.x + radius,hough_center.y + radius));
  // //     double max;
  // //     for (int j = 0; j < darts.size(); j++){
  // //       Point center_dart(darts[j].x+darts[j].width/2, darts[j].y+darts[j].height/2);
  // //       double overlap = overlapRectanglePerc(darts[j],rect_circle);
  // //       double distance = euclidean(hough_center,center_dart);
  // //       score_mat.at<double>(i,j) = (100-overlap)+ (distance/max_distance)*100;
  // //       }
  // //     }
  // // }
  //
  //
  // for( size_t i = 0; i < circles.size(); i++ )
  // {
  //    Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
  //    int radius = cvRound(circles[i][2]);
  //    // circle center
  //    circle( frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
  //    // circle outline
  //    circle( frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
  //  }
  //
	// //counter for correctly recognised darts
	// int counter = 0;
	// //draw rectangle around detected darts
	// for( int i = 0; i < darts.size(); i++ ){
	// 	rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 0, 255, 0 ), 2);
	// }
  // namedWindow( "Hough Circle Transform Demo", CV_WINDOW_AUTOSIZE );
  // imshow( "Hough Circle Transform Demo", frame );
  // waitKey(0);
  //
	// // //variable for storing the true face coordinates
	// // vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));
	// // for (int j = 0; j<truedarts.size(); j++ ){
	// // 	rectangle(frame, Point(truedarts[j].x, truedarts[j].y), Point(truedarts[j].x + truedarts[j].width, truedarts[j].y + truedarts[j].height), Scalar( 255, 0, 0 ), 2);
	// // 	for( int i = 0; i < darts.size(); i++ ){
	// // 		//if they do not overlap then go to next iteration
	// // 		if (!overlap(truedarts[j],darts[i])){
	// // 			continue; }
	// // 		else {
	// // 			//getting the % of overlap
	// // 			double percentage = overlapRectanglePerc(truedarts[j],darts[i]);
	// // 			printf("percentage of overlap  %f%%\n",percentage );
	// // 			if (percentage > 70){
  // //         if (darts[i].area() <= 1.5*truedarts[j].area() ){
  // //   				//increment the counter for correctly recognised darts and go to next face
  // //   				counter++;
  // //   				break; }
  // //         }
  // //       }
  // //     }
  // //   }
  // //   printf("Found %d darts out of %d\n",counter,truedarts.size());
  // //   double f1 = f1score(darts.size(),truedarts.size(),counter);
  // //   printf("f1score is %lf\n",f1);
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
