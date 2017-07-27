
#include <iostream>
#include <new>
#include <fstream>
#include<algorithm>
#include<filesystem>

#include <omp.h>
#include <direct.h>

#include <opencv2/opencv.hpp>
#include <openslide.h>
#include <openslide-features.h>

#include <QDebug>
#include <QMessageBox>
#include "Structure.h"


/****************
 * Convert SCN file into cv::Mat
 * **************/
bool ConvertSCN2cvMat(std::string scnpath, cv::Mat& dst, int32_t Level)
{
    //--- Check whether "scnpath" can be opened ------------------//
    qDebug() << QString::fromStdString(scnpath);
    bool open_checked;
    try{
        open_checked = openslide_can_open(scnpath.c_str());
    }
    catch(char *str){
        std::string errorstring = str;
        qDebug() << QString::fromStdString(errorstring);
    }

    if(!open_checked)
        return false;


    //--- Open an openslide object -------------------------------//
    openslide_t *obj = openslide_open(scnpath.c_str());
    if(obj == NULL)
        return false;


    //--- Get dimensions -----------------------------------------//
    int64_t width, height;
    openslide_get_level_dimensions(obj, Level, &width, &height);


    //--- Get pixel data -----------------------------------------//
    //memory
    uint32_t *dest;
    try{
        dest = new uint32_t[width*height];
    }
    catch(std::bad_alloc){
        return false;
    }

    openslide_read_region(obj, dest, 0, 0, Level, width, height);


    //--- Convert to cv::Mat ---------------------------------------//
    if (!dst.empty())	dst.release();

    dst = cv::Mat(cv::Size((int)width, (int)height), CV_8UC3);

#pragma omp parallel for

    for (int id = 0; id < (int)(width*height); id++){
        //get coordinates
        int x = id % (int)width;
        int y = id / (int)width;

        //get a value
        unsigned int pix = dest[id];
        int val = pix;

        //convert
        uchar b = val & 0xff; val = val >> 8;
        uchar g = val & 0xff; val = val >> 8;
        uchar r = val & 0xff;

        //save
        cv::Vec3b *ptr = dst.ptr<cv::Vec3b>(y);
        ptr[x] = cv::Vec3b(b, g, r);

    }//id end

    //--- Release -------------------------------------------//
    delete[] dest;


    //--- Return --------------------------------------------//
    return true;
}


/****************
 * Cut a region and convert into cv::Mat
 * **************/
bool showROI(std::string scnpath, cv::Mat& dst, int32_t LEVEL, int64_t X, int64_t Y, int64_t width, int64_t height)
{
    //--- Check whether "scnpath" can be opened ------------------//
    if(!openslide_can_open(scnpath.c_str()))
        return false;


    //--- Open an openslide object -------------------------------//
    openslide_t *obj = openslide_open(scnpath.c_str());

    //--- Set a factor -------------------------------------------//
    int64_t factor;
    if(LEVEL == 2)  factor = 16;
    else if(LEVEL == 1) factor = 4;
    else if(LEVEL == 0) factor = 1;
    else                factor = (int64_t)floor(openslide_get_level_downsample(obj, LEVEL) + 0.5);


    //--- Get pixel data -----------------------------------------//
    //memory
    uint32_t *dest;
    try{
        dest = new uint32_t[width*height];
    }
    catch(std::bad_alloc){
        return false;
    }

    openslide_read_region(obj, dest, X*factor, Y*factor, LEVEL, width, height);



    //--- Convert to cv::Mat ---------------------------------------//
    if (!dst.empty())	dst.release();

    dst = cv::Mat(cv::Size((int)width, (int)height), CV_8UC3);

#pragma omp parallel for

    for (int id = 0; id < (int)(width*height); id++){
        //get coordinates
        int x = id % (int)width;
        int y = id / (int)width;

        //get a value
        unsigned int pix = dest[id];
        int val = pix;

        //convert
        uchar b = val & 0xff; val = val >> 8;
        uchar g = val & 0xff; val = val >> 8;
        uchar r = val & 0xff;

        //save
        cv::Vec3b *ptr = dst.ptr<cv::Vec3b>(y);
        ptr[x] = cv::Vec3b(b, g, r);

    }//id end

    //--- Release -------------------------------------------//
    delete[] dest;


    //--- Return --------------------------------------------//
    return true;
}

