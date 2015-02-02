#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat image;

void main() 
{
	// Load image
	image = imread("cubs.jpg");

	// Display image
	namedWindow("Image Matting");
	imshow("Image Matting", image);

	// Hold window until escape key pressed
	waitKey(0);
}