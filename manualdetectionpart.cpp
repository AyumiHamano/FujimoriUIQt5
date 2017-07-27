
/******************************
 * Manual detection part
 * ****************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<openslide.h>


/*********************
 * Grobal variables
 * ********************/
int RESIZE_TIME_S = -1;

std::string SKIP_IMAGE_NAME;

cv::Rect SELECTION_S;
cv::Mat DISPIMG_S;

bool START_POINT_SELECTED_S = false;
bool OBJECT_SELECTED_S = false;
bool MOUSE_INSIDE_ROIRECT_S = false;



//=========================
// Select a skipped image
//=========================
void MainWindow::on_pushButton_selectSkipImageS_clicked()
{

    //--- Get an image from the directory ------------------//
    QString SKIP_DIRECTORY_PATH = QString::fromStdString(this->skipfolderpath_d);
    QFileDialog::Options options;
    QString strSelectedFilter;
    QString strFName = QFileDialog::getOpenFileName(
                this,
                tr("select"),
                SKIP_DIRECTORY_PATH,
                tr("*.tiff"),
                &strSelectedFilter, options);



    //--- Load an image and display ------------------------------------//
    if(!strFName.isEmpty()){

        //display the name
        ui->lineEdit_showSkipNameD->setText(strFName);
        ui->lineEdit_showSkipNameD->repaint();

        //get an imagename
        std::string skipimagename = strFName.toStdString();
        std::string::size_type idx = skipimagename.find_last_of("/");
        std::string::size_type idx2 = skipimagename.find(".tiff");
        if(idx2 != std::string::npos)
            skipimagename.erase(idx2, skipimagename.size());
        if(idx != std::string::npos)
            skipimagename.erase(0, idx+1);

        SKIP_IMAGE_NAME = skipimagename;


        //load an image
        std::string skipimagepath = this->skipfolderpath_d + "/" + skipimagename + ".tiff";
        cv::Mat img = cv::imread(skipimagepath, 1);
        if(img.empty()){
            QMessageBox::information(this, "CAUTION", "This image is not available:" + QString::fromStdString(skipimagepath));

            //reset display
            ui->lineEdit_showSkipNameD->clear();
            ui->lineEdit_showSkipNameD->repaint();

            return;
        }

        //create a window
        if(!this->window_created){
            cv::namedWindow("ROI", 1);
            this->window_created = true;
        }

        //resize
        if(img.cols > this->limitW || img.rows> this->limitH){

            //if(RESIZE_TIME_S == -1){
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

                RESIZE_TIME_S = (int)factorx;
           // }

            cv::resize(img, DISPIMG_S, cv::Size(img.cols/RESIZE_TIME_S, img.rows/RESIZE_TIME_S));

        }
        else{
            DISPIMG_S = img.clone();
            //if(RESIZE_TIME_S == -1){
                RESIZE_TIME_S = 1;
            //}
        }

        //display image
        cv::imshow("ROI", DISPIMG_S);
        cv::waitKey(10);

        //enable a drawing button
        ui->pushButton_drawROIs->setEnabled(true);

    }



}



//========================
// Draw a ROI
//========================
//callback function
void on_mouse_function_s(int event, int x, int y, int flags, void* param){

    cv::Rect *inputrect = static_cast<cv::Rect*>(param);

    //set a first point
    int minx = inputrect->x + inputrect->width/2 - 5;
    int maxx = inputrect->x + inputrect->width/2 + 5;
    int miny = inputrect->y + inputrect->height/2 - 5;
    int maxy = inputrect->y + inputrect->height/2 + 5;


    //--- Process a Start and a Finish Selecting Events (i.e. button-up and -down) ---//
    switch (event){
    case cv::EVENT_MOUSEMOVE:
        if(x > minx && x < maxx && y > miny && y < maxy && !MOUSE_INSIDE_ROIRECT_S){
            MOUSE_INSIDE_ROIRECT_S = true;
        }

        if(!OBJECT_SELECTED_S && MOUSE_INSIDE_ROIRECT_S){
            SELECTION_S = cv::Rect(x - inputrect->width / 2, y - inputrect->height / 2, inputrect->width, inputrect->height);
        }
        else if(!OBJECT_SELECTED_S && !MOUSE_INSIDE_ROIRECT_S){
            SELECTION_S = *inputrect;
        }

        break;
    case cv::EVENT_LBUTTONDOWN:
        OBJECT_SELECTED_S = true;
        MOUSE_INSIDE_ROIRECT_S = false;
        break;
    }
}


