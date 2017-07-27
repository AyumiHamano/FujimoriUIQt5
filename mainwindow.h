#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QProgressDialog>
#include <QStatusBar>
#include <QProgressBar>
#include <QProcess>

#include <iostream>
#include <vector>
#include <cstdint>
#include <direct.h>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "Structure.h"

//========================
// Prototype decralation
//==========================
bool ConvertSCN2cvMat(std::string scnpath, cv::Mat& dst, int32_t Level);
bool showROI(std::string scnpath, cv::Mat& dst, int32_t LEVEL, int64_t X, int64_t Y, int64_t width, int64_t height);
bool showROI(std::string scnpath, cv::Mat& dst, int32_t LEVEL, int64_t X, int64_t Y, int64_t width, int64_t height, cv::Mat matrix);
bool showROI(std::string scnpath, cv::Mat& dst, int32_t LEVEL, roiRect sliceROI, roiRect uppersliceROI, cv::Mat matrix);
bool getROIinf(std::string infpath, std::vector<roiRect>& roirects);
bool getROIinf(std::string infpath, std::vector<roiRect>& roirects,std::vector<cv::Mat>& matricies1, std::vector<cv::Mat>& matricies0);
bool getROIinf(std::string infpath, std::vector<roiRect>& roirects, std::vector<roiRect>& firstroirects, std::vector<cv::Mat>& matricies);
bool createFolder(std::string folderpath);



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void killLoop(){
        killLoopFlag_ = 1;
    }

    void keepLoop(){
        killLoopFlag_ = -1;
    }

    /*************
     * Detection Part
     * ***********/
    void on_pushButton_selectSCNd_clicked();

    void on_pushButton_selectOUTd_clicked();

    void on_pushButton_selectStartSCNd_clicked();

    void on_pushButton_selectStopSCNd_clicked();

    void on_pushButton_decideInputD_clicked();

    void on_radioButton_selectItv5_clicked();

    void on_radioButton_selectItv10_clicked();

    void on_radioButton_selectItv15_clicked();

    void on_radioButton_selectItv25_clicked();

    void setROIs();

    void on_pushButton_movePrevImgD_clicked();

    void on_pushButton_moveNextImgD_clicked();

    void on_pushButton_removeImgD_clicked();

    void on_pushButton_drawROId_clicked();

    void on_pushButton_decideROId_clicked();

    void on_pushButton_cancelROId_clicked();

    void on_pushButton_exeProgD_clicked();

    bool alignmentDP0722(roiRect Firstroi_rect, roiRect Lastroi_rect, std::vector<std::string> imagelist, std::vector<int> useID, std::vector<cv::Rect> workranges, std::vector<roiRect>& roipath, float hessianThreshold, float kpThreshold);

    void on_pushButton_quitProgD_clicked();


    /*************
     * Alignment Part
     * ***********/

    void tabCheck(int INDEX);

    void on_pushButton_selectINFa_clicked();

    void on_pushButton_selectSCNa_clicked();

    void on_pushButton_selectOUTa_clicked();

    void on_radioButton_level2_clicked();

    void on_radioButton_level1_clicked();

    void on_radioButton_level0_clicked();


    void enableButtons();


    void on_pushButton_decideInputA_clicked();

    void on_pushButton_selectROIa_clicked();

    void on_pushButton_drawROIa_clicked();

    void on_pushButton_decideROIa_clicked();

    void on_pushButton_cancelROIa_clicked();

    void on_pushButton_selectAdjImageA_clicked();

    void on_pushButton_removeAdjImageA_clicked();

    void on_pushButton_redrawAdjImageA_clicked();

    void on_pushButton_rotateAdjImageA_clicked();

    void on_pushButton_quitAdjustA_clicked();

    void on_pushButton_exeProgA_clicked();

    void on_pushButton_continueProgA_clicked();

    void on_pushButton_quitProgA_clicked();



    void on_pushButton_selectSkipImageS_clicked();

    void on_pushButton_drawROIs_clicked();

    void on_pushButton_decideROIs_clicked();

    void on_pushButton_cancelROIs_clicked();

    void on_pushButton_quitSkipImageS_clicked();


private:
    Ui::MainWindow *ui;

    //Widgets
    QProgressDialog *dialog;


    /************
     * Detection Part
     * ***********/
    //int
    int campusW, campusH;
    int startID, stopID;
    int roiInterval;
    int killLoopFlag_;
    int auto_detection_counter;
    const int limitW = 1200;
    const int limitH = 1200;
    const int extraW = 50;
    const int extraH = 50;

    std::vector<int>    inputIDs, roiIDs;

    //double
    double meanImageWidth, meanImageHeight;

    //string
    std::string scnfolderpath_d, outfolderpath_d;           //intput pathes
    std::string startscnpath_d, stopscnpath_d;              //start & stop files
    std::string workfolderpath_d, resultfolderpath_d, skipfolderpath_d;       //work & result & skip folders
    std::string inffolderpath;
    std::vector<std::string>    scnfiles;
    std::vector<std::string>    inputfiles;
    std::vector<std::string>    roipathes;
    std::vector<std::string>    skipimagepathes;

    //bool
    bool scnfolder_d_selected = false;
    bool outfolder_d_selected = false;
    bool window_created = false;
    bool first_roi_selected = false;
    bool auto_detection_canceled = false;

    std::vector<bool>   roi_selected;

    //roiRect
    std::vector<roiRect>    ROIRects;
    std::vector<roiRect>    allROIpath;
    std::vector<roiRect>    skippedRects;

    //cv::Mat
    std::vector<cv::Mat>    skipimages;

    /**************
     * Alignment Part
     * *************/
    //int
    int adjust_target_level;
    int align_roi_id;

    //double
    //std::vector<std::vector<double> > align_past_matricies;

    //string
    std::string inffolderpath_a, scnfolderpath_a, outfolderpath_a;
    std::string workfolderpath_a, resultfolderpath_a, infresultfolderpath_a;
    std::string sliceinfpath_a;
    std::string slicefolderpath_a, roifolderpath_a;
    std::string level_resultfolderpath_a;
    std::string roiimagename_a;


    std::vector<std::string> adjfiles;

    //bool
    bool inffolder_a_selected = false;
    bool scnfolder_a_selected = false;
    bool outfolder_a_selected = false;
    bool level_a_selected = false;
    bool adjust_roi_selected = false;

    //roiRect
    std::vector<roiRect>    adjustROIRects;
    std::vector<roiRect>    sliceROIRects;
    std::vector<roiRect>    uppersliceROIRects;

    //cv::Mat
    std::vector<cv::Mat> past_matricies;
    std::vector<cv::Mat> past_matricies_inv;
    std::vector<cv::Mat> past_level1_matricies;
    std::vector<cv::Mat> past_level1_matricies_inv;
    std::vector<cv::Mat> past_level0_matricies;
};

#endif // MAINWINDOW_H
