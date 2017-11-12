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

// Rect box[] = { Rect(341,101,142,170) };											//dart 4

// Rect box[] = { Rect(467,208,83,114), Rect(731,181,94,114)}; //dart 14
//
Rect box[] = {																							//dart 15
	Rect(66,133,62,81),
	Rect(374,110,76,93),
	Rect(540,124,87,93) };
//
// Rect box[] = {Rect(415,117,107,146)};												//dart 13
//
// Rect box[]= {																							//dart 5
// 	Rect(64,136,58,71),
// 	Rect(191,212,59,71),
// 	Rect(56,248,59,74),
// 	Rect(254,169,51,63),
// 	Rect(294,241,55,69),
// 	Rect(382,181,60,64),
// 	Rect(431,233,55,69),
// 	Rect(513,166,59,69),
// 	Rect(561,246,59,69),
// 	Rect(647,180,57,69),
// 	Rect(676,242,61,69)
// 	};

void detectAndDisplay( Mat frame );

String cascade_name = "dartcascade/cascade.xml";
CascadeClassifier cascade;

int overlap(Rect a, Rect b){
  if ((a.x+a.width) <= b.x ||a.x >= ( b.x+b.width )|| a.y >= ( b.y+b.height )|| (a.y + a.height) <= b.y){
    return 0;
  }
  return 1;
}

Rect createRectFromDiagonal(Point a, Point b){
  return Rect(a.x,a.y,b.x-a.x,b.y-a.y);
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
	/*
	//variable for storing the true face coordinates
	vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));

	for (int j = 0; j<truedarts.size(); j++ ){
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
	*/
}