bool showROI(std::string scnpath, cv::Mat& dst, int32_t LEVEL, int64_t X, int64_t Y, int64_t Width, int64_t Height, cv::Mat matrix)
{
    //--- Check whether "scnpath" can be opened ------------------//
    if(!openslide_can_open(scnpath.c_str()))
        return false;


    //--- Open an openslide object -------------------------------//
    openslide_t *obj = openslide_open(scnpath.c_str());

    //--- Set a factor -------------------------------------------//
    int64_t factor;
    if(LEVEL == 2)  factor = 16;
    else if(LEVEL == 1) factor = 4;
    else if(LEVEL == 0) factor = 1;
    else                factor = (int64_t)floor(openslide_get_level_downsample(obj, LEVEL) + 0.5);


    //--- Initialization  ----------------------------------//
    int64_t width = Width;
    int64_t height = Height;
    int64_t workw = width * 2;
    int64_t workh = height * 2;

    uint32_t *dest;

    //get a memory
    try{
        dest = new uint32_t[workw*workh];
    }
    catch(bad_alloc){
        return false;
    }


    //座標計算
    int64_t CX = X + width/2;
    int64_t CY = Y + height/2;
    int64_t WX = (CX - workw / 2)*factor;   if(WX<0)    WX = 0;
    int64_t WY = (CY - workh / 2)*factor;   if(WY<0)    WY = 0;

    //倍率により座標を修正
    int adjx = WX / factor;
    int adjy = WY / factor;

    //画素情報を取得
    openslide_read_region(obj, dest, WX, WY, LEVEL, workw, workh);


    //--- 結果画像の作成 ---//
    cv::Mat resultimg = cv::Mat(cv::Size((int)width, (int)height), CV_8UC3, cv::Scalar::all(0));

#ifdef _OPENMP
#pragma omp parallel for
#endif
       for (int i = 0; i < (int)width*height; i++){

           //座標を取得
           int x = i%(int)width;
           int y = i / (int)width;

           //新しい座標を計算
           cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
           origin.at<double>(0, 0) = (double)x;
           origin.at<double>(1, 0) = (double)y;
           origin.at<double>(2, 0) = 1.0;

           cv::Mat coord = matrix * origin;

           //画素情報を取得するために計算した座標を修正
           bool out_of_range = false;
           int nx = (int)((double)X + coord.at<double>(0, 0) + 0.5) - adjx;
           int ny = (int)((double)Y + coord.at<double>(1, 0) + 0.5) - adjy;
           if (nx < 0 || nx >(int)workw || ny < 0 || ny >(int)workh){
               out_of_range = true;
           }

           //画素値を取得
           unsigned int pix;
           if(!out_of_range)
               pix = dest[workw*ny + nx];
           else
               pix = 0;

           //RGB変換
           int val = pix;
           int r,g,b;
           if(!out_of_range){
             b = val & 0xff;  val = val >> 8;
             g = val & 0xff;  val = val >> 8;
             r = val & 0xff;
           }
           else{
               r = g = b = 0;
           }

           //画素値を保存
           cv::Vec3b *ptr = resultimg.ptr<cv::Vec3b>(y);
           ptr[x] = cv::Vec3b(b, g, r);

       }

       //qApp->processEvents();


       //--- 結果画像の保存 ---//
       dst = resultimg.clone();
       resultimg.release();


    //--- Release -------------------------------------------//
    delete[] dest;


    //--- Return --------------------------------------------//
    return true;
}

