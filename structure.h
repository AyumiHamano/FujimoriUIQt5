/***************************
* Hearder File for Data Structures
***************************/

#ifndef			_DATA_H_
#define		_DATA_H_

#include<iostream>
using namespace std;

#include<opencv2/opencv.hpp>


//ROI information
typedef struct{
    int			picNo;						//picture number
    string		imagename;			//imagename of ROI
    cv::Rect	roi_rect;					//coordinates of upper left, ROI's width, height
    bool			rotated;					//rotation
}roiRect;

typedef struct{
    int No;
    int targetNo;
    int labelNo;
    int TimeStep;
    double x, y;
}logPoint;

//Line structure
typedef struct{
    int x, y;
}CLine;





#endif