//draw
void MainWindow::on_pushButton_drawROIs_clicked()
{

    //--- Disable a button ----------------------------------//
    ui->pushButton_selectSkipImageS->setEnabled(false);
    ui->pushButton_quitSkipImageS->setEnabled(false);


    //--- Load an image -------------------------------------//
    std::string skipimagepath = this->skipfolderpath_d + "/" + SKIP_IMAGE_NAME + ".tiff";
    cv::Mat img = cv::imread(skipimagepath, 1);
    if(img.empty()){
        QMessageBox::information(this, "CAUTION", "This image is not available: " + QString::fromStdString(skipimagepath));
        return;
    }

    //resize
    cv::Mat resizeimg;
    if(img.cols > this->limitW || img.rows > this->limitH){
        cv::resize(img, resizeimg, cv::Size(img.cols / RESIZE_TIME_S, img.rows / RESIZE_TIME_S));
    }
    else{
        resizeimg = img.clone();
    }


    //--- Set a current ROI ----------------------------------//
    cv::Rect criteriaroi = this->allROIpath[0].roi_rect;

    //resize
    criteriaroi = cv::Rect(criteriaroi.x / (RESIZE_TIME_S*4), criteriaroi.y / (RESIZE_TIME_S*4), criteriaroi.width / (RESIZE_TIME_S*4), criteriaroi.height / (RESIZE_TIME_S*4));

    //adjust
    if(criteriaroi.x > resizeimg.cols)  criteriaroi.x = 0;
    if(criteriaroi.y > resizeimg.rows)  criteriaroi.y = 0;



    //--- Set a mouse callback function -----------------------//
    //initializatiion
    OBJECT_SELECTED_S = false;
    MOUSE_INSIDE_ROIRECT_S = false;
    START_POINT_SELECTED_S = false;

    SELECTION_S.x = criteriaroi.x;
    SELECTION_S.y = criteriaroi.y;
    SELECTION_S.width = criteriaroi.width;
    SELECTION_S.height = criteriaroi.height;

    //call
    cv::setMouseCallback("ROI", on_mouse_function_s, &criteriaroi);


    //=== LOOP ===//

    this->killLoopFlag_ = 0;

    //disable buttons
    ui->pushButton_decideROIs->setEnabled(false);
    ui->pushButton_cancelROIs->setEnabled(false);

    while(1){

        //display
        cv::Mat dispimg = resizeimg.clone();

        //adjust coordinate
        if(SELECTION_S.x < 0)   SELECTION_S.x = 0;
        else if(SELECTION_S.x + SELECTION_S.width > dispimg.cols)   SELECTION_S.x = dispimg.cols - SELECTION_S.width;
        if(SELECTION_S.y < 0)   SELECTION_S.y = 0;
        else if(SELECTION_S.y + SELECTION_S.height > dispimg.rows)  SELECTION_S.y = dispimg.rows - SELECTION_S.height;

        //drawing
        if(!OBJECT_SELECTED_S && SELECTION_S.width>0 && SELECTION_S.height > 0){
            cv::rectangle(dispimg, SELECTION_S, CV_RGB(0,0,255), 1, CV_AA, 0);
            cv::rectangle(dispimg, cv::Point(SELECTION_S.x + SELECTION_S.width/2 - SELECTION_S.width/10, SELECTION_S.y + SELECTION_S.height/2 - SELECTION_S.height/10),
                          cv::Point(SELECTION_S.x + SELECTION_S.width/2 + SELECTION_S.width/10, SELECTION_S.y + SELECTION_S.height/2 + SELECTION_S.height/10),
                          CV_RGB(0,255,0), 1, CV_AA, 0);
        }
        //drew
        else if(OBJECT_SELECTED_S){

            //enable buttons
            ui->pushButton_decideROIs->setEnabled(true);
            ui->pushButton_cancelROIs->setEnabled(true);

            //draw a selected rectangle & display
            cv::rectangle(dispimg, SELECTION_S, CV_RGB(255,0,0), 1, CV_AA, 0);
            cv::imshow("ROI", dispimg);
            cv::waitKey(10);

            QApplication::processEvents(QEventLoop::AllEvents, 1000);

            //decid or reset
            if(this->killLoopFlag_ == 1){
                break;
            }
            else if(this->killLoopFlag_ == -1){
                OBJECT_SELECTED_S = false;
                START_POINT_SELECTED_S = false;
                MOUSE_INSIDE_ROIRECT_S = false;

                this->killLoopFlag_ = 0;
                SELECTION_S = criteriaroi;
            }

            //for safe
            if(this->killLoopFlag_ == 1)
                break;

        }

        //display
        cv::imshow("ROI", dispimg);
        cv::waitKey(10);
    }

    //--- Destroy a window ------------------------------------------//
    cv::destroyWindow("ROI");
    this->window_created = false;


    //--- Disable buttons ------------------------------------------//
    ui->pushButton_decideROIs->setEnabled(false);
    ui->pushButton_cancelROIs->setEnabled(false);


    //--- Enable buttons -------------------------------------------//
    ui->pushButton_selectSkipImageS->setEnabled(true);
    ui->pushButton_quitSkipImageS->setEnabled(true);


}


