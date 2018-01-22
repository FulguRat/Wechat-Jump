#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define RATIO_OF_CHESS 0.0765625f

void screenShot(void);
void pressScreen(int ms);
void delay(int ms);

int main(int argc, char* argv[])
{
	cout << "Be sure that you have open the Jumpin-Jump, press any key to continue." << endl;
	getchar();

	while (true)
	{
		screenShot();

		int xChess = 0;
		int xBlock = 0;
		int xDistance = 0;

		Mat srcImage = imread("autojump.png");
		resize(srcImage, srcImage, Size(srcImage.cols / 2, srcImage.rows / 2));
		//imshow("src", srcImage);

		//find chess by template matching
		Mat templImage = imread("template.jpg");
		resize(templImage, templImage, Size((int)(srcImage.cols * RATIO_OF_CHESS), (int)(srcImage.cols * RATIO_OF_CHESS * 2.57f)));
		Mat resultImage = Mat::zeros(srcImage.rows - templImage.rows + 1, srcImage.rows - templImage.rows + 1, CV_32FC1);

		matchTemplate(srcImage, templImage, resultImage, CV_TM_SQDIFF);
		normalize(resultImage, resultImage, 0, 1, NORM_MINMAX, -1, Mat());

		double minValue;
		double maxValue;
		Point minLocation;
		Point maxLocation;
		Mat chessPosiImage = srcImage.clone();
		minMaxLoc(resultImage, &minValue, &maxValue, &minLocation, &maxLocation, Mat());
		rectangle(chessPosiImage, minLocation, Point(minLocation.x + templImage.cols - 1, minLocation.y + templImage.rows - 1), Scalar(0, 255, 0));
		xChess = ((minLocation.x + templImage.cols - 1) + minLocation.x) / 2;
		//imshow("chess", chessPosiImage);

		//find block
		Mat cannyImage;
		Mat grayImage;
		cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);
		Canny(grayImage, cannyImage, 5, 15);
		//imshow("canny", cannyImage);

		uchar pix = cannyImage.ptr<uchar>(cannyImage.rows / 15)[cannyImage.cols];
		cannyImage.forEach<uchar>([pix, cannyImage, xChess](uchar& pixel, const int* position) -> void
		{
			if (xChess < cannyImage.cols / 2)
			{
				if (position[0] < cannyImage.rows / 5 || position[0] > cannyImage.rows / 2 || position[1] < cannyImage.cols / 2)
				{
					pixel = 0;
				}
			}
			else
			{
				if (position[0] < cannyImage.rows / 5 || position[0] > cannyImage.rows / 2 || position[1] > cannyImage.cols / 2)
				{
					pixel = 0;
				}
			}
		});
		imshow("after", cannyImage);
		waitKey(1);

		int pointCounter = 0;
		for (int i = 0; i < cannyImage.rows; i++)
		{
			for (int j = 0; j < cannyImage.cols; j++)
			{
				uchar pix = cannyImage.ptr<uchar>(i)[j];
				uchar pixNext = cannyImage.ptr<uchar>(i)[j + 1];
				if (pix != 0)
				{
					xBlock += j;
					pointCounter++;

					if (j + 1 >= cannyImage.cols || pixNext == 0)
					{
						xBlock /= pointCounter;
						break;
					}
				}
			}
			if (xBlock != 0) { break; }
		}

		line(srcImage, Point(xChess, srcImage.rows / 2), Point(xBlock, srcImage.rows / 2), Scalar(0, 0, 255), 2);
		imshow("distance", srcImage);
		waitKey(1);

		xDistance = abs(xBlock - xChess);
		cout << "distance = " << xDistance << endl;
		int time = static_cast<int>(3.2f * (float)xDistance);
		cout << "time = " << time << endl;
		pressScreen(time);
		int randomDelay = rand() % 200;
		cout << "random delay = " << randomDelay << endl;
		delay(1000 + randomDelay);
	}

	waitKey(0);
	return 0;
}

string cmd;
void screenShot(void)
{
	cmd = "adb shell screencap -p /sdcard/autojump.png";
	system(cmd.data());
	cmd = "adb pull /sdcard/autojump.png .";
	system(cmd.data());
}

void pressScreen(int ms)
{
	cmd = "adb shell input swipe 300 300 300 300 ";
	char time[10];
	sprintf(time, "%d", ms);
	string timeStr(time);
	cmd += timeStr;
	system(cmd.data());
}

void delay(int ms)
{
	clock_t now = clock();
	while (clock() - now < ms);
}