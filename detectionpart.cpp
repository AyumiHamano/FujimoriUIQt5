
/********************
 * Functions for automatic detection
 * 2014/12/15
 * ********************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<openslide.h>

//========================
// Grobal variables
//========================
int CURRENT_ROI_ID;     //for setting ROI
int RESIZE_TIME;        //for setting ROI
int LIMIT_ROI_ID;       //for setting ROI


std::string CURRENT_ROI_PATH;   //for setting ROI
std::string CURRENT_ROI_NAME;

cv::Rect SELECTION, FIRST_ROI_RECT;
bool START_POINT_SELECTED = false;
bool OBJECT_SELECTED = false;
bool MOUSE_INSIDE_ROIRECT = false;




//========================
// Prototype decralation
//========================
bool ConvertSCN2cvMat(std::string scnpath, cv::Mat& dst, int32_t Level);





//=======================
 // Select input & output folders
//=======================
//input
void MainWindow::on_pushButton_selectSCNd_clicked()
{
    //initialization
    this->scnfolder_d_selected = false;



    //--- Tab fixing ----------------------------//
    if(ui->tab_align->isEnabled()){
        ui->tab_align->setEnabled(false);
    }


    //--- Search SCN folder ---------------------//
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;

    //get a folder
    QString strDir = QFileDialog::getExistingDirectory(this, tr("search"), tr("C:/"), options);

    //save
    if(!strDir.isEmpty()){
        //display the folder path
        ui->lineEdit_selectSCNd->setText(strDir);
        ui->lineEdit_selectSCNd->repaint();

        //save the path
        this->scnfolderpath_d = strDir.toStdString();

        //set a flag
        this->scnfolder_d_selected = true;
    }
    else{
        //check other flag
        if(!this->outfolder_d_selected && !this->scnfolder_d_selected)
            ui->tab_align->setEnabled(true);
        return;
    }


    //--- Get all filenames inside scn folder ------//
    QDir dir(strDir);
    QStringList strlFilter;
    strlFilter << "*.scn";
    QFileInfoList list = dir.entryInfoList(strlFilter, QDir::Files);

    //error check
    if(list.isEmpty()){
       QMessageBox::information(this, "CAUTION", "No scn files are available");

       //reset
       ui->lineEdit_selectSCNd->clear();
       ui->lineEdit_selectSCNd->repaint();
       this->scnfolderpath_d.clear();
       this->scnfolder_d_selected = false;

       return;

    }


    //--- Save SCN files ---------------------------//
    this->scnfiles.clear();
    for(int i=0; i<list.size(); i++){
        scnfiles.push_back(list[i].baseName().toStdString());
    }

    //--- Enable buttons -----------------------------------//
    if(this->scnfolder_d_selected && this->outfolder_d_selected){
        ui->pushButton_selectStartSCNd->setEnabled(true);
        ui->pushButton_selectStopSCNd->setEnabled(true);
        ui->pushButton_decideInputD->setEnabled(true);
    }

}

//output
void MainWindow::on_pushButton_selectOUTd_clicked()
{

    //initialization
    this->outfolder_d_selected = false;


    //--- Tab fixing ----------------------------//
    if(ui->tab_align->isEnabled()){
        ui->tab_align->setEnabled(false);
    }


    //--- Search SCN folder ---------------------//
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;

    //get a folder
    QString strDir = QFileDialog::getExistingDirectory(this, tr("search"), tr("C:/"), options);

    //save
    if(!strDir.isEmpty()){
        //display the folder path
        ui->lineEdit_selectOUTd->setText(strDir);
        ui->lineEdit_selectOUTd->repaint();

        //save the path
        this->outfolderpath_d = strDir.toStdString();

        //set a flag
        this->outfolder_d_selected = true;
    }
    else{
        //check other flag
        if(!this->outfolder_d_selected && !this->scnfolder_d_selected)
            ui->tab_align->setEnabled(true);

        return;
    }


    //--- Enable buttons -----------------------------------//
    if(this->scnfolder_d_selected && this->outfolder_d_selected){
        ui->pushButton_selectStartSCNd->setEnabled(true);
        ui->pushButton_selectStopSCNd->setEnabled(true);
        ui->pushButton_decideInputD->setEnabled(true);
    }



}



//==============================
// Select start & stop files
//==============================
//start
void MainWindow::on_pushButton_selectStartSCNd_clicked()
{

    //--- Getting a start file ----------//
    QString SCN_DIRECTORY_PATH = QString::fromStdString(this->scnfolderpath_d);
    QFileDialog::Options options;
    QString strSelectedFilter;
    QString strFName = QFileDialog::getOpenFileName(
                this,
                tr("select"),
                SCN_DIRECTORY_PATH,
                tr("*.scn"),
                &strSelectedFilter, options);

    //--- Save the filename --------------------------------//
    if(!strFName.isEmpty()){
        //display
        ui->lineEdit_selectStartSCNd->setText(strFName);
        ui->lineEdit_selectStartSCNd->repaint();

        //save
        this->startscnpath_d = strFName.toStdString();
    }

}


//stop
void MainWindow::on_pushButton_selectStopSCNd_clicked()
{
    //--- Set current directory ---------------------------//
    QString SCN_DIRECTORY_PATH = QString::fromStdString(this->scnfolderpath_d);


    //--- Getting a stop file ----------//
    QFileDialog::Options options;
    QString strSelectedFilter;
    QString strFName = QFileDialog::getOpenFileName(
                this,
                tr("select"),
                SCN_DIRECTORY_PATH,
                tr("*.scn"),
                &strSelectedFilter, options);

    //--- Save the filename --------------------------------//
    if(!strFName.isEmpty()){
        //display
        ui->lineEdit_selectStopSCNd->setText(strFName);
        ui->lineEdit_selectStopSCNd->repaint();

        //save
        this->stopscnpath_d = strFName.toStdString();
    }

}



//=============================
// Decide input
//============================
void MainWindow::on_pushButton_decideInputD_clicked()
{

    //--- Get IDs of start & stop files -----------------------------//
    if(this->startscnpath_d.empty() && this->stopscnpath_d.empty()){
        QMessageBox::information(this, "CAUTION", "Please select start & stop scn files");
        return;
    }

    bool start_selected = false;
    bool stop_selected = false;
    for(int id=0; id<(int)this->scnfiles.size(); id++){

        //get a current filename
        std::string filename = this->scnfiles[id];

        //search
        std::string::size_type idx_start = this->startscnpath_d.find(filename);
        std::string::size_type idx_stop = this->stopscnpath_d.find(filename);

        //save
        if(idx_start != std::string::npos){
            this->startID = id;
            start_selected = true;
        }
        if(idx_stop != std::string::npos){
            this->stopID = id;
            stop_selected = true;
        }

        //break check
        if(start_selected && stop_selected)
            break;
    }


    //check IDs
    if(startID > stopID){
        QMessageBox::information(this, "CAUTION", "Please check the order of start & stop files");
        return;
    }


    //--- Get input files & IDs ------------------------------------------//
    this->inputfiles.clear();
    this->inputIDs.clear();
    for(int id=this->startID; id<=this->stopID; id++){
        this->inputfiles.push_back(this->scnfiles[id]);
        this->inputIDs.push_back(id);
    }

    //check the number of input images
    int inputNum = (int)this->inputfiles.size();
    if(inputNum<3){
        QMessageBox::information(this, "CAUTION", "Please select at least 3 files");
        return;
    }


    //--- Disable buttons ------------------------------------------//
    ui->pushButton_selectSCNd->setEnabled(false);
    ui->pushButton_selectOUTd->setEnabled(false);
    ui->pushButton_selectStartSCNd->setEnabled(false);
    ui->pushButton_selectStopSCNd->setEnabled(false);
    ui->pushButton_decideInputD->setEnabled(false);


    //--- Create work & result folders -----------------------------//
    this->workfolderpath_d = this->outfolderpath_d + "/work";
    _mkdir(this->workfolderpath_d.c_str());

    this->resultfolderpath_d = this->outfolderpath_d + "/RESULT";
    _mkdir(this->resultfolderpath_d.c_str());

    this->skipfolderpath_d = this->resultfolderpath_d + "/skip";
    createFolder(this->skipfolderpath_d);

    this->inffolderpath = this->resultfolderpath_d + "/inf";
    createFolder(this->inffolderpath);

    this->resultfolderpath_d += "/detection";
    createFolder(this->resultfolderpath_d);



    //--- Convert input scn files into tiff images -----------------//
    //show the status
    int count = 0;
    QString status = "Loading... " + QString::number(count) + "/" + QString::number((int)this->inputfiles.size());
    ui->label_showImageNumD->setText(status);
    ui->label_showImageNumD->repaint();

    //variable for backup
    std::vector<int>         tmpids;
    std::vector<std::string> tmpfiles;
    std::vector<std::string> skipfiles;

    //variables for mean width & height of images
    double sumw, sumh;
    sumw = sumh = 0;

    //variables for finding max image size
    int maxW, maxH;
    maxW = maxH = 0;
    for(int id=0; id<(int)this->inputfiles.size(); id++){

        //get an input file
        int         inputID = this->inputIDs[id];
        std::string filename = this->inputfiles[id];
        std::string scnpath = this->scnfolderpath_d + "/" + filename + ".scn";
        std::string savepath = this->workfolderpath_d + "/" + filename + ".tiff";

        //call a convert function
        cv::Mat img = cv::imread(savepath, 1);
        if(!img.empty()){
            //save the name
            tmpfiles.push_back(filename);
            //save the id
            tmpids.push_back(inputID);

            //add width & height
            sumw += (double)img.cols;
            sumh += (double)img.rows;
            //finding max width & height
            if(maxW < (int)img.cols)    maxW = (int)img.cols;
            if(maxH < (int)img.rows)    maxH = (int)img.rows;

            //increment
            count++;
        }
        else{
            bool cvtcheck = ConvertSCN2cvMat(scnpath, img, 3);
            qApp->processEvents();

            //save the converted image
            if(cvtcheck){
                //save the name
                tmpfiles.push_back(filename);
                //save the ID
                tmpids.push_back(inputID);
                //save the image
                cv::imwrite(savepath, img);
                //add width & height
                sumw += (float)img.cols;
                sumh += (float)img.rows;
                //finding max width & height
                if(maxW < (int)img.cols)    maxW = (int)img.cols;
                if(maxH < (int)img.rows)    maxH = (int)img.rows;
                //increment
                count++;
            }
            else{
                //save the name
                skipfiles.push_back(filename);
            }
        }

        //show status
        QString status = "Loading... " + QString::number(count) + "/" + QString::number((int)this->inputfiles.size()) + " images";
        ui->label_showImageNumD->setText(status);
        ui->label_showImageNumD->repaint();



    }//id end


    //check the number of images
    if(count == 0){
        QMessageBox::information(this, "FATAL ERROR", "!! FATAL !!\nNo images are available for detection");
        exit(-1);
    }
    else if(count < 3){
        QMessageBox::information(this, "FATAL ERROR", "!! FATAL !!\nWe need at least 3 images for detection");
        exit(-1);
    }



    //update inputfiles
    this->inputfiles.clear();
    this->inputIDs.clear();
    for(int i=0; i<(int)tmpfiles.size(); i++){
        this->inputfiles.push_back(tmpfiles[i]);
        this->inputIDs.push_back(tmpids[i]);
    }


    //calculate mean width & height of image
    this->meanImageWidth = sumw / (double)count;
    this->meanImageHeight = sumh / (double)count;


    //--- Creating a campus and attaching each image on it ------------//
    //--- for the size normalization ----------------------------------//
    this->campusW = maxW + this->extraW;
    this->campusH = maxH + this->extraH;
    cv::Mat campus = cv::Mat::zeros(cv::Size(campusW, campusH), CV_8UC3);
    for(int i=0; i<(int)this->inputfiles.size(); i++){
        //get an imagepath
        std::string filename = this->inputfiles[i];
        std::string imagepath = this->workfolderpath_d + "/" + filename + ".tiff";

        //loading an image
        cv::Mat img = cv::imread(imagepath, 1);

        //getting a ROI
        int X = (this->campusW - (int)img.cols) / 2;
        int Y = (this->campusH - (int)img.rows) / 2;
        cv::Mat roi = campus(cv::Rect(X, Y, (int)img.cols, (int)img.rows));

        //copy the input image to the ROI
        img.copyTo(roi);

        //save the resized image
        cv::imwrite(imagepath, campus);

        //reset the campus
        campus.setTo(0);

        //show status
        QString status = "Resizing... " + QString::number(i+1) + "/" + QString::number((int)this->inputfiles.size()) + " images";
        ui->label_showImageNumD->setText(status);
        ui->label_showImageNumD->repaint();
        qApp->processEvents();

    }

    //show the status
    status = QString::number(count) + "/" + QString::number((int)this->inputfiles.size()) + " images are available";
    ui->label_showImageNumD->setText(status);
    ui->label_showImageNumD->repaint();



    //--- Save image names which were skipped at loading process ------//
    if((int)skipfiles.size() != 0){
        std::string skipfilepath = this->skipfolderpath_d + "/skipSCNfiles.txt";
        std::ofstream ofs(skipfilepath, ios::out);
        if(!ofs.fail()){
            for(int i=0; i<(int)skipfiles.size(); i++){
                //get a skiped filename
                std::string filename = skipfiles[i];
                //create a scn path
                std::string scnpath = this->scnfolderpath_d + "/" + filename + ".scn";
                //write
                ofs << scnpath << std::endl;
            }
            //close
            ofs.close();
        }
    }

    //--- Enable buttons ---------------------------------------------//
    ui->radioButton_selectItv5->setEnabled(true);
    ui->radioButton_selectItv10->setEnabled(true);
    ui->radioButton_selectItv15->setEnabled(true);
    ui->radioButton_selectItv25->setEnabled(true);


    //--- Initialization ---------------------------------------------//
    this->skipimagepathes.clear();
    this->skipimages.clear();
    this->skippedRects.clear();


}



//============================
// Set ROI interval
//===========================
void MainWindow::on_radioButton_selectItv5_clicked()
{
    this->roiInterval = 5;
    this->setROIs();
}


void MainWindow::on_radioButton_selectItv10_clicked()
{
    this->roiInterval = 10;
    this->setROIs();
}


void MainWindow::on_radioButton_selectItv15_clicked()
{
    this->roiInterval = 15;
    this->setROIs();
}



void MainWindow::on_radioButton_selectItv25_clicked()
{
    this->roiInterval = 25;
    this->setROIs();

}

//set up setting of ROI
void MainWindow::setROIs()
{

    //--- Get image pathes for ROI ----------------------//
    int fileSize = (int)inputfiles.size();
    this->roipathes.clear();
    this->roiIDs.clear();
    this->roi_selected.clear();
    for(int id=0; id<fileSize; id+=this->roiInterval){
        //get a filename
        std::string filename = this->inputfiles[id];

        //get an ID
        int ID = this->inputIDs[id];

        //create an image path
        std::string imagepath = this->workfolderpath_d + "/" + filename + ".tiff";

        //save
        this->roipathes.push_back(imagepath);
        this->roiIDs.push_back(ID);
        this->roi_selected.push_back(false);

        //must cut the last file
        if(id != fileSize - 1 && id + this->roiInterval > fileSize - 1){
            filename = this->inputfiles[fileSize - 1];
            imagepath = this->workfolderpath_d + "/" + filename + ".tiff";

            this->roipathes.push_back(imagepath);
            this->roiIDs.push_back(ID);
            this->roi_selected.push_back(false);

            break;
        }
    }

    //get memory for roiRect
    this->ROIRects.clear();
    this->ROIRects.resize((int)this->roipathes.size());

    //initialize grobal variables
    CURRENT_ROI_ID = 0;
    CURRENT_ROI_PATH = this->roipathes[CURRENT_ROI_ID];
    LIMIT_ROI_ID = (int)this->roipathes.size()- 1;

    std::string imagename = CURRENT_ROI_PATH;
    std::string::size_type idx = imagename.find_last_of("/");
    std::string::size_type idx2 = imagename.find(".");
    imagename.erase(idx2, imagename.size());
    imagename.erase(0, idx+1);

    CURRENT_ROI_NAME = imagename;




    //--- Create a window -------------------------------//
    if(!this->window_created){
        cv::namedWindow("ROI", 1);
        this->window_created = true;
    }


    //--- Enable buttons ---------------------------------//
    if(this->window_created){
        ui->pushButton_moveNextImgD->setEnabled(true);
        ui->pushButton_movePrevImgD->setEnabled(false);
        ui->pushButton_removeImgD->setEnabled(true);
        ui->pushButton_drawROId->setEnabled(true);
    }


    //--- Show the first images --------------------------//
    //load an image
    cv::Mat img = cv::imread(CURRENT_ROI_PATH, 1);
    if(img.empty()){
        QMessageBox::information(this, "FATAL ERROR", "No images are available for ROI");
        exit(-1);
    }

    //show the imagename
    QString Roipath = QString::fromStdString(CURRENT_ROI_PATH);
    ui->lineEdit_showROINameD->setText(Roipath);
    ui->lineEdit_showROINameD->repaint();

    //resize
    cv::Mat dispimg;
    if(img.cols>this->limitW || img.rows>this->limitH){
        float roiw = (float)img.cols;
        float roih = (float)img.rows;
        double factorx, factory;
        factorx = factory = 1.0;
        while (roiw > this->limitW || roih > this->limitH){
            roiw /= 2.0;
            roih /= 2.0;
            factorx *= 2.0;
            factory *= 2.0;
        }

        RESIZE_TIME = (int)factorx;//2;

        cv::resize(img, dispimg, cv::Size(img.cols/RESIZE_TIME, img.rows/RESIZE_TIME));

    }
    else{
        dispimg = img.clone();
        RESIZE_TIME = 1;
    }

    //show the image
    cv::imshow("ROI", dispimg);
    cv::waitKey(10);

    //show the number of selected ROI
    int count = 0;
    for(int i=0; i<(int)this->roi_selected.size(); i++){
        if(this->roi_selected[i])
            count++;
    }

    QString status = QString::number(count) + "/" + QString::number((int)this->roi_selected.size()) + " ROIs selected from " + QString::number(((int)this->inputfiles.size())) + " images";
    ui->label_showROInum->setText(status);
    ui->label_showROInum->repaint();

}



//==========================
// Move to previous or next & remove
//===========================
//move to previous
void MainWindow::on_pushButton_movePrevImgD_clicked()
{
    //--- Select an imagepath -----------------------------/
    CURRENT_ROI_ID--;
    if(CURRENT_ROI_ID <= 0){
        //reset
        CURRENT_ROI_ID = 0;
        //disable & enable
        ui->pushButton_movePrevImgD->setEnabled(false);
        ui->pushButton_moveNextImgD->setEnabled(true);
    }
    else{
        //enable
        ui->pushButton_moveNextImgD->setEnabled(true);
        ui->pushButton_movePrevImgD->setEnabled(true);
    }

    //load
    CURRENT_ROI_PATH = this->roipathes[CURRENT_ROI_ID];


    //get an imagename
    std::string imagename = CURRENT_ROI_PATH;
    std::string::size_type idx = imagename.find_last_of("/");
    std::string::size_type idx2 = imagename.find(".");
    imagename.erase(idx2, imagename.size());
    imagename.erase(0, idx+1);
    CURRENT_ROI_NAME = imagename;

    //show the name
    QString roi_path = QString::fromStdString(CURRENT_ROI_PATH);
    ui->lineEdit_showROINameD->setText(roi_path);
    ui->lineEdit_showROINameD->repaint();


    //--- Load the image and resize ---------------------//
    cv::Mat img = cv::imread(CURRENT_ROI_PATH, 1);
    if(img.empty()){
        QMessageBox::information(this, "ERROR", "An image is not available for ROI\n" + QString::fromStdString(CURRENT_ROI_PATH));
        exit(-1);
    }


    //resize
    cv::Mat dispimg;
    bool resized = false;
    if(img.cols > this->limitW || img.rows>this->limitH){
        cv::resize(img, dispimg, cv::Size(img.cols/RESIZE_TIME, img.rows/RESIZE_TIME));
        resized = true;
    }else
        dispimg = img.clone();
    img.release();

    //draw a renctangle
    //draw a renctangle
    if(this->roi_selected[CURRENT_ROI_ID]){
        cv::Rect roi = this->ROIRects[CURRENT_ROI_ID].roi_rect;
        if(resized){
            cv::rectangle(dispimg, cv::Rect(roi.x / RESIZE_TIME, roi.y / RESIZE_TIME,
                          roi.width / RESIZE_TIME, roi.height / RESIZE_TIME),
                          cv::Scalar(0,0,255), 1, CV_AA, 0);
        }
        else
            cv::rectangle(dispimg, roi, cv::Scalar(0,0,255), 1, CV_AA, 0);
     }

    //show the image
    cv::imshow("ROI", dispimg);
    cv::waitKey(10);

}


//move to next
void MainWindow::on_pushButton_moveNextImgD_clicked()
{
    //--- Select an imagepath -----------------------------/
    CURRENT_ROI_ID++;
    if(CURRENT_ROI_ID >= LIMIT_ROI_ID ){
        //reeset
        CURRENT_ROI_ID = LIMIT_ROI_ID;
        //disable & enable
        ui->pushButton_movePrevImgD->setEnabled(true);
        ui->pushButton_moveNextImgD->setEnabled(false);
    }
    else{
        //enable
        ui->pushButton_moveNextImgD->setEnabled(true);
        ui->pushButton_movePrevImgD->setEnabled(true);
    }

    //load
    CURRENT_ROI_PATH = this->roipathes[CURRENT_ROI_ID];

    //get an imagename
    std::string imagename = CURRENT_ROI_PATH;
    std::string::size_type idx = imagename.find_last_of("/");
    std::string::size_type idx2 = imagename.find(".");
    imagename.erase(idx2, imagename.size());
    imagename.erase(0, idx+1);
    CURRENT_ROI_NAME = imagename;

    //show the name
    QString roi_path = QString::fromStdString(CURRENT_ROI_PATH);
    ui->lineEdit_showROINameD->setText(roi_path);
    ui->lineEdit_showROINameD->repaint();


    //--- Load the image and resize ---------------------//
    cv::Mat img = cv::imread(CURRENT_ROI_PATH, 1);
    if(img.empty()){
        QMessageBox::information(this, "ERROR", "An image is not available for ROI");
        exit(-1);
    }


    //resize
    cv::Mat dispimg;
    bool resized = false;
    if(img.cols > this->limitW || img.rows>this->limitH){
        cv::resize(img, dispimg, cv::Size(img.cols/RESIZE_TIME, img.rows/RESIZE_TIME));
        resized = true;
    }else
        dispimg = img.clone();
    img.release();

    //draw a renctangle
    if(this->roi_selected[CURRENT_ROI_ID]){
        cv::Rect roi = this->ROIRects[CURRENT_ROI_ID].roi_rect;
        if(resized){
            cv::rectangle(dispimg, cv::Rect(roi.x / RESIZE_TIME, roi.y / RESIZE_TIME,
                          roi.width / RESIZE_TIME, roi.height / RESIZE_TIME),
                          cv::Scalar(0,0,255), 1, CV_AA, 0);
        }
        else
            cv::rectangle(dispimg, roi, cv::Scalar(0,0,255), 1, CV_AA, 0);
     }

    //show the image
    cv::imshow("ROI", dispimg);
    cv::waitKey(10);
}


//remove
void MainWindow::on_pushButton_removeImgD_clicked()
{
    //--- Show a caution -------------------------------------------//
    QMessageBox::StandardButton res = QMessageBox::question(this, "FINAL CHECK", "Are you sure you want remove this image from input?", QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Cancel)
        return;

    //double check
    if(this->roi_selected[CURRENT_ROI_ID]){
        res = QMessageBox::question(this, "FINAL CHECK", "This image has a ROI. Are you really sure you want remove this image from input?", QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Cancel)
            return;
    }


    //--- Save current image to skip folder ------------------------//
    cv::Mat skipimg = cv::imread(CURRENT_ROI_PATH, 1);
    if(!skipimg.empty()){

        //make a path
        std::string skipsavepath = this->skipfolderpath_d + "/" + CURRENT_ROI_NAME + ".tiff";

        //save
        cv::imwrite(skipsavepath, skipimg);


        //save the skipped image information as a roiRect data
        roiRect roi;
        roi.imagename = CURRENT_ROI_NAME;
        roi.picNo = this->roiIDs[CURRENT_ROI_ID];
        roi.rotated = false;
        this->skippedRects.push_back(roi);

    }


    //--- Remove current imagepath from ROI -----------------------//
    int targetID = -1;
    int imageID = -1;
    std::string nfilename;
    if(CURRENT_ROI_ID <= 0){
        //get a new filename
        nfilename = this->inputfiles[0+1];

        //get an image ID
        imageID = this->inputIDs[0+1];

        //get an ID of current imagepath in inputfiles
        targetID = 0;
    }
    else if(CURRENT_ROI_ID >= LIMIT_ROI_ID){
        targetID = (int)this->inputfiles.size() - 1;

        imageID = this->inputIDs[targetID - 1];

        nfilename = this->inputfiles[targetID - 1];

    }
    else{
        //find an ID of current imagepath in inputfiles
        for(int id=0; id<(int)this->inputfiles.size(); id++){
            std::string filename = this->inputfiles[id];
            std::string::size_type idx = CURRENT_ROI_PATH.find(filename);
            if(idx!=std::string::npos){
                targetID = id;
                break;
            }
        }

        nfilename = this->inputfiles[targetID - 1];

        imageID = this->inputIDs[targetID - 1];
    }


    //save new pathes
    std::string nimagepath = this->workfolderpath_d + "/" + nfilename + ".tiff";
    this->roipathes[CURRENT_ROI_ID] = nimagepath;
    this->roiIDs[CURRENT_ROI_ID] = imageID;
    this->roi_selected[CURRENT_ROI_ID] = false;
    CURRENT_ROI_PATH = nimagepath;

    std::string imagename = CURRENT_ROI_PATH;
    std::string::size_type idx = imagename.find_last_of("/");
    std::string::size_type idx2 = imagename.find(".");
    imagename.erase(idx2, imagename.size());
    imagename.erase(0, idx+1);

    CURRENT_ROI_NAME = imagename;



    //error check
    if(targetID == -1){
        QMessageBox::information(this, "FATAL ERROR", "!!FATAL!! We can't remove an image from input");
        exit(-1);
    }


    //check whether same imagepathes are saved
    std::vector<std::string> tmppathes;
    std::vector<int>         tmproiids;
    std::vector<roiRect>     tmprois;
    std::vector<bool>        tmpselected;
    bool                     same_checked;
    for(int i=0; i<(int)this->roipathes.size(); i++){

        //get a current target path
        std::string checkpath = this->roipathes[i];
        same_checked = false;

        //for check
        //qDebug() << QString::fromStdString(checkpath);

        for(int j=0; j<(int)this->roipathes.size(); j++){
            if(j==i)    continue;

            //get a compared path
            std::string path = this->roipathes[j];

            //compare
            if(path == checkpath && !this->roi_selected[i]){
                if(this->roi_selected[j]){
                    same_checked = true;
                }
                else if(i>j){
                    same_checked = true;
                }
            }

            if(same_checked)
                break;
        }

        //save
        if(!same_checked){
            tmppathes.push_back(checkpath);
            tmpselected.push_back(this->roi_selected[i]);
            tmproiids.push_back(this->roiIDs[i]);
            tmprois.push_back(this->ROIRects[i]);
        }
    }


    //update the roipathes
    this->roipathes.clear();
    this->roiIDs.clear();
    this->roi_selected.clear();
    this->ROIRects.clear();
    for(int i=0; i<(int)tmppathes.size(); i++){
        this->roipathes.push_back(tmppathes[i]);
        this->roiIDs.push_back(tmproiids[i]);
        this->roi_selected.push_back(tmpselected[i]);
        this->ROIRects.push_back(tmprois[i]);
    }

    //update CURRENT_ROI_ID
    for(int id=0; id<(int)this->roipathes.size(); id++){
        std::string path = this->roipathes[id];
        if(path == CURRENT_ROI_PATH){
            CURRENT_ROI_ID = id;
            break;
        }
    }

    //update LIMIT_ROI_ID
    LIMIT_ROI_ID = (int)this->roipathes.size() - 1;



    //--- Remove current imagepath from INPUT ----------------------//
    std::vector<std::string> tmpfiles;
    std::vector<int>         tmpids;
    for(int id=0; id<(int)this->inputfiles.size(); id++){
        if(id != targetID){
            tmpfiles.push_back((this->inputfiles[id]));
            tmpids.push_back(this->inputIDs[id]);
        }
    }

    //save
    this->inputfiles.clear();
    this->inputIDs.clear();
    for(int i=0; i<(int)tmpfiles.size();i++){
        this->inputfiles.push_back(tmpfiles[i]);
        this->inputIDs.push_back(tmpids[i]);
    }

    //check the number of input images
    int inputNum = (int)this->inputfiles.size();
    if(inputNum<3){
        QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! We need at least 3 images as input");
        exit(-1);
    }



    //--- Load the new image and display --------------------------//
    CURRENT_ROI_PATH = this->roipathes[CURRENT_ROI_ID];

    //show the imagename
    QString roi_path = QString::fromStdString(CURRENT_ROI_PATH);
    ui->lineEdit_showROINameD->setText(roi_path);
    ui->lineEdit_showROINameD->repaint();

    //update label
    int count = 0;
    for(int i=0; i<(int)this->roi_selected.size(); i++){
        if(this->roi_selected[i])
            count++;
    }
    QString status = QString::number(count) + "/" + QString::number((int)this->roi_selected.size()) + " ROIs selected from " + QString::number(((int)this->inputfiles.size())) + " images";
    ui->label_showROInum->setText(status);
    ui->label_showROInum->repaint();

    //load an image
    cv::Mat img = cv::imread(CURRENT_ROI_PATH, 1);
    if(img.empty()){
        QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! An image is not available for ROI\n" + QString::fromStdString(CURRENT_ROI_PATH));
        exit(-1);
    }

    //resize
    cv::Mat dispimg;
    bool resized = false;
    if(img.cols > this->limitW || img.rows> this->limitH){
        cv::resize(img, dispimg, cv::Size(img.cols/RESIZE_TIME, img.rows/RESIZE_TIME));
        resized = true;
    }else
        dispimg = img.clone();
    img.release();

    //draw a renctangle
    if(this->roi_selected[CURRENT_ROI_ID]){
        if(resized)
            cv::rectangle(dispimg, cv::Rect(this->ROIRects[CURRENT_ROI_ID].roi_rect.x / RESIZE_TIME, this->ROIRects[CURRENT_ROI_ID].roi_rect.y / RESIZE_TIME,
                          this->ROIRects[CURRENT_ROI_ID].roi_rect.width / RESIZE_TIME, this->ROIRects[CURRENT_ROI_ID].roi_rect.height / RESIZE_TIME),
                          cv::Scalar(0,0,255), 1, CV_AA, 0);
        else
            cv::rectangle(dispimg, this->ROIRects[CURRENT_ROI_ID].roi_rect, cv::Scalar(0,0,255), 1, CV_AA, 0);
     }

    //show the image
    cv::imshow("ROI", dispimg);
    cv::waitKey(10);


    //--- Enable & disable buttons ---------------------------------------------------------//
    if(CURRENT_ROI_ID == 0){
        ui->pushButton_moveNextImgD->setEnabled(true);
        ui->pushButton_movePrevImgD->setEnabled(false);
    }
    else if(CURRENT_ROI_ID == LIMIT_ROI_ID){
        ui->pushButton_moveNextImgD->setEnabled(false);
        ui->pushButton_movePrevImgD->setEnabled(true);
    }


}



//===========================
// Draw ROIs
//===========================
//callback function for first ROI
void on_mouse_first(int event, int x, int y, int flags, void* param){

    static cv::Point2i origin;
    cv::Rect *inputrect = static_cast<cv::Rect*>(param);

    //--- Calculate Coordinates of Selected Area (by clidk and drag) ------------//

    //--- Process a Start and a Finish Selecting Events (i.e. button-up and -down) ---//
    switch (event){
    case cv::EVENT_LBUTTONDOWN:
        if(!START_POINT_SELECTED){
            origin = cv::Point2i(x, y);
            SELECTION = cv::Rect(x, y, 0, 0);
            START_POINT_SELECTED = true;
            OBJECT_SELECTED = false;
        }
        break;
    case cv::EVENT_LBUTTONUP:
        if(x>=0 && x<inputrect->width && y>=0 && y<inputrect->height){
            START_POINT_SELECTED = false;
            OBJECT_SELECTED = true;
            break;
        }

    case cv::EVENT_MOUSEMOVE:
        if (START_POINT_SELECTED){
            SELECTION.x = CV_IMIN(x, origin.x);
            SELECTION.y = CV_IMIN(y, origin.y);
            SELECTION.x = CV_IMAX(SELECTION.x, 0);
            SELECTION.y = CV_IMAX(SELECTION.y, 0);

            SELECTION.width = CV_IABS(x - origin.x);
            SELECTION.height = CV_IABS(y - origin.y);

            if(x<0){
                SELECTION.width = origin.x;
            }
            else if(x>inputrect->width){
                SELECTION.width = CV_IABS(inputrect->width - origin.x);
            }

            if(y<0){
                SELECTION.height = origin.y;
            }
            else if(y>inputrect->height){
                SELECTION.height = CV_IABS(inputrect->height - origin.y);
            }

        }
        else if(!OBJECT_SELECTED){
            SELECTION.x = x;
            SELECTION.y = y;
            SELECTION.width = 0;
            SELECTION.height = 0;
        }
        break;
    }

}

//callback function for other ROIs
void on_mouse_others(int event, int x, int y, int flags, void* param){

    cv::Rect *inputrect = static_cast<cv::Rect*>(param);

    //set a first point
    //static cv::Point2i origin = cv::Point2i(inputrect->x + inputrect->width/2, inputrect->y + inputrect->height/2);

    int minx = inputrect->x + inputrect->width/2 - inputrect->width/10; //-10;
    int maxx = inputrect->x + inputrect->width/2 + inputrect->width/10; //+ 10;
    int miny = inputrect->y + inputrect->height/2 - inputrect->height/10; //- 10;
    int maxy = inputrect->y + inputrect->height/2 + inputrect->height/10; //+ 10;


    //--- Process a Start and a Finish Selecting Events (i.e. button-up and -down) ---//
    switch (event){
    case cv::EVENT_MOUSEMOVE:
        if(x > minx && x < maxx && y > miny && y < maxy && !MOUSE_INSIDE_ROIRECT){
            MOUSE_INSIDE_ROIRECT = true;
        }

        if(!OBJECT_SELECTED && MOUSE_INSIDE_ROIRECT){
            SELECTION = cv::Rect(x - inputrect->width / 2, y - inputrect->height / 2, inputrect->width, inputrect->height);
        }
        else if(!OBJECT_SELECTED && !MOUSE_INSIDE_ROIRECT){
            SELECTION = *inputrect;
        }

        break;
    case cv::EVENT_LBUTTONDOWN:
        OBJECT_SELECTED = true;
        MOUSE_INSIDE_ROIRECT = false;
        break;
    }
}

//get a next of previous ID of current ROI
int findCriterionROI(std::vector<bool>& slctedROI)
{
    //--- Check whether current_roi_id has already been selected ---//
    if(slctedROI[CURRENT_ROI_ID])
        return CURRENT_ROI_ID;


    //--- Count the number of selected ROI -----------------------//
    int count = 0;
    int roiNum = (int)slctedROI.size();

    std::vector<int> IDs;
    for(int i=0; i<roiNum; i++){
        if(slctedROI[i]){
            count++;
            IDs.push_back(i);
        }
    }

    //--- get a next or previous ID --------------------------//
    int returnID;
    if(count == 1){
        return(IDs[0]);
    }
    else{
        //search
        int i = 1;
        int j = 1;
        int count = 0;
        while(1){

            //set IDs
            int preID = CURRENT_ROI_ID - i;
            int nexID = CURRENT_ROI_ID + j;

            //ID check
            if(preID>=0 && slctedROI[preID]){
               returnID = preID;
               break;
            }
            else if(nexID <= LIMIT_ROI_ID && slctedROI[nexID]){
                returnID = nexID;
                break;
            }

            //increment
            if(preID > 0)   i++;
            if(nexID < LIMIT_ROI_ID)    j++;

            //break check
            if(count > 100000000){
                returnID = -1;
                break;
            }

            //add
            count++;
        }
    }


    return returnID;
}



//draw ROI
void MainWindow::on_pushButton_drawROId_clicked()
{
    //--- Enable & disable buttons ------------------//
    //radio buttons
    ui->radioButton_selectItv5->setEnabled(false);
    ui->radioButton_selectItv10->setEnabled(false);
    ui->radioButton_selectItv15->setEnabled(false);
    ui->radioButton_selectItv25->setEnabled(false);

    //move & remove buttons
    ui->pushButton_moveNextImgD->setEnabled(false);
    ui->pushButton_movePrevImgD->setEnabled(false);
    ui->pushButton_removeImgD->setEnabled(false);

    //drawing & decision & repaint buttons
    ui->pushButton_drawROId->setEnabled(false);

    //set a new text of pushbutton
    ui->pushButton_drawROId->setText(tr("描画中"));
    ui->pushButton_drawROId->repaint();


    //--- Load an image ----------------------------//
    cv::Mat img = cv::imread(CURRENT_ROI_PATH, 1);
    if(img.empty()){
        QMessageBox::information(this, "FATAL ERROR", "An image is not available for ROI");
        exit(-1);
    }


    //--- Resize -----------------------------------//
    cv::Mat resizeimg;
    bool img_resized = false;
    if(img.cols > this->limitW || img.rows> this->limitH){
        cv::resize(img, resizeimg, cv::Size(img.cols/RESIZE_TIME, img.rows/RESIZE_TIME));
        img_resized = true;
    }
    else{
        img.copyTo(resizeimg);
    }


    //--- Initialization following current state -----//
    cv::Rect inputrect(0,0,resizeimg.cols, resizeimg.rows);
    cv::Rect criteriaroi;
    if(!this->first_roi_selected){

        //initialization
        SELECTION.x = SELECTION.y = SELECTION.width = SELECTION.height = 0;
        OBJECT_SELECTED = false;
        START_POINT_SELECTED = false;

        //call a callback function
        cv::setMouseCallback("ROI", on_mouse_first, &inputrect);
    }
    else{
        //get a next or previous ID of current ROI
        int roiID = findCriterionROI(this->roi_selected);

        if(roiID == -1){
            criteriaroi = cv::Rect(FIRST_ROI_RECT.x, FIRST_ROI_RECT.y, FIRST_ROI_RECT.width, FIRST_ROI_RECT.height);
        }
        else{
            cv::Rect roi = this->ROIRects[roiID].roi_rect;
            criteriaroi = cv::Rect(roi.x, roi.y, roi.width, roi.height);
        }


        //check resize flag
        if(img_resized){
            criteriaroi = cv::Rect(criteriaroi.x / RESIZE_TIME, criteriaroi.y / RESIZE_TIME, criteriaroi.width / RESIZE_TIME, criteriaroi.height / RESIZE_TIME);
        }

        //check roi's rocation on a current image
        if(criteriaroi.x+criteriaroi.width > resizeimg.cols)
            criteriaroi.x = resizeimg.cols - criteriaroi.width;

        if(criteriaroi.y+criteriaroi.height > resizeimg.rows)
            criteriaroi.y = resizeimg.rows - criteriaroi.height;


        //initialization
        SELECTION.x = criteriaroi.x;
        SELECTION.y = criteriaroi.y;
        SELECTION.width = criteriaroi.width;
        SELECTION.height = criteriaroi.height;
        OBJECT_SELECTED = true;
        MOUSE_INSIDE_ROIRECT = false;


        //call a callback function
        cv::setMouseCallback("ROI", on_mouse_others, &criteriaroi);

    }


    //=== LOOP ===//
    //initialization for breaking unlimited loop
    this->killLoopFlag_ = 0;

    //disable buttons
    ui->pushButton_decideROId->setEnabled(false);
    ui->pushButton_cancelROId->setEnabled(false);


    while(1){

        //display
        cv::Mat dispimg = resizeimg.clone();

        //adjust coordinate
        if(SELECTION.x < 0) SELECTION.x = 0;
        else if(SELECTION.x + SELECTION.width > resizeimg.cols)   SELECTION.x = resizeimg.cols - SELECTION.width;
        if(SELECTION.y < 0) SELECTION.y = 0;
        else if(SELECTION.y+SELECTION.height > resizeimg.rows) SELECTION.y = resizeimg.rows - SELECTION.height;


        //display the coordinate
        QString coord_str = "x:" + QString::number(SELECTION.x) + " y:" + QString::number(SELECTION.y) + " (" + QString::number(SELECTION.width) + ", " + QString::number(SELECTION.height) + ")";
        ui->label_showROIcoordD->setText(coord_str);
        ui->label_showROIcoordD->repaint();


        //drawing
        if(!OBJECT_SELECTED && SELECTION.width>0 && SELECTION.height > 0){
            cv::rectangle(dispimg, SELECTION, CV_RGB(0,0,255), 1, CV_AA, 0);
            if(this->first_roi_selected){
                cv::rectangle(dispimg, cv::Point(SELECTION.x + SELECTION.width/2 - SELECTION.width/10, SELECTION.y + SELECTION.height/2 - SELECTION.height/10),
                              cv::Point(SELECTION.x + SELECTION.width/2 + SELECTION.width/10, SELECTION.y + SELECTION.height/2 + SELECTION.height/10),
                              CV_RGB(0,255,0), 1, CV_AA, 0);
            }
        }
        //drew
        else if(OBJECT_SELECTED){

            //enable buttons
            ui->pushButton_decideROId->setEnabled(true);
            ui->pushButton_cancelROId->setEnabled(true);


            //draw a selected rectangle & display
            cv::rectangle(dispimg, SELECTION, CV_RGB(255,0,0), 1, CV_AA, 0);
            cv::imshow("ROI", dispimg);
            cv::waitKey(10);

            //connect to decide & reset buttons
            QApplication::processEvents(QEventLoop::AllEvents, 1000);

            //decid or reset
            if(this->killLoopFlag_ == 1){
                break;
            }
            else if(this->killLoopFlag_ == -1){
                OBJECT_SELECTED = false;
                START_POINT_SELECTED = false;
                MOUSE_INSIDE_ROIRECT = false;

                this->killLoopFlag_ = 0;
                //reset
                if(!this->first_roi_selected){
                    SELECTION.x = 0;
                    SELECTION.y = 0;
                    SELECTION.width = 0;
                    SELECTION.height = 0;
                }
                else
                    SELECTION = criteriaroi;
            }

            //for safe
            if(this->killLoopFlag_ == 1)
                break;

        }

        //display
        cv::imshow("ROI", dispimg);
        cv::waitKey(10);
    }


    //--- Check whether all ROIs are selected ----------------------//
    int count = 0;
    int roiNum = (int)this->roi_selected.size();
    for(int i=0; i<roiNum; i++){
        if(this->roi_selected[i])
            count++;
    }

    //break check
    bool continue_checked = false;
    if(count == roiNum){

        //final check
        QMessageBox::StandardButton res = QMessageBox::question(this, "FINAL CHECK", "Will you finish drawing ROIs?", QMessageBox::Yes | QMessageBox::No);
        if(res == QMessageBox::No || res == QMessageBox::Close){
            continue_checked = true;
         }
    }
    else{
        continue_checked = true;
    }


    if(!continue_checked){

        //destroy a window
        cv::destroyWindow("ROI");
        this->window_created = false;

        //disable buttons
        ui->pushButton_moveNextImgD->setEnabled(false);
        ui->pushButton_movePrevImgD->setEnabled(false);
        ui->pushButton_removeImgD->setEnabled(false);
        ui->pushButton_decideROId->setEnabled(false);
        ui->pushButton_drawROId->setEnabled(false);
        ui->pushButton_cancelROId->setEnabled(false);

        //enable a button
        ui->pushButton_exeProgD->setEnabled(true);

        return;
    }
    //continue drawing
    else{

        //disable buttons
        ui->pushButton_decideROId->setEnabled(false);
        ui->pushButton_cancelROId->setEnabled(false);

        //enable buttons
        ui->pushButton_drawROId->setText(tr("描画"));
        ui->pushButton_drawROId->repaint();
        ui->pushButton_drawROId->setEnabled(true);

        ui->pushButton_removeImgD->setEnabled(true);

        if(CURRENT_ROI_ID<=0){
            ui->pushButton_moveNextImgD->setEnabled(true);
            ui->pushButton_movePrevImgD->setEnabled(false);
        }
        else if(CURRENT_ROI_ID>=LIMIT_ROI_ID){
            ui->pushButton_moveNextImgD->setEnabled(false);
            ui->pushButton_movePrevImgD->setEnabled(true);
        }
        else{
            ui->pushButton_moveNextImgD->setEnabled(true);
            ui->pushButton_movePrevImgD->setEnabled(true);
        }


    }

}


//decide ROI
void MainWindow::on_pushButton_decideROId_clicked()
{

    //--- disable buttons ---//
    ui->pushButton_decideROId->setEnabled(false);
    ui->pushButton_cancelROId->setEnabled(false);


    //--- Get a filename of current ROI -----//
    std::string croipath = CURRENT_ROI_PATH;
    std::string::size_type idx = croipath.find_last_of("/");
    std::string::size_type idx2 = croipath.find(".tiff");
    if(idx2 != std::string::npos)
        croipath.erase(idx2, croipath.size());
    if(idx != std::string::npos)
        croipath.erase(0, idx+1);



    //--- Save a ROI -----------------------//
    //set
    roiRect roi;
    roi.imagename = croipath;

    roi.picNo = this->roiIDs[CURRENT_ROI_ID];
    roi.rotated = false;
    roi.roi_rect = cv::Rect(SELECTION.x*RESIZE_TIME, SELECTION.y*RESIZE_TIME, SELECTION.width*RESIZE_TIME, SELECTION.height*RESIZE_TIME);

    //save
    this->ROIRects[CURRENT_ROI_ID] = roi;


    //--- Set flags -----------------------//
    this->roi_selected[CURRENT_ROI_ID] = true;
    if(!this->first_roi_selected){
        this->first_roi_selected = true;
        FIRST_ROI_RECT = roi.roi_rect;
    }


    //--- Show current status -------------//
    int count = 0;
    int roiNum = (int)this->roi_selected.size();
    for(int i=0; i<roiNum; i++){
        if(this->roi_selected[i])
            count++;
    }
    QString status = QString::number(count) + "/" + QString::number(roiNum) + " ROIs selected from " + QString::number((int)this->inputfiles.size()) + " images";
    ui->label_showROInum->setText(status);
    ui->label_showROInum->repaint();

//    //--- Reset the display --------------//
//    SELECTION.x = SELECTION.y = SELECTION.width = SELECTION.height = 0;

}

//cancel ROI
void MainWindow::on_pushButton_cancelROId_clicked()
{

    //--- Disable buttons ----------------//
    ui->pushButton_decideROId->setEnabled(false);
    ui->pushButton_cancelROId->setEnabled(false);

}


//=======================
// Quit detection program
//=======================
void MainWindow::on_pushButton_quitProgD_clicked()
{
    //--- Reset all flags -----------------------------//
    this->window_created = false;
    this->first_roi_selected = false;
    this->auto_detection_canceled = false;
    roi_selected.clear();


    //--- Reset all string variables ------------------//
    this->startscnpath_d.clear();
    this->stopscnpath_d.clear();
    this->inputfiles.clear();
    this->roipathes.clear();

    //--- Reset all roiRect variables -----------------//
    this->ROIRects.clear();
    this->allROIpath.clear();


    //--- Enable buttons ------------------------------//
    ui->pushButton_selectSCNd->setEnabled(true);
    ui->pushButton_selectOUTd->setEnabled(true);
    ui->pushButton_selectStartSCNd->setEnabled(true);
    ui->pushButton_selectStopSCNd->setEnabled(true);
    ui->pushButton_decideInputD->setEnabled(true);


    //--- Disable buttons -----------------------------//
    //radio buttons
    ui->radioButton_selectItv5->setAutoExclusive(false);
    ui->radioButton_selectItv5->setChecked(false);
    ui->radioButton_selectItv5->setAutoExclusive(true);
    ui->radioButton_selectItv5->setEnabled(false);

    ui->radioButton_selectItv10->setAutoExclusive(false);
    ui->radioButton_selectItv10->setChecked(false);
    ui->radioButton_selectItv10->setAutoExclusive(true);
    ui->radioButton_selectItv10->setEnabled(false);

    ui->radioButton_selectItv15->setAutoExclusive(false);
    ui->radioButton_selectItv15->setChecked(false);
    ui->radioButton_selectItv15->setAutoExclusive(true);
    ui->radioButton_selectItv15->setEnabled(false);

    ui->radioButton_selectItv25->setAutoExclusive(false);
    ui->radioButton_selectItv25->setChecked(false);
    ui->radioButton_selectItv25->setAutoExclusive(true);
    ui->radioButton_selectItv25->setEnabled(false);

    //push buttons
    ui->pushButton_moveNextImgD->setEnabled(false);
    ui->pushButton_movePrevImgD->setEnabled(false);
    ui->pushButton_removeImgD->setEnabled(false);
    ui->pushButton_drawROId->setEnabled(false);
    ui->pushButton_decideROId->setEnabled(false);
    ui->pushButton_cancelROId->setEnabled(false);
    ui->pushButton_exeProgD->setEnabled(false);
    ui->pushButton_quitProgD->setEnabled(false);

    ui->pushButton_drawROId->setText(tr("描画"));
    ui->pushButton_drawROId->repaint();


    //--- Reset labels --------------------------------//
    ui->lineEdit_selectStartSCNd->clear();
    ui->lineEdit_selectStartSCNd->repaint();
    ui->lineEdit_selectStopSCNd->clear();
    ui->lineEdit_selectStopSCNd->repaint();

    ui->label_showImageNumD->clear();
    ui->label_showImageNumD->repaint();
    ui->label_showROInum->clear();
    ui->label_showROInum->repaint();

    ui->lineEdit_showROINameD->clear();
    ui->lineEdit_showROINameD->repaint();

    ui->label_reportResultD->clear();
    ui->label_reportResultD->repaint();

    ui->lineEdit_showSkipNameD->clear();
    ui->lineEdit_showSkipNameD->repaint();


    //--- Tab fix --------------------------------------//
    ui->tab_align->setEnabled(true);

}



