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

/*
By downloading, copying, installing or using the software you agree to this
license. If you do not agree to this license, do not download, install,
copy or use the software.

License Agreement
For Open Source Computer Vision Library
(3-clause BSD License)

Copyright (C) 2013, OpenCV Foundation, all rights reserved.
Third party copyrights are property of their respective owners.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the names of the copyright holders nor the names of the contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are
disclaimed. In no event shall copyright holders or contributors be liable for
any direct, indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/

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
	// camera parameters are read from somewhere
	//readCameraParameters("out_camera_data.yml",cameraMatrix, distCoeffs);
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

			//vector< Mat > rvecs, tvecs;
			//cv::aruco::estimatePoseSingleMarkers(corners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);
			//// draw axis for each marker

			for (int i = 0; i < ids.size(); i++)
			{
				//	cv::aruco::drawAxis(imageCopy, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);

				int targetCenterX = (corners[i][1].x + corners[i][2].x + corners[i][3].x + corners[i][0].x) / 4;
				int targetCenterY = (corners[i][1].y + corners[i][2].y + corners[i][3].y + corners[i][0].y) / 4;
				int targetWidth = (corners[i][1].x + corners[i][2].x - corners[i][3].x - corners[i][0].x) / 2.0;
				int targetHeight = (-corners[i][1].y + corners[i][2].y + corners[i][3].y - corners[i][0].y) / 2.0;
				int targetAngle = max(round(((double)targetHeight / (double)targetWidth - 1.0) * 136.0), 0.0);

				//角度計算
				//left right check
				float yTmp = corners[i][3].y - corners[i][0].y;
				float xTmp = corners[i][3].x - corners[i][0].x;
				float leftBoard = sqrt(yTmp*yTmp + xTmp*xTmp);
				yTmp = -corners[i][1].y + corners[i][2].y;
				xTmp = -corners[i][1].x + corners[i][2].x;
				float rightBoard = sqrt(yTmp*yTmp + xTmp*xTmp);
				if (leftBoard > rightBoard)
				{
					targetAngle *= -1;
				}
				//角度輸出
				ostringstream oss1;
				oss1 << "angle: " << targetAngle;
				putText(imageCopy, oss1.str(), corners[i][1], FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 200), 1, LINE_8, false);

				//距離計算
				//double baseDist1 = 30;
				//double basePixelHeight1 = 144; //5.5cm mark
				double baseDist1 = 45;
				double basePixelHeight1 = 197; //10.9cm mark
				int targetDist = round(baseDist1 * basePixelHeight1 / targetHeight);
				//double baseDist2 = 120;
				//double basePixelHeight2 = 37;
				//int targetDist = round(baseDist2 * basePixelHeight2 / targetHeight);

				//距離輸出
				ostringstream oss2;
				oss2 << "dist: " << targetDist;
				putText(imageCopy, oss2.str(), corners[i][2], FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 200), 1, LINE_8, false);

				//update output var
				mark->UpdateMark(ids[i], targetAngle, targetDist, targetCenterX, targetCenterY);
			}
		}
		else
		{
			mark->UpdateMark(0,0,0,0,0);
		}
		cv::imshow("out", imageCopy);

		char key = (char)cv::waitKey(33);
		if (key == 27)
			break;
	}
	mark->running = false;
}