//decide
void MainWindow::on_pushButton_decideROIs_clicked()
{
    //--- Disable buttons -------------------------------//
    ui->pushButton_decideROIs->setEnabled(false);
    ui->pushButton_cancelROIs->setEnabled(false);


    //--- Find current skipped image from skipped roiRect data -----------------//
    int targetID = -1;
    for(int i=0; i<(int)this->skippedRects.size(); i++){
        //get a filename
        std::string filename = this->skippedRects[i].imagename;
        //check
        if(filename == SKIP_IMAGE_NAME){
            targetID = i;
            break;
        }
    }

    //check the ID
    if(targetID == -1){
        QMessageBox::information(this, "CAUTION", "This image is not available:" + QString::fromStdString(SKIP_IMAGE_NAME));
        return;
    }


    //--- Get width and height of the current skipped image ---//
    std::string scnpath = this->scnfolderpath_d + "/" + SKIP_IMAGE_NAME + ".scn";
    openslide_t *obj = openslide_open(scnpath.c_str());
    int64_t width, height;
    if(!obj == NULL){
        openslide_get_layer_dimensions(obj, 3, &width, &height);
    }
    else{
        width = height = 0;
    }

    int dx = (this->campusW - width) / 2;
    int dy = (this->campusH - height) / 2;


    //--- Set ROI information -----------------------------------//
    roiRect roi = this->skippedRects[targetID];
    roi.roi_rect = cv::Rect((SELECTION_S.x*RESIZE_TIME_S-dx)*4, (SELECTION_S.y*RESIZE_TIME_S-dy)*4, SELECTION_S.width*RESIZE_TIME_S*4, SELECTION_S.height*RESIZE_TIME_S*4);



    //--- Save the ROi image ------------------------------------//
    scnpath = this->scnfolderpath_d + "/" + roi.imagename + ".scn";
    cv::Mat img;
    bool img_checked = showROI(scnpath, img, 2, roi.roi_rect.x, roi.roi_rect.y, roi.roi_rect.width, roi.roi_rect.height);
    if(img_checked){
        std::string savepath = this->resultfolderpath_d + "/" + roi.imagename + ".tiff";
        cv::imwrite(savepath, img);
    }


    //--- Insert ROI information into allROIpath ----------------//
    //initialization
    const int insert_imageID = roi.picNo;
    bool insert_checked = false;
    std::vector<roiRect> tmprects;

    for(int i=0; i<(int)this->allROIpath.size(); i++){
        //get an imageID
        int imageID = this->allROIpath[i].picNo;

        //check
        if(imageID > insert_imageID && !insert_checked){
            tmprects.push_back(roi);    //insert skipped ROI information
            insert_checked = true;
        }

        //save
        tmprects.push_back(this->allROIpath[i]);
    }

    if(!insert_checked){
        tmprects.push_back(roi);
    }



    //--- Reset allROIpath ----------------------------------------//
    this->allROIpath.clear();
    for(int i=0; i<(int)tmprects.size(); i++){
        this->allROIpath.push_back(tmprects[i]);
    }


    //--- Remove current image from skipfolder --------------------//
    std::string skipimagepath = this->skipfolderpath_d + "/" + roi.imagename + ".tiff";
    std::remove(skipimagepath.c_str());

}


//cancel
void MainWindow::on_pushButton_cancelROIs_clicked()
{
    //--- Disable buttons ----------------------------//
    ui->pushButton_decideROIs->setEnabled(false);
    ui->pushButton_cancelROIs->setEnabled(false);

}

//quit
void MainWindow::on_pushButton_quitSkipImageS_clicked()
{
    //--- Disable buttons ----------------------------------//
    ui->pushButton_selectSkipImageS->setEnabled(false);
    ui->pushButton_quitSkipImageS->setEnabled(false);
    ui->pushButton_drawROIs->setEnabled(false);
    ui->pushButton_decideROIs->setEnabled(false);
    ui->pushButton_cancelROIs->setEnabled(false);


    //--- Save ROI path information ------------------------//
    //open a file
    std::string infpath = this->inffolderpath + "/roiInf_level2.tsv";
    std::ofstream ofs(infpath, ios::out);
    if(ofs.fail()){
        QMessageBox::information(this, "CAUTION", "file open error in doElastix :" + QString::fromStdString(infpath));
        ui->pushButton_quitProgD->setEnabled(true);
        return;
    }

    //write information
    const int allpathNum = (int)this->allROIpath.size();
    for(int no = 0; no<allpathNum; no++){

        //get an object
        roiRect obj = this->allROIpath[no];

        //output to a file
        ofs << obj.picNo << "\t" << obj.imagename << "\t" << obj.roi_rect.x << "\t" << obj.roi_rect.y << "\t" << obj.roi_rect.width << "\t" << obj.roi_rect.height << "\t" << obj.rotated << std::endl;

    }

    //file close
    ofs.close();


    //--- Enable a button ----------------------------------//
    ui->pushButton_quitProgD->setEnabled(true);

}

