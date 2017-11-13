/////////////////////////////////////////////////////////////////////////////
//
// COMS30121 - face.cpp
//
/////////////////////////////////////////////////////////////////////////////

// header inclusion
// header inclusion
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

void detectAndDisplay( Mat frame );

String cascade_name = "dartcascade/cascade.xml";
CascadeClassifier cascade;

Rect box[] = {Rect(456,28,128,150)}; //dart 0
// Rect box[] = {Rect(201,144,180,170)}; //dart 1
// Rect box[] = {Rect(111,108,70,70)};	//dart 2
// Rect box[] = {Rect(325,154,63,58)};	//dart 3
// Rect box[] = {Rect(195,104,137,167)}; //dart 4
// Rect box[] = {Rect(442,149,70,83)};//dart 5
// rect box[] = {Rect(214,121,56,57)};//dart 6
// rect box[] = {Rect(262,175,84,128)};//dart 7
// rect box[] = {Rect(74,261,48,72)};//dart 8
// rect box[] = {Rect(852,226,95,102)};//dart 8
// rect box[] = {Rect(226,65,192,194)};//dart 9
// rect box[] = {Rect(104,112,72,88),Rect(590,136,46,63),Rect(924,160,23,47)};	//dart 10
// rect box[] = {Rect(185,114,39,37)};//dart 11
// rect box[] = {Rect(166,86,42,113)};//dart 12
// rect box[] = {Rect(285,128,106,107)};//dart 13
// rect box[] = {Rect(134,114,98,99),Rect(1002,109,98,95)};//dart 14
// rect box[] = {Rect(166,70,107,106)};//dart 15

int overlap(Rect a, Rect b){
  if ((a.x+a.width) <= b.x ||a.x >= ( b.x+b.width )|| a.y >= ( b.y+b.height )|| (a.y + a.height) <= b.y){
    return 0;
  }
  return 1;
}

Rect createRectFromDiagonal(Point a, Point b){
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


/** @function main */

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

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
	std::vector<Rect> darts;
	Mat frame_gray;

	// 1. Prepare Image by turning it into Grayscale and normalising lighting
	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );

	// 2. Perform Viola-Jones Object Detection
	cascade.detectMultiScale( frame_gray, darts, 1.1, 1, 0|CV_HAAR_SCALE_IMAGE, Size(50, 50), Size(500,500) );

	//counter for correctly recognised darts
	int counter = 0;
	//draw rectangle around detected darts
	for( int i = 0; i < darts.size(); i++ ){
		rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 0, 255, 0 ), 2);
	}

	//variable for storing the true face coordinates
	vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));
	for (int j = 0; j<truedarts.size(); j++ ){
		rectangle(frame, Point(truedarts[j].x, truedarts[j].y), Point(truedarts[j].x + truedarts[j].width, truedarts[j].y + truedarts[j].height), Scalar( 255, 0, 0 ), 2);
		for( int i = 0; i < darts.size(); i++ ){
			//if they do not overlap then go to next iteration
			if (!overlap(truedarts[j],darts[i])){
				continue; }
			else {
				//getting the % of overlap
				double percentage = overlapRectanglePerc(truedarts[j],darts[i]);
				printf("percentage of overlap  %f%%\n",percentage );
				if (percentage > 70){
					//increment the counter for correctly recognised darts and go to next face
					counter++;
					break; }
			}
		}
	}
	printf("Found %d darts out of %d\n",counter,truedarts.size());

}