bool showROI(std::string scnpath, cv::Mat& dst, int32_t LEVEL, roiRect sliceROI, roiRect uppersliceROI, cv::Mat matrix)
{
    //--- Check whether "scnpath" can be opened ------------------//
    if(!openslide_can_open(scnpath.c_str()))
        return false;



    //--- Open an openslide object -------------------------------//
    openslide_t *obj = openslide_open(scnpath.c_str());


    //--- Get variables ------------------------------------------//
    int64_t X = uppersliceROI.roi_rect.x;
    int64_t Y = uppersliceROI.roi_rect.y;
    int64_t width = uppersliceROI.roi_rect.width;
    int64_t height = uppersliceROI.roi_rect.height;
    int64_t workw = width * 2;
    int64_t workh = height * 2;
    int roix = sliceROI.roi_rect.x;
    int roiy = sliceROI.roi_rect.y;
    int roiw = sliceROI.roi_rect.width;
    int roih = sliceROI.roi_rect.height;

    //--- Set a factor -------------------------------------------//
    int64_t factor;
    if(LEVEL == 2)  factor = 16;
    else if(LEVEL == 1) factor = 4;
    else if(LEVEL == 0) factor = 1;
    else                factor = (int64_t)floor(openslide_get_level_downsample(obj, LEVEL) + 0.5);


    //--- Calculate coordinates for creating an image ------------//
    //座標計算
    int64_t CX, CY;
    CX = X + width/2;
    CY = Y + height/2;
    int64_t WX = (CX - workw / 2)*factor;   if(WX<0)    WX = 0;
    int64_t WY = (CY - workh / 2)*factor;   if(WY<0)    WY = 0;

    //倍率により座標を修正
    int adjx = WX / factor;
    int adjy = WY / factor;


    //--- Get pixel data -----------------------------------------//
    //memory
    uint32_t *dest;
    try{
        dest = new uint32_t[workw*workh];
    }
    catch(std::bad_alloc){
        return false;
    }


    //画素情報を取得
    openslide_read_region(obj, dest, WX, WY, LEVEL, workw, workh);


    //--- Convert to cv::Mat ---------------------------------------//
    if (!dst.empty())	dst.release();

    dst = cv::Mat(cv::Size(roiw, roih), CV_8UC3);

#ifdef _OPENMP
#pragma omp parallel for private(x)
#endif
       for(int y=roiy; y<(roiy+roih); y++){
           for(int x=roix; x<(roix+roiw); x++){

               //新しい座標を計算
               cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
               origin.at<double>(0, 0) = (double)x;
               origin.at<double>(1, 0) = (double)y;
               origin.at<double>(2, 0) = 1.0;

               cv::Mat coord = matrix * origin;

               //画素情報を取得するために計算した座標を修正
               bool out_of_range = false;
               int nx = (int)((double)X+ coord.at<double>(0, 0) + 0.5) - adjx;//((double)this->adjustROIRects[currentID].roi_rect.x + coord.at<double>(0, 0) + 0.5) - adjx;
               int ny = (int)((double)Y + coord.at<double>(1, 0) + 0.5) - adjy;//((double)this->adjustROIRects[currentID].roi_rect.y + coord.at<double>(1, 0) + 0.5) - adjy;
               if (nx < 0 || nx >(int)workw || ny < 0 || ny >(int)workh){
                   out_of_range = true;
               }

               //画素値を取得
               unsigned int pix;
               if(!out_of_range)
                   pix = dest[workw*ny + nx];
               else
                   pix = 0;

               //RGB変換
               int val = pix;
               int r,g,b;
               if(!out_of_range){
                 b = val & 0xff;  val = val >> 8;
                 g = val & 0xff;  val = val >> 8;
                 r = val & 0xff;
               }
               else{
                   r = g = b = 0;
               }

               //画素値を保存
               cv::Vec3b *ptr = dst.ptr<cv::Vec3b>(y-roiy);
               ptr[x-roix] = cv::Vec3b(b, g, r);

               }
           }

       //--- Release -------------------------------------------//
       delete[] dest;


       //--- Return --------------------------------------------//
       return true;

}



/*****************
 * Get ROI information
 * ***************/
bool getROIinf(std::string infpath, std::vector<roiRect>& roirects, std::vector<cv::Mat>& matricies1, std::vector<cv::Mat> &matricies0)
{
    //--- open a file ---//
    std::ifstream ifs(infpath);
    if(ifs.fail()){
        return false;
    }


    //--- load information ---//
    roiRect roi;
    std::string line;
    while (std::getline(ifs, line)){

        std::istringstream linestream(line);
        std::string token;
        int count = 0;
        cv::Mat mat = cv::Mat::zeros(cv::Size(3,3), CV_64FC1);
        cv::Mat umat = cv::Mat::zeros(cv::Size(3,3), CV_64FC1);
        while (std::getline(linestream, token, '\t')){
            if (count == 0)	roi.picNo = std::atoi(token.c_str());
            else if (count == 1) roi.imagename = token;
            else if (count == 2) roi.roi_rect.x = std::atoi(token.c_str());
            else if (count == 3) roi.roi_rect.y = std::atoi(token.c_str());
            else if (count == 4) roi.roi_rect.width = std::atoi(token.c_str());
            else if (count == 5) roi.roi_rect.height = std::atoi(token.c_str());
            else if (count == 6){
                int flag = std::atoi(token.c_str());
                if (flag == 1)	roi.rotated = true;
                else					roi.rotated = false;
            }
            else if(count == 7) umat.at<double>(0,0) = std::atof(token.c_str());
            else if(count == 8) umat.at<double>(0,1) = std::atof(token.c_str());
            else if(count == 9) umat.at<double>(0,2) = std::atof(token.c_str());
            else if(count == 10) umat.at<double>(1,0) = std::atof(token.c_str());
            else if(count == 11) umat.at<double>(1,1) = std::atof(token.c_str());
            else if(count == 12) umat.at<double>(1,2) = std::atof(token.c_str());
            else if(count == 13) umat.at<double>(2,0) = std::atof(token.c_str());
            else if(count == 14) umat.at<double>(2,1) = std::atof(token.c_str());
            else if(count == 15) umat.at<double>(2,2) = std::atof(token.c_str());
            else if(count == 16) mat.at<double>(0,0) = std::atof(token.c_str());
            else if(count == 17) mat.at<double>(0,1) = std::atof(token.c_str());
            else if(count == 18) mat.at<double>(0,2) = std::atof(token.c_str());
            else if(count == 19) mat.at<double>(1,0) = std::atof(token.c_str());
            else if(count == 20) mat.at<double>(1,1) = std::atof(token.c_str());
            else if(count == 21) mat.at<double>(1,2) = std::atof(token.c_str());
            else if(count == 22) mat.at<double>(2,0) = std::atof(token.c_str());
            else if(count == 23) mat.at<double>(2,1) = std::atof(token.c_str());
            else if(count == 24) mat.at<double>(2,2) = std::atof(token.c_str());
            count++;
        }
        //save
        roirects.push_back(roi);
        matricies1.push_back(mat);
        matricies0.push_back(umat);
    }

    ifs.close();

    return true;

}

