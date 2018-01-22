#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define RATIO_OF_CHESS 0.0765625f

int main(int argc, char* argv[])
{
	int xChess = 0;
	int xBlock = 0;
	
	Mat srcImage = imread("test1.png");
	resize(srcImage, srcImage, Size(srcImage.cols / 2, srcImage.rows / 2));
	imshow("src", srcImage);

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
	imshow("chess", chessPosiImage);

	//find block
	Mat hsvImage;
	cvtColor(srcImage, hsvImage, COLOR_BGR2HSV_FULL);
	imshow("hsv", hsvImage);

	Vec3b pix = hsvImage.ptr<Vec3b>(hsvImage.rows / 15)[hsvImage.cols];
	hsvImage.forEach<Vec3b>([pix, hsvImage, xChess](Vec3b& pixel, const int* position) -> void
	{
		if (fabs(pixel[0] - pix[0]) < 10 && fabs(pixel[1] - pix[1]) < 40)
		{
			pixel[0] = 0;
			pixel[1] = 0;
			pixel[2] = 0;
		}
		if (xChess < hsvImage.cols / 2)
		{
			if (position[0] < hsvImage.rows / 5 || position[0] > hsvImage.rows / 2 || position[1] < hsvImage.cols / 2)
			{
				pixel[0] = 0;
				pixel[1] = 0;
				pixel[2] = 0;
			}
		}
		else
		{
			if (position[0] < hsvImage.rows / 5 || position[0] > hsvImage.rows / 2 || position[1] > hsvImage.cols / 2)
			{
				pixel[0] = 0;
				pixel[1] = 0;
				pixel[2] = 0;
			}
		}
	});
	imshow("after", hsvImage);

	int pointCounter = 0;
	for (int i = 0; i < hsvImage.rows; i++)
	{
		for (int j = 0; j < hsvImage.cols; j++)
		{
			Vec3b pix = hsvImage.ptr<Vec3b>(i)[j];
			Vec3b pixNext = hsvImage.ptr<Vec3b>(i)[j + 1];
			if (pix[0] != 0 || pix[1] != 0 || pix[2] != 0)
			{
				xBlock += j;
				pointCounter++;

				if (j + 1 >= hsvImage.cols || (pixNext[0] == 0 && pixNext[1] == 0 && pixNext[2] == 0))
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

	waitKey(0);
	return 0;
}