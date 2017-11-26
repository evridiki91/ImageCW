#include "dart.hpp"
/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
	std::vector<Rect> darts;
	Mat frame_gray, blurred;
  Mat gradx = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
  Mat grady = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
  Mat mag = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
  Mat thr = Mat::zeros(frame.rows, frame.cols,CV_8UC1);
  Mat dir = Mat::zeros(frame.rows, frame.cols,CV_64FC1);

	// 1. Prepare Image by turning it into Grayscale and normalising lighting
	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );
  //applying gaussian blur to get rid of noise
  GaussianBlur( frame_gray, blurred , Size(3,3), 0, 0, BORDER_DEFAULT );
  imwrite("blurred.jpg",blurred);
  // calculating gradient for x and y

  Sobel( blurred, gradx, CV_64FC1, 1, 0, 3, scaleFactorSobel, 0, BORDER_DEFAULT);
  Sobel( blurred, grady, CV_64FC1, 0, 1, 3, scaleFactorSobel, 0, BORDER_DEFAULT);

  writeToCSV("gradx.csv",gradx);
  writeToCSV("grady.csv",grady);
  imwrite("gradx.jpg",gradx);
  imwrite("grady.jpg",grady);

  // cartToPolar(gradx,grady,mag_non_normalised,dir);
  gradient_magnitude(gradx,grady,mag);
  imwrite("magnitude.jpg",mag);
  threshold(mag,thr, thresh, maxValue, THRESH_BINARY);
  imwrite("threshold.jpg",thr);
  gradient_direction(gradx,grady,dir);

  writeToCSV("dir.csv",dir);

  /*Parameters for Hough transform*/
  // int maxr = round(hypot(mag.size[0], mag.size[1]))/2; //hypotenous = diagonal of image
  int maxr = 170; //works better than above apparently 200
  int minr = 40;  //found through eyeballing.

  vector<Vec3f> circles,circles1;
  hough_circle(thr,dir, minr, maxr );
  printf("hough transform done\n");
  writeToCSV("dir.csv",dir);

  // Apply the Hough Transform to find the circles
  HoughCircles( blurred, circles, CV_HOUGH_GRADIENT, 1, blurred.rows/8, cannyThresh, Houghthresh, minr,maxr);
  for( int i = 0; i < circles.size(); i++ )
  {
     Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
     int radius = cvRound(circles[i][2]);
     // circle center
     circle( frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
     // circle outline
     circle( frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
   }
   printf("found %d\n",circles.size());

   std::vector<Rect> circle_rect;
   Point diagonal_a,diagonal_b,diagonal_a1,diagonal_b1;
   Rect concentric;
   for( size_t i = 0; i < circles.size(); i++ ){

     Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
     int radius = cvRound(circles[i][2]);
     diagonal_a = Point(center.x - radius, center.y - radius);
     diagonal_b = Point(center.x + radius, center.y + radius);
     //calcualating inside square using trigonometry and pythagorean theorem
     int width_2 = round( sqrt(2)*radius/2);
     diagonal_a1 = Point(center.x - width_2, center.y - width_2);
     diagonal_b1 = Point(center.x + width_2, center.y + width_2);

     concentric = Rect(diagonal_a1,diagonal_b1);

     HoughCircles( blurred(concentric), circles1, CV_HOUGH_GRADIENT, 1, concentric.width/8, cannyThresh/2, Houghthresh/2, radius/8,radius*3/4);
     for( int j = 0; j < circles1.size(); j++ )
     {
        Point center(cvRound(circles1[i][0]+concentric.x), cvRound(circles1[i][1])+concentric.y);
        int radius = cvRound(circles1[i][2]);
        if (radius <= 0) continue;
        circle( frame, center, radius, Scalar(255,0,0), 2, 8, 0 );
      }
      printf("%d\n",circles1.size());


     Rect rectangle_c = Rect(diagonal_a,diagonal_b);
     circle_rect.push_back(rectangle_c);
   }

  // 2. Perform Viola-Jones Object Detection
	// cascade.detectMultiScale( frame_gray, darts, 1.1, 1, 0|CV_HAAR_SCALE_IMAGE, Size(50, 50), Size(500,500) );
  // vector<Vec4i> lines;
  // threshold(mag,thr, thresh, maxValue, THRESH_BINARY);
  //
  // HoughLinesP(thr, lines, 1, CV_PI/180, 70, minr, 0 );
  //
  // for( size_t i = 0; i < lines.size(); i++ )
  // {
  //     line( frame, Point(lines[i][0], lines[i][1]),
  //         Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 2, 8 );
  // }
  //
  // vector<Rect> final_rect;
  // Mat score_mat = Mat::zeros(circles.size(),darts.size(),CV_32F);
  // if ((darts.size() == 0) && (circles.size() == 0)) {
  //   final_rect.clear();
  // }
  // else if (circles.size() == 0) {
  //   final_rect = darts;
  // }
  // else if (darts.size() == 0) {
  //   final_rect = circle_rect;
  // }
  // else{
  //   //max distance?
  //   float max_distance = (hypot(frame.size[0], frame.size[1]));
  //   for (int j = 0; j < darts.size(); j++){
  //     Point center_dart(darts[j].x+darts[j].width/2, darts[j].y+darts[j].height/2);
  //     for( int i = 0; i < circle_rect.size(); i++ ){
  //       Point center_circle(circle_rect[i].x+circle_rect[i].width/2, circle_rect[i].y+circle_rect[i].height/2);
  //       float overlap = overlapRectanglePerc(darts[j],circle_rect[i]);
  //       printf("overlap of VJ %d and HT %d is %f\n",j,i,overlap);
  //       float distance = euclidean(center_dart,center_circle);
  //       printf("distance of VJ %d and HT %d is %f\n\n",j,i,distance);
  //
  //       if (distance < 40 && overlap > 0 ){
  //
  //         // check normalised
  //         score_mat.at<float>(i,j) = (100-overlap) + (distance/max_distance)*100;
  //         printf("score is %f\n",score_mat.at<float>(i,j));
  //         if (score_mat.at<float>(i,j) < 80) {
  //
  //           Rect correct = Rect(
  //             Point((darts[j].x+circle_rect[i].x)/2,(darts[j].y+circle_rect[i].y)/2),
  //             Point(((darts[j].x + darts[j].width)+ (circle_rect[i].x + circle_rect[i].width ))/2,
  //             ((darts[j].y + darts[j].height)+ (circle_rect[i].y + circle_rect[i].height ))/2));
  //           final_rect.push_back(correct);
  //         }
  //       }
  //     }
  //   }
  // }
  //
  // printf("final rect size  %d\n",final_rect.size() );
  // printf("circle rect size  %d\n",circle_rect.size() );
  // printf("VJ rect size  %d\n",darts.size() );
  //
	// //counter for correctly recognised darts
	// int counter = 0;
  //
  // for( size_t i = 0; i < circle_rect.size(); i++ ){
	// 	rectangle(frame, Point(circle_rect[i].x, circle_rect[i].y), Point(circle_rect[i].x + circle_rect[i].width, circle_rect[i].y + circle_rect[i].height), Scalar( 255, 255, 0 ), 2);
	// }
  //
  // for( size_t i = 0; i < darts.size(); i++ ){
  //   rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 0, 255, 0 ), 2);
  // }
  //
	// //draw rectangle around detected darts
	// for( size_t i = 0; i < final_rect.size(); i++ ){
	// 	rectangle(frame, Point(final_rect[i].x, final_rect[i].y), Point(final_rect[i].x + final_rect[i].width, final_rect[i].y + final_rect[i].height), Scalar( 0, 0, 255 ), 2);
	// }
  //
	// //variable for storing the true face coordinates
	// vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));
	// for (size_t j = 0; j<truedarts.size(); j++ ){
	// 	rectangle(frame, Point(truedarts[j].x, truedarts[j].y), Point(truedarts[j].x + truedarts[j].width, truedarts[j].y + truedarts[j].height), Scalar( 255, 0, 0 ), 2);
	// 	for( size_t i = 0; i < final_rect.size(); i++ ){
	// 		//if they do not overlap then go to next iteration
	// 		if (!overlap(truedarts[j],final_rect[i])){
	// 			continue; }
	// 		else {
	// 			//getting the % of overlap
	// 			float percentage = overlapRectanglePerc(truedarts[j],final_rect[i]);
	// 			printf("percentage of overlap  %f%%\n",percentage );
	// 			if (percentage > 65){
  //         if (final_rect[i].area() <= 1.5*truedarts[j].area() ){
  //   				//increment the counter for correctly recognised darts and go to next face
  //   				counter++;
  //   				break; }
  //         }
  //       }
  //     }
  //   }
  //   printf("Found %d darts out of %d\n",counter,truedarts.size());
  //   float f1 = f1score(final_rect.size(),truedarts.size(),counter);
  //   printf("f1score is %lf\n",f1);
  //   printf("size of final detections is  %d\n",final_rect.size());

}


int main( int argc, const char** argv ) {
	Mat frame = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	if( !cascade.load( cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

	detectAndDisplay( frame );
	imwrite( "detected.jpg", frame );

	return 0;
}
