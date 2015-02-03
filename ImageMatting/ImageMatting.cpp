#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat source;
Mat grayscale;
Mat affinity;

void main() 
{
	// Load image
	source = imread("square.png", CV_LOAD_IMAGE_GRAYSCALE);
	int i = source.type();

	int numPix = source.rows * source.cols;

	// Create affinity matrix
	affinity = Mat::zeros(numPix, numPix, CV_8UC1);

	int diff = 0;
	for (int i=0; i<numPix; i++)
	{
		for (int j=0; j<numPix; j++)
		{
			diff = j - i;
			affinity.at<uchar>(i,j) = diff;
			affinity.at<uchar>(j,i) = diff;
		}
	}

	// Display image
	namedWindow("Image Matting");
	imshow("Image Matting", source);

	namedWindow("Affinity Matrix");
	imshow("Affinity Matrix", affinity);

	// Hold window until escape key pressed
	waitKey(0);
}