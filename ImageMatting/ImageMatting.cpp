#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

Mat source;
Mat grayscale;
Mat affinity;

void main() 
{
	// Load image
	/*source = imread("square.png", CV_LOAD_IMAGE_GRAYSCALE);
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
	waitKey(0);*/



		// Open another image
		Mat image;
		image = cv::imread("korver.jpg");

		// define bounding rectangle 
		cv::Rect rectangle(50, 70, image.cols - 150, image.rows - 180);

		cv::Mat result; // segmentation result (4 possible values)
		cv::Mat bgModel, fgModel; // the models (internally used)

		// GrabCut segmentation
		cv::grabCut(image,    // input image
			result,   // segmentation result
			rectangle,// rectangle containing foreground 
			bgModel, fgModel, // models
			1,        // number of iterations
			cv::GC_INIT_WITH_RECT); // use rectangle
		cout << "oks pa dito" << endl;
		// Get the pixels marked as likely foreground
		cv::compare(result, cv::GC_PR_FGD, result, cv::CMP_EQ);
		// Generate output image
		cv::Mat foreground(image.size(), CV_8UC3, cv::Scalar(255, 255, 255));
		image.copyTo(foreground, result); // bg pixels not copied

		// draw rectangle on original image
		cv::rectangle(image, rectangle, cv::Scalar(255, 255, 255), 1);
		cv::namedWindow("Image");
		cv::imshow("Image", image);

		// display result
		cv::namedWindow("Segmented Image");
		cv::imshow("Segmented Image", foreground);


		waitKey();
}