bool getROIinf(std::string infpath, std::vector<roiRect>& roirects)
{
    //--- open a file ---//
    std::ifstream ifs(infpath);
    if(ifs.fail()){
        return false;
    }


    //--- load information ---//
    roiRect roi;
    std::string line;
    while (std::getline(ifs, line)){

        std::istringstream linestream(line);
        std::string token;
        int count = 0;
        while (std::getline(linestream, token, '\t'))
        {
            if (count == 0)	roi.picNo = std::atoi(token.c_str());
            else if (count == 1) roi.imagename = token;
            else if (count == 2) roi.roi_rect.x = std::atoi(token.c_str());
            else if (count == 3) roi.roi_rect.y = std::atoi(token.c_str());
            else if (count == 4) roi.roi_rect.width = std::atoi(token.c_str());
            else if (count == 5) roi.roi_rect.height = std::atoi(token.c_str());
            else if (count == 6){
                int flag = std::atoi(token.c_str());
                if (flag == 1)	roi.rotated = true;
                else					roi.rotated = false;
            }
            count++;
        }
        //save
        roirects.push_back(roi);
    }

    ifs.close();

    return true;

}

bool getROIinf(std::string infpath, std::vector<roiRect>& roirects, std::vector<roiRect>& firstroirects, std::vector<cv::Mat>& matricies)
{
    //--- open a file ---//
    std::ifstream ifs(infpath);
    if(ifs.fail()){
        return false;
    }


    //--- load information ---//
    roiRect roi, uroi;
    std::string line;
    while (std::getline(ifs, line)){

        std::istringstream linestream(line);
        std::string token;
        int count = 0;
        cv::Mat mat = cv::Mat::zeros(cv::Size(3,3), CV_64FC1);
        while (std::getline(linestream, token, '\t')){
            if (count == 0)	roi.picNo = std::atoi(token.c_str());
            else if (count == 1) roi.imagename = token;
            else if (count == 2) roi.roi_rect.x = std::atoi(token.c_str());
            else if (count == 3) roi.roi_rect.y = std::atoi(token.c_str());
            else if (count == 4) roi.roi_rect.width = std::atoi(token.c_str());
            else if (count == 5) roi.roi_rect.height = std::atoi(token.c_str());
            else if (count == 6){
                int flag = std::atoi(token.c_str());
                if (flag == 1)	roi.rotated = true;
                else            roi.rotated = false;
            }
            else if (count == 7) uroi.roi_rect.x = std::atoi(token.c_str());
            else if (count == 8) uroi.roi_rect.y = std::atoi(token.c_str());
            else if (count == 9) uroi.roi_rect.width = std::atoi(token.c_str());
            else if (count == 10) uroi.roi_rect.height = std::atoi(token.c_str());
            else if(count == 11) mat.at<double>(0,0) = std::atof(token.c_str());
            else if(count == 12) mat.at<double>(0,1) = std::atof(token.c_str());
            else if(count == 13) mat.at<double>(0,2) = std::atof(token.c_str());
            else if(count == 14) mat.at<double>(1,0) = std::atof(token.c_str());
            else if(count == 15) mat.at<double>(1,1) = std::atof(token.c_str());
            else if(count == 16) mat.at<double>(1,2) = std::atof(token.c_str());
            else if(count == 17) mat.at<double>(2,0) = std::atof(token.c_str());
            else if(count == 18) mat.at<double>(2,1) = std::atof(token.c_str());
            else if(count == 19) mat.at<double>(2,2) = std::atof(token.c_str());
            count++;
        }
        //save
        roirects.push_back(roi);
        firstroirects.push_back(uroi);
        matricies.push_back(mat);
    }

    ifs.close();

    return true;

}



/****************
 * Create a folder
 * ***************/
bool createFolder(std::string folderpath)
{
    //create a folder
    _mkdir(folderpath.c_str());

    //clear inside a folder
    namespace sys = std::tr2::sys;
    sys::path p(folderpath.c_str());			//delete
    std::for_each(sys::directory_iterator(p), sys::directory_iterator(),
        [](const sys::path& p){
        if (sys::is_regular_file(p)){
            std::remove(p.file_string().c_str());
        }
    });

    return true;
}






