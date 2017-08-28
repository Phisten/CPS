/* *************************************************************
CPS Mark Detector Demo (x86,vs2013,opencv3.2.0 + opencv2.4.10)
Last Update: Phisten  - 2017/08/28

-----v1.0.0
multiProc
multiOpCV
CPS_Mark DLL API demo

************************************************************** */

#include "stdafx.h"

#include <iostream>
#include <thread>
#include "CPS_Mark.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {

	//----------------------

	// Init Mark Detect Thread (openCV 3.2.0)
	const int CameraIndex = 0;
	CPS_Mark markDetector0;
	thread CPS_0(&CPS_Mark::DetectCardProcess, &markDetector0, CameraIndex);
	cout << "Thread CPS_0 Start : " << CPS_0.get_id() << endl;
	cout << "How to stop thread : focus webcam window and press ESC" << endl;
	
	//----------------------

	// openCV 2.4.10 Test
	Mat img(400, 400, CV_8UC3, Scalar(255, 255, 255));
	ostringstream oss1;
	oss1 << "curProject OpenCv version: " << CV_VERSION_EPOCH << "." << CV_VERSION_MAJOR << "." << CV_VERSION_MINOR;
	putText(img, oss1.str() , cv::Point(100, 100), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 200), 1, 8, false);

	Mat image;
	image = imread("1.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file

	namedWindow("Display window1", WINDOW_AUTOSIZE);// Create a window for display.
	namedWindow("Display window2", WINDOW_AUTOSIZE);// Create a window for display.

	// output Mark Info
	while (markDetector0.IsRunning())
	{
		imshow("Display window1", img);                   // Show our image inside it.
		imshow("Display window2", image);                   // Show our image inside it.

		CPS_Mark::MarkInfo mark0(markDetector0.GetMarkInfo());

		if (mark0.Id > 0)
		{
			printf("ID=%d, Dist=%d Angle=%d  Center:(%d,%d)\n", mark0.Id, mark0.Distance, mark0.Angle, mark0.CenterX, mark0.CenterY);
		}

		char key = (char)cv::waitKey(100);
		if (key == 27)
			break;
	}
	//----------------------
	destroyAllWindows();
	cv::waitKey(100);
	CPS_0.join();
	return 0;
}