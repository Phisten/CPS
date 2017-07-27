/* *************************************************************
CPS Mark Detector Demo (x86,vs2013,opencv2.4.10)
Last Update: Phisten  - 2017/07/10

-----v1.0.0 alpha



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

using namespace std;
using namespace cv;

void createCard();
void DetectCard();

/**
*/
int main(int argc, char *argv[]) {

	//cv::aruco::CharucoBoard board = ... // create charuco board
	//	cv::Size imgSize = ... // camera image size
	//	std::vector< std::vector<cv::Point2f> > allCharucoCorners;
	//std::vector< std::vector<int> > allCharucoIds;
	//// Detect charuco board from several viewpoints and fill allCharucoCorners and allCharucoIds

	//// After capturing in several viewpoints, start calibration
	//cv::Mat cameraMatrix, distCoeffs;
	//std::vector< Mat > rvecs, tvecs;
	//int calibrationFlags = ... // Set calibration flags (same than in calibrateCamera() function)
	//double repError = cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, board, imgSize, cameraMatrix, distCoeffs, rvecs, tvecs, calibrationFlags);

	DetectCard();




	return 0;
}

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	fs["camera_matrix"] >> camMatrix;
	fs["distortion_coefficients"] >> distCoeffs;
	return true;
}

void DetectCard()
{
	cv::VideoCapture inputVideo;
	inputVideo.open(0);

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

				//int targetCenterX = (corners[i][1].x + corners[i][2].x + corners[i][3].x + corners[i][0].x) / 4;
				//int targetCenterY = (corners[i][1].y + corners[i][2].y + corners[i][3].y + corners[i][0].y) / 4;
				int targetWidth = (corners[i][1].x + corners[i][2].x - corners[i][3].x - corners[i][0].x) / 2.0;
				int targetHeight = (-corners[i][1].y + corners[i][2].y + corners[i][3].y - corners[i][0].y) / 2.0;
				int targetAngle = max(round(((double)targetHeight / (double)targetWidth - 1.0) * 136.0), 0.0);

				//¨¤«×­pºâ
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
				//¨¤«×§¡­È
				//angleRowData.RemoveAt(0);
				//angleRowData.Add(targetAngle);
				//targetAngle = Math.Round(angleRowData.Average());
				ostringstream oss1;
				oss1 << "angle: " << targetAngle;
				putText(imageCopy, oss1.str(), corners[i][1], FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 200), 1, LINE_8, false);

				//¶ZÂ÷­pºâ
				//double baseDist1 = 30;
				//double basePixelHeight1 = 144; //5.5cm mark
				double baseDist1 = 45;
				double basePixelHeight1 = 197; //10.9cm mark
				int targetDist = round(baseDist1 * basePixelHeight1 / targetHeight);
				//double baseDist2 = 120;
				//double basePixelHeight2 = 37;
				//int targetDist = round(baseDist2 * basePixelHeight2 / targetHeight);

				ostringstream oss2;
				oss2 << "dist: " << targetDist;
				putText(imageCopy, oss2.str(), corners[i][2], FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 200), 1, LINE_8, false);

			}
		}
		cv::imshow("out", imageCopy);

		char key = (char)cv::waitKey(33);
		if (key == 27)
			break;
	}
}

void createCard()
{
	cv::Mat markerImage;
	Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

	for (int i = 1; i < 60; i++)
	{
		cv::aruco::drawMarker(dictionary, i, 200, markerImage, 1);
		ostringstream fileName;
		string imagepath = "card\\";
		fileName << imagepath << i << ".jpg";
		imwrite(fileName.str(), markerImage);
		cout << fileName.str() << endl;
	}

	cv::imshow("out", markerImage);
	char key = (char)cv::waitKey(100000);
}
