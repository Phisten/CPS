/* *************************************************************
CPS Mark Detector Demo (x86,vs2013,opencv2.4.10)
Last Update: Phisten  - 2017/07/10

-----v1.0.0 alpha



************************************************************** */

#include "stdafx.h"
#include <highgui.h>
#include <cv.h>

using namespace std;
using namespace cv;

#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480


int main()
{
	//camera init
	VideoCapture cap(2);
	Mat frame;
	if (!cap.isOpened()){
		return -1;
	}

	//init GUI
	namedWindow("srcImg");
	namedWindow("roiSrcImg");
	int firstWindowX = 200;
	cvMoveWindow("srcImg", firstWindowX, 0);
	cvMoveWindow("roiSrcImg", firstWindowX + CAMERA_WIDTH, 0);
	cvMoveWindow("blobCoiThresImg", firstWindowX + CAMERA_WIDTH * 2, 0);

	//config
	int markColorLeft = 0;
	int markColorRight = 30;

	while (true){
		//read Img
		if (!cap.read(frame))
			break;
		Mat src = Mat(frame);
		//roi setting
		int roiX = 0, roiY = 120;
		Rect srcRoi = Rect(roiX, roiY, CAMERA_WIDTH - 1 - roiX, CAMERA_HEIGHT - 1 - roiY);

		//pre-process srcImg
		Mat preProcImg = src(srcRoi);


		//segmentation
		//Mat segImg;
		Mat hsvImg;
		//  bgr2hsv  select channel
		cvtColor(preProcImg, hsvImg, CV_BGR2HSV);
		vector<Mat> hsvChannel;
		split(hsvImg, hsvChannel);

		Mat HueCoi;
		inRange(hsvChannel[0], Scalar(6.0), Scalar(255.0), HueCoi);
		morphologyEx(HueCoi, HueCoi, MORPH_CLOSE, Mat()); //fill bloackRect white space
		morphologyEx(HueCoi, HueCoi, MORPH_OPEN, Mat()); //del background black dot
		
		//mark detector init (config: range)
		//  TODO: config blob param
		//SimpleBlobDetector::Params params;
		SimpleBlobDetector::Params params;
		params.filterByArea = true;
		params.minArea = 60*60;
		params.maxArea = 2147483647;
		// Filter by Circularity
		params.filterByCircularity = true;
		params.minCircularity = 00.1;
		// Filter by Convexity
		params.filterByConvexity = true;
		params.minConvexity = 0.01;
		// Filter by Inertia
		params.filterByInertia = true;
		params.minInertiaRatio = 0.01;
		//  blob search colorRectMark
		SimpleBlobDetector blobDetector(params);
		vector<KeyPoint> kps;
		
		blobDetector.detect(HueCoi, kps);
		
		//mark detect
		

		//output mark info (number,dist,angle)


		//debug-------


		//show Image
		//  srcImg
		{
			//draw roi
			rectangle(src, srcRoi, Scalar(225, 0, 0));
		}
		imshow("srcImg", src);

		Mat dst = hsvChannel[0];
		imshow("roiSrcImg", dst);


		if (!kps.empty()){
			cout << kps.size() << endl;

			//draw blobTarget
			//ostringstream oss;
			//oss << kps.size();
			//putText(src, oss.str(), Point(0, 480), 0, 24, Scalar(255, 255, 0), 2);


			drawKeypoints(HueCoi, kps, HueCoi, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
			for each (KeyPoint var in kps)
			{
				rectangle(HueCoi, Rect(var.pt, Size(var.size, var.size)), Scalar(255, 0, 0), 2);
			}
		}
		imshow("blobCoiThresImg", HueCoi);
		cv::waitKey(30);
	}
}

