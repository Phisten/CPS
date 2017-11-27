/* *************************************************************
CPS Mark Detector Lib (x86,vs2013,opencv3.2.0)
Last Update: Phisten  - 2017/08/28

-----v1.2.0
DLL API:
	 class CPS_Mark
	 struct MarkInfo
	 CPS_Mark::MarkInfo CPS_Mark::GetMarkInfo();
DLL API Thread Func:
	 static void CPS_Mark::DetectCardProcess(CPS_Mark*, int); 

-----v1.1.0 alpha
c++ ver

************************************************************** */

#include "stdafx.h"

#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <iostream>
#include <ctime>
#include <math.h>
#include <mutex>
#include "CPS_Mark.h"


using namespace std;
using namespace cv;

#define DEBUG_OUTPUT

CPS_Mark::CPS_Mark()
{
}
CPS_Mark::~CPS_Mark()
{
}

//取得最後獲得的MarkInfo
CPS_Mark::MarkInfo CPS_Mark::GetMarkInfo()
{
	std::lock_guard<std::mutex> lock(mutex1);
	CPS_Mark::MarkInfo optInfo(markInfo);
	return optInfo;
}

bool CPS_Mark::IsRunning()
{
	bool tmp = running;
	return tmp;
}

//private 更新MarkInfo
void CPS_Mark::UpdateMark(int id, int angle, int distance,int centerX,int centerY)
{
	std::lock_guard<std::mutex> lock(mutex1);
	markInfo.Id = id;
	markInfo.Angle = angle;
	markInfo.Distance = distance;
	markInfo.CenterX = centerX;
	markInfo.CenterY = centerY;
}

void CPS_Mark::DetectCardProcess(CPS_Mark *mark,int cameraIndex = 0)
{
	mark->running = true;

	cv::VideoCapture inputVideo;
	inputVideo.open(cameraIndex);

	cv::Mat cameraMatrix, distCoeffs;
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
	while (inputVideo.grab()) {

		cv::Mat image, imageCopy;
		inputVideo.retrieve(image);
		image.copyTo(imageCopy);
		std::vector<int> ids;
		std::vector<std::vector<cv::Point2f> > corners;
		cv::aruco::detectMarkers(image, dictionary, corners, ids);
		// if at least one marker detected
		if (ids.size() > 0)
		{
			cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);

			for (int i = 0; i < ids.size(); i++)
			{
				int targetCenterX = (corners[i][1].x + corners[i][2].x + corners[i][3].x + corners[i][0].x) / 4;
				int targetCenterY = (corners[i][1].y + corners[i][2].y + corners[i][3].y + corners[i][0].y) / 4;
				int targetWidth = (corners[i][1].x + corners[i][2].x - corners[i][3].x - corners[i][0].x) / 2.0;
				int targetHeight = (-corners[i][1].y + corners[i][2].y + corners[i][3].y - corners[i][0].y) / 2.0;

				//angle calculate
				//int targetAngle = max(round(((double)targetHeight / (double)targetWidth - 1.0) * 136.0), 0.0);  //-45~45 Approximate 
				double HWRate = min(1.0,(double)targetWidth / (double)targetHeight);
				int arcCosAngle = (int)(acos(HWRate) * 180.0 / 3.1415926);
				float yTmp = corners[i][3].y - corners[i][0].y;
				float xTmp = corners[i][3].x - corners[i][0].x;
				float leftBoard = sqrt(yTmp*yTmp + xTmp*xTmp);
				yTmp = -corners[i][1].y + corners[i][2].y;
				xTmp = -corners[i][1].x + corners[i][2].x;
				float rightBoard = sqrt(yTmp*yTmp + xTmp*xTmp);
				//left right check
				int targetAngle = arcCosAngle;
				if (leftBoard > rightBoard)
				{
					targetAngle *= -1;
				}
				//distance calculate
				double baseDist1 = 45;
				double basePixelHeight1 = 197; //10.9cm mark
				int targetDist = round(baseDist1 * basePixelHeight1 / targetHeight);

#ifdef DEBUG_OUTPUT
				//angle debug output
				ostringstream oss1;
				oss1 << "angle: " << targetAngle;
				putText(imageCopy, oss1.str(), corners[i][1], FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 200), 1, LINE_8, false);
				//distance debug output
				ostringstream oss2;
				oss2 << "dist: " << targetDist;
				putText(imageCopy, oss2.str(), corners[i][2], FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 200), 1, LINE_8, false);
#endif

				//update output var
				mark->UpdateMark(ids[i], targetAngle, targetDist, targetCenterX, targetCenterY);
			}
		}
		else
		{
			mark->UpdateMark(0,0,0,0,0);
		}

#ifdef DEBUG_OUTPUT
		cv::imshow("out", imageCopy);
#endif

		char key = (char)cv::waitKey(33);
		if (key == 27)
			break;
	}
	mark->running = false;
}
