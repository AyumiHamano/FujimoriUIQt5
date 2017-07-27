
/*************************
 * Alignment part
 * **********************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<openslide.h>

//================
// Grobal variables
//==================
int RESIZE_TIME_A = -1;

cv::Rect SELECTION_A;
cv::Mat DISPIMG_A;

bool START_POINT_SELECTED_A = false;
bool OBJECT_SELECTED_A = false;
bool MOUSE_INSIDE_ROIRECT_A = false;


//====================-
// Tab check
//====================
void MainWindow::tabCheck(int INDEX)
{
    //--- Check the index ---//
    if(INDEX != 1)
        return;

    //--- Show the status ----------------//
    if(!this->scnfolderpath_d.empty() && !this->outfolderpath_d.empty() && !this->inffolderpath.empty() ){
        //show the inffolder
        this->inffolderpath_a = this->inffolderpath;
        ui->lineEdit_selectINFa->setText(QString::fromStdString(this->inffolderpath_a));
        ui->lineEdit_selectINFa->repaint();
        this->inffolder_a_selected = true;

        //show the scn folder
        this->scnfolderpath_a = this->scnfolderpath_d;
        ui->lineEdit_selectSCNa->setText(QString::fromStdString(this->scnfolderpath_a));
        ui->lineEdit_selectSCNa->repaint();
        this->scnfolder_a_selected = true;

        //show the output folder
        this->outfolderpath_a = this->outfolderpath_d;
        ui->lineEdit_selectOUTa->setText(QString::fromStdString(this->outfolderpath_a));
        ui->lineEdit_selectOUTa->repaint();
        this->outfolder_a_selected = true;

    }

    //--- Reset flags ----------------------//
    this->level_a_selected = false;
    this->adjust_roi_selected = false;

    //enable
    ui->pushButton_selectINFa->setEnabled(true);
    ui->pushButton_selectSCNa->setEnabled(true);
    ui->pushButton_selectOUTa->setEnabled(true);
    ui->pushButton_decideInputA->setEnabled(true);

    ui->radioButton_level0->setEnabled(true);
    ui->radioButton_level1->setEnabled(true);
    ui->radioButton_level2->setEnabled(true);
}



//==========================
// Select folders
//===========================
//inf folder
void MainWindow::on_pushButton_selectINFa_clicked()
{
    //--- Tab fixed ---------------------------//
    if(ui->tab_detect->isEnabled()){
        ui->tab_detect->setEnabled(false);
    }


    //--- Search the folder ------------------//
    QString defaultfolderpath;
    if(!this->inffolderpath_a.empty()){
        defaultfolderpath = QString::fromStdString(this->inffolderpath_a);
    }
    else{
        defaultfolderpath = "C://";
    }

    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString strDir = QFileDialog::getExistingDirectory(this, tr("search"), defaultfolderpath, options);
    if(!strDir.isEmpty()){
        //display the path
        ui->lineEdit_selectINFa->setText(strDir);
        ui->lineEdit_selectINFa->repaint();
        //save the path
        this->inffolderpath_a = strDir.toStdString();
        //set the flag
        this->inffolder_a_selected = true;
    }
    else{
        //check other flag
        if(!this->outfolder_a_selected && !this->scnfolder_a_selected && !this->inffolder_a_selected)
            ui->tab_detect->setEnabled(true);

        return;
    }


    //--- Get all filenames which have ".tsv" -------------//
    QDir dir(strDir);
    QStringList strlFilter;
    strlFilter << "*.tsv";
    QFileInfoList list = dir.entryInfoList(strlFilter, QDir::Files);

    //Error check
    if(list.isEmpty()){
        QMessageBox::information(this, "CAUTION", "No .tsv files available");
        ui->lineEdit_selectINFa->clear();
        ui->lineEdit_selectINFa->repaint();
        this->inffolder_a_selected = false;
        return;
    }

}


//scn folder
void MainWindow::on_pushButton_selectSCNa_clicked()
{
    //--- Tab fixed ---------------------------//
    if(ui->tab_detect->isEnabled()){
        ui->tab_detect->setEnabled(false);
    }


    //--- Search the folder ------------------//
    QString defaultfolderpath;
    if(!this->scnfolderpath_a.empty()){
        defaultfolderpath = QString::fromStdString(this->scnfolderpath_a);
    }
    else{
        defaultfolderpath = "C://";
    }

    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString strDir = QFileDialog::getExistingDirectory(this, tr("search"), defaultfolderpath, options);
    if(!strDir.isEmpty()){
        //display the path
        ui->lineEdit_selectSCNa->setText(strDir);
        ui->lineEdit_selectSCNa->repaint();
        //save the path
        this->scnfolderpath_a = strDir.toStdString();
        //set the flag
        this->scnfolder_a_selected = true;
    }
    else{
        //check other flag
        if(!this->outfolder_a_selected && !this->scnfolder_a_selected && !this->inffolder_a_selected)
            ui->tab_detect->setEnabled(true);

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
       ui->lineEdit_selectSCNa->clear();
       ui->lineEdit_selectSCNa->repaint();
       this->scnfolderpath_a.clear();
       this->scnfolder_a_selected = false;

       return;

    }
}


//output folder
void MainWindow::on_pushButton_selectOUTa_clicked()
{
    //--- Tab fixed ---------------------------//
    if(ui->tab_detect->isEnabled()){
        ui->tab_detect->setEnabled(false);
    }


    //--- Search the folder ------------------//
    QString defaultfolderpath;
    if(!this->outfolderpath_a.empty()){
        defaultfolderpath = QString::fromStdString(this->outfolderpath_a);
    }
    else{
        defaultfolderpath = "C://";
    }

    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString strDir = QFileDialog::getExistingDirectory(this, tr("search"), defaultfolderpath, options);
    if(!strDir.isEmpty()){
        //display the path
        ui->lineEdit_selectOUTa->setText(strDir);
        ui->lineEdit_selectOUTa->repaint();
        //save the path
        this->outfolderpath_a = strDir.toStdString();
        //set the flag
        this->outfolder_a_selected = true;
    }
    else{
        //check other flag
        if(!this->outfolder_a_selected && !this->scnfolder_a_selected && !this->inffolder_a_selected)
            ui->tab_detect->setEnabled(true);

        return;
    }

}



//==================================
// Select level
//===================================
//level2
void MainWindow::on_radioButton_level2_clicked()
{
    //tab fix
    ui->tab_detect->setEnabled(false);


    //check
    std::string infpath = this->inffolderpath_a + "/roiInf_level2.tsv";
    std::ifstream ifs(infpath.c_str());
    if(ifs.fail()){
        QMessageBox::information(this, "CAUTION", "Auto detection must be executed before this process");

        //reset
        ui->radioButton_level2->setAutoExclusive(false);
        ui->radioButton_level2->setChecked(false);
        ui->radioButton_level2->setAutoExclusive(true);

        return;
    }

    //Set current level
    this->adjust_target_level = 2;
    this->level_a_selected = true;
}


//level1
void MainWindow::on_radioButton_level1_clicked()
{
    //tab fix
    ui->tab_detect->setEnabled(false);

    //initialization
    this->level_a_selected = false;

    //--- Level2 must be done before this level -----------------------//
    std::string infpath_level2 = this->resultfolderpath_a + "/inf/roiInf_level1_slice.tsv";
    std::string infpath_level2_2 = this->inffolderpath_a + "/roiInf_level1_slice.tsv";
    std::ifstream ifs(infpath_level2);
    std::ifstream ifs2(infpath_level2_2);

    //check
    if(ifs.fail() && ifs2.fail()){
        QMessageBox::information(this, "CAUTION", "low must be alignmented before this process");

        //reset
        ui->radioButton_level1->setAutoExclusive(false);
        ui->radioButton_level1->setChecked(false);
        ui->radioButton_level1->setAutoExclusive(true);

        return;
    }
    else if(!ifs.fail()){
        this->sliceinfpath_a = infpath_level2;
    }
    else{
        this->sliceinfpath_a = infpath_level2_2;
    }


    //--- Set flags ----------------------------------------//
    this->adjust_target_level = 1;
    this->level_a_selected = true;

}


//level0
void MainWindow::on_radioButton_level0_clicked()
{
    //tab fix
    ui->tab_detect->setEnabled(false);

    //initialization
    this->level_a_selected = false;

    //--- Level1 must be done before this level -----------------------//
    std::string infpath_level1 = this->resultfolderpath_a + "/inf/roiInf_level0_slice.tsv";
    std::string infpath_level1_2 = this->inffolderpath_a + "/roiInf_level0_slice.tsv";
    std::ifstream ifs(infpath_level1);
    std::ifstream ifs2(infpath_level1_2);

    //check
    if(ifs.fail() && ifs2.fail()){
        QMessageBox::information(this, "CAUTION", "middle must be alignmented before this process");

        //reset
        ui->radioButton_level0->setAutoExclusive(false);
        ui->radioButton_level0->setChecked(false);
        ui->radioButton_level0->setAutoExclusive(true);

        return;
    }
    else if(!ifs.fail()){
        this->sliceinfpath_a = infpath_level1;
    }
    else{
        this->sliceinfpath_a = infpath_level1_2;
    }


    //--- Set flags ----------------------------------------//
    this->adjust_target_level = 0;
    this->level_a_selected = true;
}




//================================-
// Decide input
//==================================
void MainWindow::enableButtons()
{
    ui->pushButton_selectINFa->setEnabled(true);
    ui->pushButton_selectOUTa->setEnabled(true);
    ui->pushButton_selectSCNa->setEnabled(true);
    ui->pushButton_decideInputA->setEnabled(true);

    ui->radioButton_level0->setEnabled(true);
    ui->radioButton_level1->setEnabled(true);
    ui->radioButton_level2->setEnabled(true);
}


void MainWindow::on_pushButton_decideInputA_clicked()
{
    //---フォルダの選択状況をチェック  ---------------------------------------------//
    if(!this->inffolder_a_selected){
        QMessageBox::information(this, "caution", "Please set the INF directory");
        return;
    }
    if(!this->scnfolder_a_selected){
        QMessageBox::information(this, "caution", "Please set the SCN directory");
        return;
    }
    if(!this->outfolder_a_selected){
        QMessageBox::information(this, "caution", "Please set the OUTPUT directory");
        return;
    }
    if(!this->level_a_selected){
        QMessageBox::information(this, "caution", "Please select the resolution level");
        return;
    }



    //--- 各解像度に合わせた初期設定 -----------------------------//
    if(this->inffolder_a_selected && this->outfolder_a_selected && this->scnfolder_a_selected && this->level_a_selected){

        //各種コンテナの無効化
        ui->pushButton_selectINFa->setEnabled(false);
        ui->pushButton_selectSCNa->setEnabled(false);
        ui->pushButton_selectOUTa->setEnabled(false);
        ui->pushButton_decideInputA->setEnabled(false);

        ui->radioButton_level0->setEnabled(false);
        ui->radioButton_level1->setEnabled(false);
        ui->radioButton_level2->setEnabled(false);

        //結果出力フォルダの作成
        this->resultfolderpath_a = this->outfolderpath_a + "/RESULT_ALIGN";
        _mkdir(this->resultfolderpath_a.c_str());

        //作業用フォルダの作成
        this->workfolderpath_a = this->outfolderpath_a + "/work_align";
        _mkdir(this->workfolderpath_a.c_str());

        //INFファイル保存用フォルダの作成
        this->infresultfolderpath_a = this->resultfolderpath_a + "/inf";
        _mkdir(this->infresultfolderpath_a.c_str());

        //ROI操作用変数の初期化
        this->align_roi_id = 0;
        this->adjust_roi_selected = false;


        //=== 解像度別の処理 ===//
        //level2
        if(this->adjust_target_level == 2){

            //ROI情報の読み込み
            this->adjustROIRects.clear();
            std::string infpath = this->inffolderpath_a + "/roiInf_level2.tsv";
            bool inf_checked = getROIinf(infpath, this->adjustROIRects);
            if(!inf_checked){
                QMessageBox::information(this, "ERROR", "We can't open " + QString::fromStdString(infpath) + "\n Please select a correct folder path");
                //enable buttons
                this->enableButtons();
                return;

            }

            //level2結果保存用フォルダの作成
            this->level_resultfolderpath_a = this->resultfolderpath_a + "/low";
            bool check = createFolder(this->level_resultfolderpath_a);

            //作業用フォルダにROI読み込み用フォルダを作成
            this->roifolderpath_a = this->workfolderpath_a + "/low";
            check = createFolder(this->roifolderpath_a);

            //ROI読み込み用フォルダ以下に切片画像読み込み用フォルダを作成
            this->slicefolderpath_a = this->roifolderpath_a + "/slice";
            check = createFolder(this->slicefolderpath_a);

            //進行状況ダイアログの初期化
            this->dialog->reset();
            this->dialog->setMinimum(0);
            this->dialog->setMaximum((int)this->adjustROIRects.size()-1);
            this->dialog->setLabelText("Input image loading");
            this->dialog->repaint();
            this->dialog->show();
            qApp->processEvents();

            //ROIと切片画像を読み込み
            int count = 0;
            int count_slice = 0;
            int maxW, maxH;
            maxW = maxH = 0;

            for(int i=0; i<(int)this->adjustROIRects.size(); i++){

                //ダイアログ表示
                this->dialog->setValue(i);
                qApp->processEvents();

                //画像名の取得
                std::string imagename = this->adjustROIRects[i].imagename;

                //ROIの矩形情報を取得
                int x = this->adjustROIRects[i].roi_rect.x;
                int y = this->adjustROIRects[i].roi_rect.y;
                int width = this->adjustROIRects[i].roi_rect.width;
                int height = this->adjustROIRects[i].roi_rect.height;

                //切片画像のオリジナル情報を持つSCNファイルへのパスを作成
                std::string scnpath = this->scnfolderpath_a + "/" + imagename + ".scn";

                //SCNファイルよりROI像をレベル２の解像度、切片画像をレベル３の解像度で読み込み
                //※ROI修正用にレベル３で切片画像全体を読み込んでいる
                cv::Mat img, sliceimg;
                bool img_checked = showROI(scnpath, img, 2, x, y, width, height);
                bool slice_checked = ConvertSCN2cvMat(scnpath, sliceimg, 3);

                //読み込めた場合、読み込んだROIを保存
                if(img_checked){
                    //save
                    std::string savepath = this->roifolderpath_a + "/" + imagename + ".tiff";
                    cv::imwrite(savepath, img);
                    count++;
                }
                //読み込めた場合、切片画像を保存
                if(slice_checked){
                    roiRect sliceroi;
                    sliceroi.imagename = imagename;
                    this->sliceROIRects.push_back(sliceroi);

                    //save
                    std::string savepath = this->slicefolderpath_a + "/" + imagename + ".tiff";
                    cv::imwrite(savepath, sliceimg);

                    //切片画像の幅高さの最大値を取得
                    if(sliceimg.cols > maxW)
                        maxW = sliceimg.cols;
                    if(sliceimg.rows > maxH)
                        maxH = sliceimg.rows;

                    count_slice++;
                }

            }

            //切り出してきたROIをすべて読み込めたかチェック
            if(count != (int)this->adjustROIRects.size()){
                QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! We can't load all input images");
                exit(-1);
            }

            if(count_slice != (int)this->adjustROIRects.size()){
                QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! We can't load all slice images");
                exit(-1);
            }

            //--- 画像サイズを統一するため、切片画像の幅高さの最大値を用いてキャンバスを作成し、その上に切片画像を載せる ------------//
            this->campusW = maxW + this->extraW;
            this->campusH = maxH + this->extraH;
            cv::Mat campus = cv::Mat::zeros(cv::Size(campusW, campusH), CV_8UC3);
            for(int i=0; i<(int)this->sliceROIRects.size(); i++){
                //切片画像のファイルパスを取得
                std::string filename = this->sliceROIRects[i].imagename;
                std::string imagepath = this->slicefolderpath_a + "/" + filename + ".tiff";

                //画像読み込み
                cv::Mat img = cv::imread(imagepath, 1);

                //キャンバス上に画像貼り付け位置を設定
                int X = (this->campusW - (int)img.cols) / 2;
                int Y = (this->campusH - (int)img.rows) / 2;
                cv::Mat roi = campus(cv::Rect(X, Y, (int)img.cols, (int)img.rows));

                //キャンバスに貼り付け
                img.copyTo(roi);

                //キャンバスを保存
                cv::imwrite(imagepath, campus);

                //キャンバスを初期化
                campus.setTo(0);

            }

        }
        //level1 and level0
        else if(this->adjust_target_level == 1){

            //ROI情報を読み込み
            this->sliceROIRects.clear();
            bool inf_checked = getROIinf(this->sliceinfpath_a, this->sliceROIRects, this->past_level1_matricies, this->past_level0_matricies);
            if(!inf_checked){
                QMessageBox::information(this, "ERROR", "We can't open " + QString::fromStdString(this->sliceinfpath_a) + "\n Please select a correct folder path");
                //enable buttons
                this->enableButtons();
                return;
            }

            //結果保存用フォルダの作成
            this->level_resultfolderpath_a = this->resultfolderpath_a + "/middle";
            this->roifolderpath_a = this->workfolderpath_a + "/middle";
            bool check = createFolder(this->level_resultfolderpath_a);
            check = createFolder(this->roifolderpath_a);

            //本来のROI位置より数回り大きい作業用画像（作業用ROI）の保存用フォルダの作成
            this->slicefolderpath_a = this->roifolderpath_a + "/slice";
            check = createFolder(this->slicefolderpath_a);

            //進行状況ダイアログの初期化
            this->dialog->reset();
            this->dialog->setMinimum(0);
            this->dialog->setMaximum((int)this->sliceROIRects.size()-1);
            this->dialog->setLabelText("Input image loading");
            this->dialog->repaint();
            this->dialog->show();
            qApp->processEvents();

            //画像の読み込み
            int count = 0;
            for(int i=0; i<(int)this->sliceROIRects.size(); i++){

                //ダイアログの表示
                this->dialog->setValue(i);
                qApp->processEvents();

                //画像名を取得
                std::string imagename = this->sliceROIRects[i].imagename;

                //作業用ROIの矩形情報を取得
                int x = this->sliceROIRects[i].roi_rect.x;
                int y = this->sliceROIRects[i].roi_rect.y;
                int width = this->sliceROIRects[i].roi_rect.width;
                int height = this->sliceROIRects[i].roi_rect.height;
                cv::Mat matrix = this->past_level1_matricies[i];

                //SCNファイルパスを作成
                std::string scnpath = this->scnfolderpath_a + "/" + imagename + ".scn";

                //処理中の解像度で作業用ROI画像を読み込み
                cv::Mat img;
                bool img_checked = showROI(scnpath, img, this->adjust_target_level, x, y, width, height, matrix);

                //逆行列を保存
                cv::Mat invmat = matrix.inv();
                this->past_level1_matricies_inv.push_back(invmat);


                //読み込めた場合、保存
                if(img_checked){
                    //元のSCN画像が上下反転している場合、この時点で正しい向きに直して保存
                    if(this->sliceROIRects[i].rotated){
                        cv::Mat tmpimg = img.clone();
                        img.release();
                        cv::flip(tmpimg, img, -1);
                        tmpimg.release();
                    }
                    std::string savepath = this->slicefolderpath_a + "/" + imagename + ".tiff";
                    cv::imwrite(savepath, img);
                    count++;
                }
            }

            //読み込んだ画像の枚数をチェック
            if(count != (int)this->sliceROIRects.size()){
                QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! We can't load all input images");
                exit(-1);
            }

        }
        //level0
        else{

            //ROI情報を読み込み
            this->sliceROIRects.clear();
            this->uppersliceROIRects.clear();
            bool inf_checked = getROIinf(this->sliceinfpath_a, this->sliceROIRects,this->uppersliceROIRects, this->past_matricies);
            if(!inf_checked){
                QMessageBox::information(this, "ERROR", "We can't open " + QString::fromStdString(this->sliceinfpath_a) + "\n Please select a correct folder path");
                //enable buttons
                this->enableButtons();
                return;
            }

            //結果保存用フォルダの作成
            this->level_resultfolderpath_a = this->resultfolderpath_a + "/high";
            this->roifolderpath_a = this->workfolderpath_a + "/high";
            bool check = createFolder(this->level_resultfolderpath_a);
            check = createFolder(this->roifolderpath_a);

            //本来のROI位置より数回り大きい作業用画像（作業用ROI）の保存用フォルダの作成
            this->slicefolderpath_a = this->roifolderpath_a + "/slice";
            check = createFolder(this->slicefolderpath_a);

            //進行状況ダイアログの初期化
            this->dialog->reset();
            this->dialog->setMinimum(0);
            this->dialog->setMaximum((int)this->sliceROIRects.size()-1);
            this->dialog->setLabelText("Input image loading");
            this->dialog->repaint();
            this->dialog->show();
            qApp->processEvents();

            //画像の読み込み
            int count = 0;
            for(int i=0; i<(int)this->sliceROIRects.size(); i++){

                //ダイアログの表示
                this->dialog->setValue(i);
                qApp->processEvents();

                //画像名を取得
                std::string imagename = this->sliceROIRects[i].imagename;

                //作業用ROIの矩形情報を取得
//                int x = this->sliceROIRects[i].roi_rect.x;
//                int y = this->sliceROIRects[i].roi_rect.y;
//                int width = this->sliceROIRects[i].roi_rect.width;
//                int height = this->sliceROIRects[i].roi_rect.height;

//                int ux = this->uppersliceROIRects[i].roi_rect.x;
//                int uy = this->uppersliceROIRects[i].roi_rect.y;
//                int uwidth = this->sliceROIRects[i].roi_rect.width;
//                int uheight = this->sliceROIRects[i].roi_rect.height;

                cv::Mat matrix = this->past_matricies[i];

                //SCNファイルパスを作成
                std::string scnpath = this->scnfolderpath_a + "/" + imagename + ".scn";

                //処理中の解像度で作業用ROI画像を読み込み
                cv::Mat img;
                bool img_checked = showROI(scnpath, img, this->adjust_target_level, this->sliceROIRects[i], this->uppersliceROIRects[i], matrix);

                //逆行列を保存
                cv::Mat invmat = matrix.inv();
                this->past_matricies_inv.push_back(invmat);


                //読み込めた場合、保存
                if(img_checked){
                    //元のSCN画像が上下反転している場合、この時点で正しい向きに直して保存
                    if(this->sliceROIRects[i].rotated){
                        cv::Mat tmpimg = img.clone();
                        img.release();
                        cv::flip(tmpimg, img, -1);
                        tmpimg.release();
                    }
                    std::string savepath = this->slicefolderpath_a + "/" + imagename + ".tiff";
                    cv::imwrite(savepath, img);
                    count++;
                }
            }

            //読み込んだ画像の枚数をチェック
            if(count != (int)this->sliceROIRects.size()){
                QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! We can't load all input images");
                exit(-1);
            }

        }


        //--- 次の処理のためのボタンを有効 ---------------------------------------------//
        ui->pushButton_selectROIa->setEnabled(true);
    }

}



//=============================
//　ROIの設定
//============================
//ROIを切り出す対象となる画像を設定
void MainWindow::on_pushButton_selectROIa_clicked()
{
    //--- ROIを切り出す画像名を取得 ---------------------------//
    //画像が保存されているフォルダのパスを作成
    std::string imagefolderpath;
    if(this->adjust_target_level == 2)
        imagefolderpath = this->roifolderpath_a;
    else
        imagefolderpath = this->slicefolderpath_a;

    //画像名を取得
    QFileDialog::Options options;
    QString strSelectedFilter;
    QString strFname = QFileDialog::getOpenFileName(
                this,
                tr("select"),
                QString::fromStdString(imagefolderpath),
                tr("*.tiff"),
                &strSelectedFilter, options );


    //---　選択した画像名の表示とフラグ設定 -------------//
    if(!strFname.isEmpty()){
        //画像名を表示
        ui->lineEdit_showROINameA->setText(strFname);
        ui->lineEdit_showROINameA->repaint();

        //ROIを切り出す対象画像として画像名のみ保存
        std::string imagepath = strFname.toStdString();
        std::string::size_type idx = imagepath.find_last_of("/");
        std::string::size_type idx2 = imagepath.find(".");
        if(idx2 != std::string::npos)
            imagepath.erase(idx2, imagepath.size());
        if(idx != std::string::npos)
           imagepath.erase(0, idx+1);
        this->roiimagename_a = imagepath;

        //対象画像の配列中でのインデックスを取得
        std::vector<roiRect> tmprects;
        if(this->adjust_target_level == 2){
            tmprects = adjustROIRects;
        }
        else{
            tmprects = sliceROIRects;
        }

        this->align_roi_id = -1;
        for(int i=0; i<(int)tmprects.size(); i++){
            //画像名を取得
            std::string filename = tmprects[i].imagename;

            //対象画像と一致すれば、インデックスを保存してブレイク
            if(filename == this->roiimagename_a){
                this->align_roi_id = i;
                break;
            }
        }

        //画像が存在しない場合、強制終了
        if(this->align_roi_id == -1){
            QMessageBox::information(this, "CAUTION", "Alignment ROI ID is -1");
            exit(-1);
        }

        //解像度別にROI編集のための初期設定
        //解像度２の場合
        if(this->adjust_target_level == 2){
            //アライメントの基準となるROIが選択されたことをフラグで設定
            this->adjust_roi_selected = true;

            //編集用ボタンの有効か
            ui->pushButton_selectAdjImageA->setEnabled(true);
            ui->pushButton_quitAdjustA->setEnabled(true);

            //表示用に画像サイズを変更する必要がないため、他の処理と分けるために-1を設定
            RESIZE_TIME_A = -1;

        }
        //解像度１または０の場合
        else{
            //作業用ROIからアライメント対象となるROIを切り出すため、切り出し作業用ボタンを有効化
            ui->pushButton_drawROIa->setEnabled(true);

            //表示用画面の設定
            if(!this->window_created){
                cv::namedWindow("ROI", 1);
                this->window_created = true;
            }

            //作業用ROIの読み込み
            std::string imagepath = this->slicefolderpath_a + "/" + this->roiimagename_a + ".tiff";
            cv::Mat img = cv::imread(imagepath, 1);


            //そのままでは大きすぎるため、表示に適する倍率でリサイズ
            if(img.cols > this->limitW || img.rows>this->limitH){
                float roiw = (float)img.cols;
                float roih = (float)img.rows;
                float factorx = 1.0;
                float factory = 1.0;
                while(roiw>this->limitW || roih>this->limitH){
                    roiw /= 2.;
                    roih /= 2.;
                    factorx *= 2.;
                    factory *= 2.;
                }
                RESIZE_TIME_A = (int)factorx;
            }
            else{
                RESIZE_TIME_A = 1;
            }


            cv::resize(img, DISPIMG_A, cv::Size(img.cols/RESIZE_TIME_A, img.rows/RESIZE_TIME_A));

            //表示
            cv::imshow("ROI", DISPIMG_A);
            cv::waitKey(10);
        }

    }

}

//マウス操作用のコールバック関数
void on_mouse_function(int event, int x, int y, int flags, void* param){

    static cv::Point2i origin;
    cv::Rect *inputrect = static_cast<cv::Rect*>(param);

    //--- Calculate Coordinates of Selected Area (by clidk and drag) ------------//



    //--- Process a Start and a Finish Selecting Events (i.e. button-up and -down) ---//
    switch (event){
    case cv::EVENT_LBUTTONDOWN:
        if(!START_POINT_SELECTED_A){
            //描画の基点を取得
            origin = cv::Point2i(x, y);
            SELECTION_A = cv::Rect(x, y, 0, 0);
            START_POINT_SELECTED_A = true;
            OBJECT_SELECTED_A = false;
        }
        break;
    case cv::EVENT_LBUTTONUP:
        START_POINT_SELECTED_A = false;
        OBJECT_SELECTED_A = true;
        return;
   case cv::EVENT_MOUSEMOVE:
        if (START_POINT_SELECTED_A){
            //矩形情報を取得
            SELECTION_A.x = CV_IMIN(x, origin.x);
            SELECTION_A.y = CV_IMIN(y, origin.y);
            SELECTION_A.x = CV_IMAX(SELECTION_A.x, 0);
            SELECTION_A.y = CV_IMAX(SELECTION_A.y, 0);

            SELECTION_A.width = CV_IABS(x - origin.x);
            SELECTION_A.height = CV_IABS(y - origin.y);

            if(x<0){
                SELECTION_A.width = origin.x;
            }
            else if(x>inputrect->width){
                SELECTION_A.width = CV_IABS(inputrect->width - origin.x);
            }

            if(y<0){
                SELECTION_A.height = origin.y;
            }
            else if(y>inputrect->height){
                SELECTION_A.height = CV_IABS(inputrect->height - origin.y);
            }

        }
        else if(!OBJECT_SELECTED_A){
            SELECTION_A.x = x;
            SELECTION_A.y = y;
            SELECTION_A.width = 0;
            SELECTION_A.height = 0;
        }
        break;
    }
}


//ROIの描画
void MainWindow::on_pushButton_drawROIa_clicked()
{
    //--- ボタンの無効か ------------------------------//
    ui->pushButton_selectROIa->setEnabled(false);
    ui->pushButton_drawROIa->setEnabled(false);


    //--- 描画用フラグを初期化 -----------------------------------//
    START_POINT_SELECTED_A = false;
    OBJECT_SELECTED_A = false;
    MOUSE_INSIDE_ROIRECT_A = false;

    //--- 矩形情報の初期化 --------------------------------//
    SELECTION_A.x = SELECTION_A.y = SELECTION_A.width = SELECTION_A.height = 0;


    //---　描画用コールバック関数の設定 ------------------------//
    cv::Rect inputrect(0,0,DISPIMG_A.cols, DISPIMG_A.rows);
    cv::setMouseCallback("ROI", on_mouse_function, &inputrect);

    //=== LOOP ===//
    ui->pushButton_decideROIa->setEnabled(false);
    ui->pushButton_cancelROIa->setEnabled(false);

    //無限ループ脱出用フラグの初期化
    this->killLoopFlag_ = 0;


    while(1){

        //表示用画像の取得
        cv::Mat dispimg = DISPIMG_A.clone();

        //描画された矩形座標の修正
        if(SELECTION_A.x < 0) SELECTION_A.x = 0;
        else if(SELECTION_A.x + SELECTION_A.width > dispimg.cols)   SELECTION_A.x = dispimg.cols - SELECTION_A.width;
        if(SELECTION_A.y < 0) SELECTION_A.y = 0;
        else if(SELECTION_A.y+SELECTION_A.height > dispimg.rows) SELECTION_A.y = dispimg.rows - SELECTION_A.height;

        //画面に座標情報を表示
        QString coord_str = "x:" + QString::number(SELECTION_A.x) + " y:" + QString::number(SELECTION_A.y) + " (" + QString::number(SELECTION_A.width) + ", " + QString::number(SELECTION_A.height) + ")";
        ui->label_showROIcoordA->setText(coord_str);
        ui->label_showROIcoordA->repaint();


        //矩形を描画
        if(!OBJECT_SELECTED_A && SELECTION_A.width>0 && SELECTION_A.height > 0){
            cv::rectangle(dispimg, SELECTION_A, CV_RGB(0,0,255), 1, CV_AA, 0);
        }
        //描画終了時、画面上の矩形をROIとして設定するか判定
        else if(OBJECT_SELECTED_A){

            //設定用ボタンの有効化
            ui->pushButton_decideROIa->setEnabled(true);
            ui->pushButton_cancelROIa->setEnabled(true);

            //選択中の矩形を表示
            cv::rectangle(dispimg, SELECTION_A, CV_RGB(255,0,0), 1, CV_AA, 0);
            cv::imshow("ROI", dispimg);
            cv::waitKey(10);

            //決定ボタン、再描画ボタンと同期
            QApplication::processEvents(QEventLoop::AllEvents, 1000);

            //ROIが決定すればループを脱出、決定しなければ全設定を初期化して再度描画
            if(this->killLoopFlag_ == 1){
                break;
            }
            else if(this->killLoopFlag_ == -1){
                OBJECT_SELECTED_A = false;
                START_POINT_SELECTED_A = false;
                this->killLoopFlag_ = 0;
                SELECTION_A.x = 0;
                SELECTION_A.y = 0;
                SELECTION_A.width = 0;
                SELECTION_A.height = 0;
            }

            //無限ループ防止
            if(this->killLoopFlag_ == 1)
                break;

        }

        //矩形が描画された画像を表示
        cv::imshow("ROI", dispimg);
        cv::waitKey(10);
    }


    //--- 設定されたROI情報をもとに，全ての作業用ROIからROIを切り出し -------------------------//
    //進行状況ダイアログの初期化
    this->dialog->reset();
    this->dialog->setMinimum(0);
    this->dialog->setMaximum((int)this->adjustROIRects.size()- 1);
    this->dialog->setLabelText("ROI image loading");
    this->dialog->show();
    qApp->processEvents();

    for(int i=0; i<(int)this->sliceROIRects.size(); i++){


        //ダイアログの表示
        this->dialog->setValue(i);
        qApp->processEvents();

        //作業用ROIから設定されたROIと同じ位置を切り出すための座標計算
        roiRect roi;
        roi.imagename = this->sliceROIRects[i].imagename;
        roi.picNo = this->sliceROIRects[i].picNo;
        roi.rotated = this->sliceROIRects[i].rotated;
        if(this->adjust_target_level==2)
        {
            if(!roi.rotated){//作業用ROIのオリジナルの向きが、上下反転していない場合
                roi.roi_rect.x = (this->adjustROIRects[this->align_roi_id].roi_rect.x - this->sliceROIRects[this->align_roi_id].roi_rect.x)  + this->sliceROIRects[i].roi_rect.x;
                roi.roi_rect.y = (this->adjustROIRects[this->align_roi_id].roi_rect.y - this->sliceROIRects[this->align_roi_id].roi_rect.y)  + this->sliceROIRects[i].roi_rect.y;
            }
            else{
                roi.roi_rect.x = (this->sliceROIRects[this->align_roi_id].roi_rect.width - this->adjustROIRects[this->align_roi_id].roi_rect.x) - this->sliceROIRects[this->align_roi_id].roi_rect.x - this->adjustROIRects[this->align_roi_id].roi_rect.width + this->sliceROIRects[i].roi_rect.x;
                roi.roi_rect.y = (this->sliceROIRects[this->align_roi_id].roi_rect.height - this->adjustROIRects[this->align_roi_id].roi_rect.y) - this->sliceROIRects[this->align_roi_id].roi_rect.y - this->adjustROIRects[this->align_roi_id].roi_rect.height + this->sliceROIRects[i].roi_rect.y;
            }

        }
        else{

            if(!roi.rotated){//作業用ROIのオリジナルの向きが、上下反転していない場合
                roi.roi_rect.x = (this->adjustROIRects[this->align_roi_id].roi_rect.x);// - this->sliceROIRects[this->align_roi_id].roi_rect.x)  + this->sliceROIRects[i].roi_rect.x;
                roi.roi_rect.y = (this->adjustROIRects[this->align_roi_id].roi_rect.y);// - this->sliceROIRects[this->align_roi_id].roi_rect.y)  + this->sliceROIRects[i].roi_rect.y;
            }
            else{
                roi.roi_rect.x =  (this->adjustROIRects[this->align_roi_id].roi_rect.x);//this->adjustROIRects[this->align_roi_id].roi_rect.x - this->sliceROIRects[this->align_roi_id].roi_rect.width;//(this->sliceROIRects[this->align_roi_id].roi_rect.width - (this->adjustROIRects[this->align_roi_id].roi_rect.x);// - this->sliceROIRects[this->align_roi_id].roi_rect.x) - this->adjustROIRects[this->align_roi_id].roi_rect.width) + this->sliceROIRects[i].roi_rect.x;
                roi.roi_rect.y =  (this->adjustROIRects[this->align_roi_id].roi_rect.y);//this->adjustROIRects[this->align_roi_id].roi_rect.y - this->sliceROIRects[this->align_roi_id].roi_rect.height;//(this->sliceROIRects[this->align_roi_id].roi_rect.height - (this->adjustROIRects[this->align_roi_id].roi_rect.y);// - this->sliceROIRects[this->align_roi_id].roi_rect.y) - this->adjustROIRects[this->align_roi_id].roi_rect.height) + this->sliceROIRects[i].roi_rect.y;
            }
        }
        roi.roi_rect.width = this->adjustROIRects[this->align_roi_id].roi_rect.width;
        roi.roi_rect.height = this->adjustROIRects[this->align_roi_id].roi_rect.height;
        this->adjustROIRects[i] = roi;

        //ROIの切り出し
        std::string scnpath = this->scnfolderpath_a + "/" + this->adjustROIRects[i].imagename + ".scn";
        int64_t x = this->adjustROIRects[i].roi_rect.x;
        int64_t y = this->adjustROIRects[i].roi_rect.y;
        int64_t width = this->adjustROIRects[i].roi_rect.width;
        int64_t height = this->adjustROIRects[i].roi_rect.height;
        cv::Mat img;
        bool img_checked;
        if(this->adjust_target_level == 2)
            img_checked = showROI(scnpath, img, this->adjust_target_level, x, y, width, height);
        else{
            std::string imgpath = this->slicefolderpath_a + "/" + this->adjustROIRects[i].imagename + ".tiff";
            cv::Mat sliceimg = cv::imread(imgpath, 1);
            if(sliceimg.empty()){
                cv::imshow("windows", sliceimg);
                cv::waitKey(0);
            }
            img = sliceimg(roi.roi_rect);
            img_checked = true;
            if(img.empty())
                img_checked = false;
        }

        //読み込めた場合、保存
        if(img_checked){
            //flip
            if(this->adjustROIRects[i].rotated && this->adjust_target_level==2){
                cv::Mat tmpimg = img.clone();
                img.setTo(0);
                cv::flip(tmpimg, img, -1);
                tmpimg.release();
            }
            std::string savepath = this->roifolderpath_a + "/" + this->adjustROIRects[i].imagename + ".tiff";
            cv::imwrite(savepath, img);
        }

    }//i loop end


    //--- 表示用ウィンドウを削除 --------------------------------//
    cv::destroyWindow("ROI");
    this->window_created = false;

    ui->label_showROIcoordA->clear();
    ui->label_showROIcoordA->repaint();

    //---　決定ボタン等を無効化 ---------------------------------//
    ui->pushButton_decideROIa->setEnabled(false);
    ui->pushButton_cancelROIa->setEnabled(false);


    //--- アライメントの基準ROI選択用ボタン等の有効化 -----------------------------------//
    ui->pushButton_selectAdjImageA->setEnabled(true);
    ui->pushButton_quitAdjustA->setEnabled(true);

}


//ROIの決定
void MainWindow::on_pushButton_decideROIa_clicked()
{
    //--- ROI情報を保存 -------------------------------//
    //矩形情報を設定
    roiRect roi;
    roi.imagename = this->roiimagename_a;
    roi.rotated = this->sliceROIRects[this->align_roi_id].rotated;
    if(!roi.rotated){

        //スライス画像上での座標を計算
        int x, y;
        if(this->adjust_target_level == 2)
        {
            x = SELECTION_A.x * RESIZE_TIME_A + this->sliceROIRects[this->align_roi_id].roi_rect.x;
            y = SELECTION_A.y * RESIZE_TIME_A + this->sliceROIRects[this->align_roi_id].roi_rect.y;
        }
        else{
            x = SELECTION_A.x * RESIZE_TIME_A;// + this->sliceROIRects[this->align_roi_id].roi_rect.x;
            y = SELECTION_A.y * RESIZE_TIME_A;// + this->sliceROIRects[this->align_roi_id].roi_rect.y;

        }
//        //SCN画像上での座標を計算
//        cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
//        origin.at<double>(0, 0) = (double)x;
//        origin.at<double>(1, 0) = (double)y;
//        origin.at<double>(2, 0) = 1.0;
//        cv::Mat coord = this->past_matricies[this->align_roi_id] * origin;

        //保存
        roi.roi_rect.x = x;//(int)(coord.at<double>(0,0)+ (double)this->sliceROIRects[this->align_roi_id].roi_rect.x + 0.5);
        roi.roi_rect.y = y;//(int)(coord.at<double>(1,0)+ (double)this->sliceROIRects[this->align_roi_id].roi_rect.y + 0.5);

//        int roix = (int)(coord.at<double>(0,0));
//        int roiy = (int)(coord.at<double>(1,0));
//        int dumx = roi.roi_rect.x;
//        int dumy = roi.roi_rect.y;
//        int sdumx = this->sliceROIRects[this->align_roi_id].roi_rect.x;
//        int sdumy = this->sliceROIRects[this->align_roi_id].roi_rect.y;
    }
    else{
        //スライス画像上での座標を計算
        int x, y;
        if(this->adjust_target_level == 2){
            x = this->sliceROIRects[this->align_roi_id].roi_rect.width - SELECTION_A.x * RESIZE_TIME_A - SELECTION_A.width * RESIZE_TIME_A + this->sliceROIRects[this->align_roi_id].roi_rect.x;
            y = this->sliceROIRects[this->align_roi_id].roi_rect.height - SELECTION_A.y * RESIZE_TIME_A - SELECTION_A.height * RESIZE_TIME_A + this->sliceROIRects[this->align_roi_id].roi_rect.y;
        }
        else{
            x = SELECTION_A.x * RESIZE_TIME_A;//this->sliceROIRects[this->align_roi_id].roi_rect.width - SELECTION_A.x * RESIZE_TIME_A;// - SELECTION_A.width * RESIZE_TIME_A + this->sliceROIRects[this->align_roi_id].roi_rect.x;
            y = SELECTION_A.y * RESIZE_TIME_A;;//this->sliceROIRects[this->align_roi_id].roi_rect.height - SELECTION_A.y * RESIZE_TIME_A;// - SELECTION_A.height * RESIZE_TIME_A + this->sliceROIRects[this->align_roi_id].roi_rect.y;
        }

//        //SCN画像上での座標を計算
//        cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
//        origin.at<double>(0, 0) = (double)x;
//        origin.at<double>(1, 0) = (double)y;
//        origin.at<double>(2, 0) = 1.0;
//        cv::Mat coord = this->past_matricies[this->align_roi_id] * origin;

        //保存
        roi.roi_rect.x = x;//(int)(coord.at<double>(0,0)+ this->sliceROIRects[this->align_roi_id].roi_rect.x+0.5);
        roi.roi_rect.y = y;//(int)(coord.at<double>(1,0) + this->sliceROIRects[this->align_roi_id].roi_rect.y+0.5);
    }
    roi.roi_rect.width = SELECTION_A.width * RESIZE_TIME_A;
    roi.roi_rect.height = SELECTION_A.height * RESIZE_TIME_A;

    //ROI配列に設定した矩形情報を保存
    this->adjustROIRects.clear();
    this->adjustROIRects.resize((int)this->sliceROIRects.size());
    this->adjustROIRects[this->align_roi_id] = roi;

    //--- 描画用矩形情報の初期化 -------------------//
    SELECTION_A.x = SELECTION_A.y = SELECTION_A.width = SELECTION_A.height = 0;


 }


//描画中のROIをキャンセル
void MainWindow::on_pushButton_cancelROIa_clicked()
{
//    //--- Set flags ---------------//
//    OBJECT_SELECTED_A = false;
//    START_POINT_SELECTED_A = false;

    //--- 決定ボタン等を無効化 ----------//
    ui->pushButton_decideROIa->setEnabled(false);
    ui->pushButton_cancelROIa->setEnabled(false);

    //---描画用矩形情報の初期化-------------------//
    SELECTION_A.x = SELECTION_A.y = SELECTION_A.width = SELECTION_A.height = 0;
}




//========================================
//アライメント前のROI修正
//========================================
//修正対象の画像を取得
void MainWindow::on_pushButton_selectAdjImageA_clicked()
{

    //--- ROI編集用ボタンを無効化 ------------------------------------//
    ui->pushButton_selectROIa->setEnabled(false);
    ui->pushButton_drawROIa->setEnabled(false);
    ui->pushButton_decideROIa->setEnabled(false);
    ui->pushButton_cancelROIa->setEnabled(false);


    //--- 守勢対象ROIの取得 --------------------------//
    //全ROI画像が保存されているフォルダのパスを作成
    std::string folderpath = this->roifolderpath_a;

    //修正対象のROI画像名を取得
    QFileDialog::Options options;
    QString strSelectedFilter;
    QStringList strFNames = QFileDialog::getOpenFileNames(
                this,
                tr("select"),
                QString::fromStdString(folderpath),
                tr("*.tiff"),
                &strSelectedFilter, options );



    //-- 選択された場合、保存 -----------------------------------//
    if(!strFNames.isEmpty()){

        //画像名を表示
        this->adjfiles.clear();
        ui->textEdit_showSelectedImagesA->clear();
        ui->textEdit_showSelectedImagesA->repaint();

        //画像名を取得し、修正対象として記録
        for(int i=0; i<strFNames.size(); i++){
            QString fname = strFNames[i];
            std::string stdfname = fname.toStdString();

            std::string::size_type idxd = stdfname.find(".");
            std::string::size_type idxs = stdfname.find_last_of("/");
            if(idxd != std::string::npos)
                stdfname.erase(idxd, stdfname.size());
            if(idxs != std::string::npos)
                stdfname.erase(0, idxs+1);
            std::string filepath = stdfname;

            //stack
            this->adjfiles.push_back(filepath);

            //display
            ui->textEdit_showSelectedImagesA->append(QString::fromStdString(filepath));
            ui->textEdit_showSelectedImagesA->repaint();

        }//i loop end

        //修正対象選択ボタンを無効化
        ui->pushButton_selectAdjImageA->setEnabled(false);
        ui->pushButton_quitAdjustA->setEnabled(false);

        //修正対象枚数に応じて、処理を行うボタンを有効化
        int imgNum = (int)this->adjfiles.size();
        if(imgNum == 1){  //修正対象が１枚の時は、再描画、削除、上下反転操作が可能
            ui->pushButton_redrawAdjImageA->setEnabled(true);
            ui->pushButton_removeAdjImageA->setEnabled(true);
            ui->pushButton_rotateAdjImageA->setEnabled(true);
        }
        else{
            ui->pushButton_redrawAdjImageA->setEnabled(false);
            ui->pushButton_removeAdjImageA->setEnabled(true);
            ui->pushButton_rotateAdjImageA->setEnabled(true);
        }


    }
    else{
        return;
    }


}

//選択中の画像を削除
void MainWindow::on_pushButton_removeAdjImageA_clicked()
{

    //--- 編集用ボタンを無効化 -----------------------------------------//
    ui->pushButton_removeAdjImageA->setEnabled(false);
    ui->pushButton_redrawAdjImageA->setEnabled(false);
    ui->pushButton_rotateAdjImageA->setEnabled(false);


    //--- 最終確認 ----------------------------------//
    QMessageBox::StandardButton res = QMessageBox::question(this, "FINAL CHECK", "Are you sure you want remove this images from input?", QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Cancel || res == QMessageBox::Close){
        //enable
        ui->pushButton_selectAdjImageA->setEnabled(true);
        ui->pushButton_quitAdjustA->setEnabled(true);
        //clear
        ui->textEdit_showSelectedImagesA->clear();
        ui->textEdit_showSelectedImagesA->repaint();

        return;
    }

    //削除対象に基準ROIが含まれていないか確認
    bool roi_checked = false;
    for(int i=0; i < (int)this->adjfiles.size(); i++){
        std::string filename = this->adjfiles[i];
        if(filename == this->roiimagename_a){
            roi_checked = true;
            break;
        }
    }

    //含まれていた場合、再度最終確認メッセージを表示
    if(roi_checked){
        res = QMessageBox::question(this, "FINAL CHECK", "ROI image will be removed. Are you really sure?", QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Cancel || res == QMessageBox::Close){
            //enable
            ui->pushButton_selectAdjImageA->setEnabled(true);
            ui->pushButton_quitAdjustA->setEnabled(true);
            //clear
            ui->textEdit_showSelectedImagesA->clear();
            ui->textEdit_showSelectedImagesA->repaint();

            return;
        }

    }

    //--- 選択された画像をアライメント対象から削除 ----------------------------------------------//
    std::vector<int> removeid;

    //削除対象となるインデックスを取得
    for(int i=0; i<(int)this->adjfiles.size(); i++){
        //get a filename
        std::string filename = this->adjfiles[i];

        //search
        for(int j=0; j<(int)this->adjustROIRects.size(); j++){
            if(filename == this->adjustROIRects[j].imagename){
                removeid.push_back(j);
                break;
            }
        }
    }

    //削除
    int idcount = 0;
    int targetID = removeid[idcount];

    std::vector<roiRect> tmprects;
    std::vector<roiRect> tmpslices;
    std::vector<roiRect> tmpuslices;
    std::vector<cv::Mat> tmpmat, tmpmat0, tmpmat1;
    std::vector<cv::Mat> tmpinvmat, tmpinvmat0, tmpinvmat1;

    std::sort(removeid.begin(), removeid.end());    //sort


    for(int i=0; i<(int)this->adjustROIRects.size(); i++){
        //削除対象でなければ保存
        if(i != targetID || idcount == (int)removeid.size()){
            tmprects.push_back(this->adjustROIRects[i]);
            if(this->adjust_target_level == 1)
            {
                tmpslices.push_back(this->sliceROIRects[i]);
                tmpmat0.push_back(this->past_level0_matricies[i]);
                tmpmat1.push_back(this->past_level1_matricies[i]);
                tmpinvmat1.push_back(this->past_level1_matricies_inv[i]);
            }
            else if(this->adjust_target_level == 0)
            {
                tmpuslices.push_back(this->uppersliceROIRects[i]);
                tmpmat.push_back((this->past_matricies[i]));
                tmpinvmat.push_back((this->past_matricies_inv[i]));
            }
        }
        else{
            //削除対象画像名を取得
            std::string filename = this->adjustROIRects[i].imagename;
            //削除対象のROIパスを作成
            std::string imagepath = this->roifolderpath_a + "/" + filename + ".tiff";
            //削除対象の作業用ROIパスを作成
            std::string imagepath_slice = this->slicefolderpath_a + "/" + filename + ".tiff";
            //ROI画像を削除
            std::remove(imagepath.c_str());
            //作業用ROI画像を削除
            std::remove(imagepath_slice.c_str());
            //increment idcount
            idcount++;
            //次の削除対象インデックスを取得
            if(idcount != (int)removeid.size())
                targetID = removeid[idcount];
        }
    }

    //ROI情報の更新
    this->adjustROIRects.clear();
    if(this->adjust_target_level == 1)
    {
        this->sliceROIRects.clear();
        this->past_level0_matricies.clear();
        this->past_level1_matricies.clear();
        this->past_level1_matricies_inv.clear();
    }
    else if(this->adjust_target_level == 0)
    {
        this->past_matricies.clear();
        this->past_matricies_inv.clear();
        this->uppersliceROIRects.clear();
    }
    for(int i=0; i<(int)tmprects.size(); i++){
        this->adjustROIRects.push_back(tmprects[i]);
        if(this->adjust_target_level == 1)
        {
            this->sliceROIRects.push_back(tmpslices[i]);
            this->past_level0_matricies.push_back(tmpmat0[i]);
            this->past_level1_matricies.push_back(tmpmat1[i]);
            this->past_level1_matricies_inv.push_back(tmpinvmat1[i]);
        }
        else if(this->adjust_target_level == 0)
        {
            this->past_matricies.push_back(tmpmat[i]);
            this->past_matricies_inv.push_back(tmpinvmat[i]);
            this->uppersliceROIRects.push_back(tmpuslices[i]);
        }
    }

    //--- アライメント基準ROI情報の更新 --------------------------------------------//
    if(roi_checked){
        this->align_roi_id = 0;
        this->roiimagename_a = this->adjustROIRects[0].imagename;
    }
    else{
        for(int i=0; i<(int)this->adjustROIRects.size(); i++){
            std::string filename = this->adjustROIRects[i].imagename;
            if(filename == this->roiimagename_a){
                this->align_roi_id = i;
                break;
            }
        }
    }


    //--- 選択ボタン等を有効化 ------------------------------------------//
    ui->pushButton_selectAdjImageA->setEnabled(true);
    ui->pushButton_quitAdjustA->setEnabled(true);

    //--- 選択中ファイル名表示用テキストボックスを初期化 -------------------------------------------//
    ui->textEdit_showSelectedImagesA->clear();
    ui->textEdit_showSelectedImagesA->repaint();

}

//上下反転
void MainWindow::on_pushButton_rotateAdjImageA_clicked()
{

    //--- 編集用ボタンを無効化 -----------------------------------------//
    ui->pushButton_removeAdjImageA->setEnabled(false);
    ui->pushButton_redrawAdjImageA->setEnabled(false);
    ui->pushButton_rotateAdjImageA->setEnabled(false);


    //---　最終確認 ----------------------------------//
    QMessageBox::StandardButton res = QMessageBox::question(this, "FINAL CHECK", "Are you sure you want rotate this images?", QMessageBox::Yes | QMessageBox::Cancel);
    if(res == QMessageBox::Cancel || res == QMessageBox::Close){
        //enable
        ui->pushButton_selectAdjImageA->setEnabled(true);
        ui->pushButton_quitAdjustA->setEnabled(true);
        //clear
        ui->textEdit_showSelectedImagesA->clear();
        ui->textEdit_showSelectedImagesA->repaint();

        return;
    }

    //--- 上下反転 ---------------------------------------//
    for(int i=0; i<(int)this->adjfiles.size(); i++){
        //画像名を取得
        std::string filename = this->adjfiles[i];

        for(int j=0; j<(int)this->adjustROIRects.size(); j++){
            //対象画像の場合、処理を実行
            if(filename == this->adjustROIRects[j].imagename){

                //ROIを読み込み
                std::string imagepath = this->roifolderpath_a + "/" + filename + ".tiff";
                cv::Mat img = cv::imread(imagepath, 1);
                if(img.empty()){
                   QMessageBox::information(this, "CAUTION", "An image can not be rotated." + QString::fromStdString(imagepath));
                   continue;
                }

                //作業用ROIを読み込み
               std::string imagepath_slice = this->slicefolderpath_a + "/" + filename + ".tiff";
               cv::Mat img_slice = cv::imread(imagepath_slice, 1);
               if(img_slice.empty()){
                  QMessageBox::information(this, "CAUTION", "A alice image can not be rotated." + QString::fromStdString(imagepath_slice));
                  continue;
               }

                //ROIを上下反転
                cv::Mat tmp = img.clone();
                img.release();
                cv::flip(tmp, img, -1);

                //作業用ROIを上下反転
                tmp = img_slice.clone();
                img_slice.release();
                cv::flip(tmp, img_slice, -1);
                tmp.release();

                //反転処理後のROIを保存
                cv::imwrite(imagepath, img);

                //反転処理後の作業用ROIを保存
                cv::imwrite(imagepath_slice, img_slice);

                //反転フラグをセット
                if(this->adjustROIRects[j].rotated)
                    this->adjustROIRects[j].rotated = false;
                else
                    this->adjustROIRects[j].rotated = true;

                //break
                break;
            }
        }
    }


    //--- 選択ボタン等を有効化 ------------------------------------------//
    ui->pushButton_selectAdjImageA->setEnabled(true);
    ui->pushButton_quitAdjustA->setEnabled(true);

    //--- 選択中ファイル名表示用テキストボックスの初期化 -------------------------------------------//
    ui->textEdit_showSelectedImagesA->clear();
    ui->textEdit_showSelectedImagesA->repaint();


}

//再描画用のマウスコールバック関数
void on_mouse_function2(int event, int x, int y, int flags, void* param){

    cv::Rect *inputrect = static_cast<cv::Rect*>(param);

    //ROI中心部に小さな矩形を設定し、マウスポインタの内外判定に利用
    int minx = inputrect->x + inputrect->width/2 - inputrect->width/10;
    int maxx = inputrect->x + inputrect->width/2 + inputrect->width/10;
    int miny = inputrect->y + inputrect->height/2 - inputrect->height/10;
    int maxy = inputrect->y + inputrect->height/2 + inputrect->height/10;


    //--- マウス操作別に矩形情報を設定 ---//
    switch (event){
    case cv::EVENT_MOUSEMOVE:
        //設定されているROIの矩形をスタンプとして画像に描画
        if(x > minx && x < maxx && y > miny && y < maxy && !MOUSE_INSIDE_ROIRECT_A){
            MOUSE_INSIDE_ROIRECT_A = true;
        }

        if(!OBJECT_SELECTED_A && MOUSE_INSIDE_ROIRECT_A){
            SELECTION_A = cv::Rect(x - inputrect->width / 2, y - inputrect->height / 2, inputrect->width, inputrect->height);
        }
        else if(!OBJECT_SELECTED_A && !MOUSE_INSIDE_ROIRECT_A){
            SELECTION_A = *inputrect;
        }

        break;
    case cv::EVENT_LBUTTONDOWN:
        OBJECT_SELECTED_A = true;
        MOUSE_INSIDE_ROIRECT_A = false;
        break;
    }
}

//ROIの再描画
void MainWindow::on_pushButton_redrawAdjImageA_clicked()
{

    //--- 編集用ボタンを無効化 -----------------------------------------//
    ui->pushButton_removeAdjImageA->setEnabled(false);
    ui->pushButton_redrawAdjImageA->setEnabled(false);
    ui->pushButton_rotateAdjImageA->setEnabled(false);


    //--- 再描画の対象ROIインデックスを取得 ---------------------------------------------//
    int targetID = -1;
    for(int i=0; i<(this)->adjustROIRects.size(); i++){
        //get a filename
        std::string filename = this->adjustROIRects[i].imagename;

        //check
        if(filename == this->adjfiles[0]){
            targetID = i;
            break;
        }
    }

    //画像が存在しない場合、強制的に戻る
    if(targetID == -1){
        QMessageBox::information(this, "CAUTION", "This image doesn't exist : " + QString::fromStdString(this->adjfiles[0]));
        //enable
        ui->pushButton_selectAdjImageA->setEnabled(true);
        ui->pushButton_quitAdjustA->setEnabled(true);
        //clear
        ui->textEdit_showSelectedImagesA->clear();
        ui->textEdit_showSelectedImagesA->repaint();

        return;
    }


    //--- 再描画対象ROIの作業用ROIを読み込み --------------------------------------------//
    std::string filename = this->adjfiles[0];
    std::string imagepath = this->slicefolderpath_a + "/" + filename + ".tiff";
    cv::Mat img = cv::imread(imagepath, 1);
    if(img.empty()){
        QMessageBox::information(this, "CAUTION", "This image doesn't exist");
        //enable buttons
        ui->pushButton_selectAdjImageA->setEnabled(true);
        ui->pushButton_quitAdjustA->setEnabled(true);
        //clear
        ui->textEdit_showSelectedImagesA->clear();
        ui->textEdit_showSelectedImagesA->repaint();
        return;
    }


    //---　対象画像の解像度３における幅と高さを取得 --------------------//
    std::string scnpath = this->scnfolderpath_a + "/" + filename + ".scn";
    openslide_t *obj = openslide_open(scnpath.c_str());
    int64_t W, H;
    if(obj != NULL){
        openslide_get_layer_dimensions(obj, 3, &W, &H);
    }
    else{
        W = H =0;
    }
    int dx = (this->campusW - W)/2;
    int dy = (this->campusH - H)/2;


    //--- 画像を表示 ---------------------------------------//
    //リサイズ
    cv::Mat resizeimg;
    if(this->adjust_target_level == 2 && RESIZE_TIME_A == -1){

        if(RESIZE_TIME_A == -1){
            if(img.cols > this->limitW || img.rows > this->limitH){
                float roiw = (float)img.cols;
                float roih = (float)img.rows;
                float factorx = 1.0;
                while(roiw > this->limitW || roih > this->limitH){
                    roiw /= 2.;
                    roih /= 2.;
                    factorx *= 2.;
                }

                RESIZE_TIME_A = (int)factorx;
            }
            else{
                RESIZE_TIME_A = 1;
            }
        }

   }
    cv::resize(img, resizeimg, cv::Size(img.cols/RESIZE_TIME_A, img.rows/RESIZE_TIME_A));


    //ROIの矩形情報を設定
    cv::Rect currentroi;
    if(this->adjust_target_level == 2){
        //set current roi
        currentroi.x = (this->adjustROIRects[targetID].roi_rect.x) / (4 * RESIZE_TIME_A)+ dx/RESIZE_TIME_A;
        currentroi.y = (this->adjustROIRects[targetID].roi_rect.y) / (4 * RESIZE_TIME_A)+ dy/RESIZE_TIME_A;
        currentroi.width = this->adjustROIRects[targetID].roi_rect.width / (4 * RESIZE_TIME_A);
        currentroi.height = this->adjustROIRects[targetID].roi_rect.height / (4 * RESIZE_TIME_A);
    }
    else{
        //set current roi
        currentroi.x = this->adjustROIRects[targetID].roi_rect.x / RESIZE_TIME_A;//(this->adjustROIRects[targetID].roi_rect.x - this->sliceROIRects[targetID].roi_rect.x) / RESIZE_TIME_A;
        currentroi.y = this->adjustROIRects[targetID].roi_rect.y  / RESIZE_TIME_A;//(this->adjustROIRects[targetID].roi_rect.y - this->sliceROIRects[targetID].roi_rect.y) / RESIZE_TIME_A;
        currentroi.width = this->adjustROIRects[targetID].roi_rect.width / RESIZE_TIME_A;
        currentroi.height = this->adjustROIRects[targetID].roi_rect.height / RESIZE_TIME_A;

    }


    //対象画像が上下判定している場合、座標を修正
    if(this->adjustROIRects[targetID].rotated){
        //coordinate
        if(this->adjust_target_level == 2){
            currentroi.x = resizeimg.cols - (this->adjustROIRects[targetID].roi_rect.x / (4 * RESIZE_TIME_A) + dx/RESIZE_TIME_A) - this->adjustROIRects[targetID].roi_rect.width / (4 * RESIZE_TIME_A);
            currentroi.y = resizeimg.rows - (this->adjustROIRects[targetID].roi_rect.y / (4 * RESIZE_TIME_A) + dy/RESIZE_TIME_A) - this->adjustROIRects[targetID].roi_rect.height / (4 * RESIZE_TIME_A);
        }
//        else{
//            currentroi.x = resizeimg.cols - (this->adjustROIRects[targetID].roi_rect.x - this->sliceROIRects[targetID].roi_rect.x) / RESIZE_TIME_A - this->adjustROIRects[targetID].roi_rect.width / RESIZE_TIME_A;
//            currentroi.y = resizeimg.rows - (this->adjustROIRects[targetID].roi_rect.y - this->sliceROIRects[targetID].roi_rect.y) / RESIZE_TIME_A - this->adjustROIRects[targetID].roi_rect.height / RESIZE_TIME_A;
//        }

    }

    //作業用ROI画像を表示
    cv::namedWindow("ROI", 1);
    cv::imshow("ROI", resizeimg);

    //コールバック関数を設定
    cv::setMouseCallback("ROI", on_mouse_function2, &currentroi);

    //=== LOOP ===//

    //初期化
    OBJECT_SELECTED_A = false;
    MOUSE_INSIDE_ROIRECT_A = false;
    START_POINT_SELECTED_A = false;

    SELECTION_A.x = currentroi.x;
    SELECTION_A.y = currentroi.y;
    SELECTION_A.width = currentroi.width;
    SELECTION_A.height = currentroi.height;

    while(1){
        //描画用画像を設定
        cv::Mat dispimg = resizeimg.clone();

        //矩形情報を描画用に修正
        if(SELECTION_A.x < 0)   SELECTION_A.x = 0;
        else if(SELECTION_A.x + SELECTION_A.width > resizeimg.cols) SELECTION_A.x = resizeimg.cols - SELECTION_A.width;
        if(SELECTION_A.y < 0)   SELECTION_A.y = 0;
        else if(SELECTION_A.y + SELECTION_A.height > resizeimg.rows) SELECTION_A.y = resizeimg.rows - SELECTION_A.height;

        //描画
        if(!OBJECT_SELECTED_A && SELECTION_A.width>0 && SELECTION_A.height > 0){
            cv::rectangle(dispimg, SELECTION_A, CV_RGB(0,0,255), 1, CV_AA, 0);
            cv::rectangle(dispimg, cv::Point(SELECTION_A.x + SELECTION_A.width/2 - SELECTION_A.width/10, SELECTION_A.y + SELECTION_A.height/2 - SELECTION_A.height/10),
                          cv::Point(SELECTION_A.x + SELECTION_A.width/2 + SELECTION_A.width/10, SELECTION_A.y + SELECTION_A.height/2 + SELECTION_A.height/10),
                          CV_RGB(0,255,0), 1, CV_AA, 0);

        }
        //描画が終了した場合、新しいROIとして保存するか、もう一度描画するかを判定
        else if(OBJECT_SELECTED_A){

            //draw a selected rectangle & display
            cv::rectangle(dispimg, SELECTION_A, CV_RGB(255,0,0), 1, CV_AA, 0);
            cv::imshow("ROI", dispimg);
            cv::waitKey(10);

            //wait input
            QMessageBox::StandardButton res = QMessageBox::question(this, "FINAL CHECK", "Set a new ROI?", QMessageBox::Yes | QMessageBox::Cancel);
            if(res == QMessageBox::Cancel || res == QMessageBox::Close){
                OBJECT_SELECTED_A = false;
                MOUSE_INSIDE_ROIRECT_A = false;
                SELECTION_A = currentroi;
            }
            else if(res == QMessageBox::Yes){
                break;
            }
        }

        //矩形を描画した画像を表示
        cv::imshow("ROI", dispimg);
        cv::waitKey(10);
    }


    //--- 表示用ウィンドウの破棄 -----------------------------//
    cv::destroyWindow("ROI");


    //--- 新しいROI情報を保存 -------------------------------//
    roiRect roi;
    roi.imagename = this->adjustROIRects[targetID].imagename;
    roi.picNo = this->adjustROIRects[targetID].picNo;
    roi.rotated = this->adjustROIRects[targetID].rotated;
    if(this->adjust_target_level == 2){
        if(!roi.rotated){
            roi.roi_rect.x = (SELECTION_A.x*RESIZE_TIME_A - dx)*4;
            roi.roi_rect.y = (SELECTION_A.y*RESIZE_TIME_A - dy)*4;
        }
        else{
            roi.roi_rect.x = img.cols*4 - (SELECTION_A.x*RESIZE_TIME_A*4) - this->adjustROIRects[targetID].roi_rect.width - dx*4; //ayumi
            roi.roi_rect.y = img.rows*4 - (SELECTION_A.y*RESIZE_TIME_A*4) - this->adjustROIRects[targetID].roi_rect.height - dy*4;
        }
    }
    else{
        //スライス画像上での座標を計算
        int x, y;
        if(true){//(!roi.rotated){
            x = SELECTION_A.x * RESIZE_TIME_A;// + this->sliceROIRects[targetID].roi_rect.x;
            y = SELECTION_A.y * RESIZE_TIME_A;// + this->sliceROIRects[targetID].roi_rect.y;
        }
//        else{
//            x = img.cols - SELECTION_A.x*RESIZE_TIME_A;// - this->adjustROIRects[targetID].roi_rect.width + this->sliceROIRects[targetID].roi_rect.x;
//            y = img.rows - SELECTION_A.y*RESIZE_TIME_A;// - this->adjustROIRects[targetID].roi_rect.height + this->sliceROIRects[targetID].roi_rect.y;
//        }

//        //SCN画像上での座標を計算
//        cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
//        origin.at<double>(0, 0) = (double)x;
//        origin.at<double>(1, 0) = (double)y;
//        origin.at<double>(2, 0) = 1.0;
//        cv::Mat coord = this->past_matricies_inv[targetID] * origin;//this->past_matricies_inv[targetID] * origin;

        //保存
        roi.roi_rect.x = x;//(int)(coord.at<double>(0,0)+0.5+ this->sliceROIRects[targetID].roi_rect.x);
        roi.roi_rect.y = y;//(int)(coord.at<double>(1,0)+0.5+ this->sliceROIRects[targetID].roi_rect.y);
    }
    roi.roi_rect.width = this->adjustROIRects[targetID].roi_rect.width;
    roi.roi_rect.height = this->adjustROIRects[targetID].roi_rect.height;
    this->adjustROIRects[targetID] = roi;


    //--- 新しいROI画像を保存 ----------------------------------//
    //矩形情報の設定
    filename = this->adjustROIRects[targetID].imagename;
    scnpath = this->scnfolderpath_a + "/" + filename + ".scn";
    int64_t x = this->adjustROIRects[targetID].roi_rect.x;
    int64_t y = this->adjustROIRects[targetID].roi_rect.y;
    int64_t width = this->adjustROIRects[targetID].roi_rect.width;
    int64_t height = this->adjustROIRects[targetID].roi_rect.height;


    //元画像からの切り出し
    cv::Mat roiimg;
    bool roi_checked;
    if(this->adjust_target_level == 2)
       roi_checked = showROI(scnpath, roiimg, this->adjust_target_level, x, y, width, height);
    else
    {
        std::string imgpath = this->slicefolderpath_a + "/" + filename + ".tiff";
        cv::Mat img = cv::imread(imgpath, 1);
        roiimg = img(this->adjustROIRects[targetID].roi_rect);
        roi_checked = true;
    }

    //切り出した画像を保存
    if(roi_checked){
        //flip
        if(this->adjustROIRects[targetID].rotated && this->adjust_target_level == 2){//(this->adjustROIRects[targetID].rotated){
            cv::Mat tmpimg = roiimg.clone();
            roiimg.setTo(0);
            cv::flip(tmpimg, roiimg, -1);
            tmpimg.release();
        }
        std::string savepath = this->roifolderpath_a + "/" + filename + ".tiff";
        cv::imwrite(savepath, roiimg);
    }


    //--- 選択ボタン等を有効化 ------------------------------------------//
    ui->pushButton_selectAdjImageA->setEnabled(true);
    ui->pushButton_quitAdjustA->setEnabled(true);

    //--- 選択中ファイル名表示用テキストボックスの初期化 -------------------------------------------//
    ui->textEdit_showSelectedImagesA->clear();
    ui->textEdit_showSelectedImagesA->repaint();
}




//============================
//編集の修了
//=============================
void MainWindow::on_pushButton_quitAdjustA_clicked()
{
    //---　編集用ボタン等を無効化 ----------------------------------------//
    ui->pushButton_selectAdjImageA->setEnabled(false);
    ui->pushButton_removeAdjImageA->setEnabled(false);
    ui->pushButton_redrawAdjImageA->setEnabled(false);
    ui->pushButton_rotateAdjImageA->setEnabled(false);
    ui->pushButton_quitAdjustA->setEnabled(false);


    //---　これ以降必要ないため、作業用ROIをすべて削除 ------------------------------------//
    for(int i=0; i<(int)this->sliceROIRects.size(); i++){
        //get a filename
        std::string filename = this->sliceROIRects[i].imagename;

        //create an imagepath
        std::string imagepath = this->slicefolderpath_a + "/" + filename + ".tiff";

        //remove
        std::remove(imagepath.c_str());

    }

    //作業用ROIフォルダの削除
    std::remove(this->slicefolderpath_a.c_str());


    //--- アライメント実行用ボタンを有効化 ------------------------------------------//
    ui->pushButton_exeProgA->setEnabled(true);

}


//================================
//　アライメントを続行
//================================
void MainWindow::on_pushButton_continueProgA_clicked()
{

    //--- 解像度および基準ROIフラグをリセット ----------------------------------------//
    this->level_a_selected = false;
    this->adjust_roi_selected = false;


    //--- ROI情報をリセット --------------------------------------//
    this->adjustROIRects.clear();
    this->sliceROIRects.clear();


    //--- テキストボックスを初期化 ----------------------------------------//
    ui->lineEdit_showROINameA->clear();
    ui->lineEdit_showROINameA->repaint();

    ui->textEdit_showSelectedImagesA->clear();
    ui->textEdit_showSelectedImagesA->repaint();


    //--- アライメント続行ボタン等を無効化 -------------------------------------//
    ui->pushButton_continueProgA->setEnabled(false);
    ui->pushButton_quitProgA->setEnabled(false);


    //--- そのほかのボタンを有効化-------------------------------------//
    //push buttons
    ui->pushButton_selectINFa->setEnabled(true);
    ui->pushButton_selectOUTa->setEnabled(true);
    ui->pushButton_selectSCNa->setEnabled(true);
    ui->pushButton_decideInputA->setEnabled(true);

    //radio buttons
    ui->radioButton_level0->setAutoExclusive(false);
    ui->radioButton_level0->setChecked(false);
    ui->radioButton_level0->setAutoExclusive(true);
    ui->radioButton_level0->setEnabled(true);

    ui->radioButton_level1->setAutoExclusive(false);
    ui->radioButton_level1->setChecked(false);
    ui->radioButton_level1->setAutoExclusive(true);
    ui->radioButton_level1->setEnabled(true);

    ui->radioButton_level2->setAutoExclusive(false);
    ui->radioButton_level2->setChecked(false);
    ui->radioButton_level2->setAutoExclusive(true);
    ui->radioButton_level2->setEnabled(true);

}



//=============================
// アライメントプログラムを終了
//=============================
void MainWindow::on_pushButton_quitProgA_clicked()
{

    //--- 表示情報を初期化 ---------------------------------------//
    //alignment variables
    this->inffolderpath_a.clear();
    this->scnfolderpath_a.clear();
    this->outfolderpath_a.clear();

    this->workfolderpath_a.clear();
    this->resultfolderpath_a.clear();
    this->infresultfolderpath_a.clear();

    this->sliceinfpath_a.clear();
    this->slicefolderpath_a.clear();
    this->roifolderpath_a.clear();

    this->level_resultfolderpath_a.clear();
    this->roiimagename_a.clear();

    this->adjfiles.clear();

    //detection variables
    this->scnfolderpath_d.clear();
    this->outfolderpath_d.clear();


    //--- フラグを初期化 -----------------------------------------------//
    //alignment variables
    this->inffolder_a_selected = false;
    this->scnfolder_a_selected = false;
    this->outfolder_a_selected = false;
    this->level_a_selected = false;
    this->adjust_roi_selected = false;

    //detection variables
    this->scnfolder_d_selected = false;
    this->outfolder_d_selected = false;

    //--- ROI情報を初期化 --------------------------------------------//
    this->adjustROIRects.clear();
    this->sliceROIRects.clear();


    //--- テキストボックスを初期化 ----------------------------------------------//
    ui->lineEdit_selectINFa->clear();
    ui->lineEdit_selectINFa->repaint();

    ui->lineEdit_selectOUTa->clear();
    ui->lineEdit_selectOUTa->repaint();

    ui->lineEdit_selectSCNa->clear();
    ui->lineEdit_selectSCNa->repaint();

    ui->lineEdit_showROINameA->clear();
    ui->lineEdit_showROINameA->repaint();

    ui->textEdit_showSelectedImagesA->clear();
    ui->textEdit_showSelectedImagesA->repaint();

    ui->label_showROIcoordA->clear();
    ui->label_showROIcoordA->repaint();


    //--- ボタンを有効化 ------------------------------------------//
    //push buttons
    ui->pushButton_selectINFa->setEnabled(true);
    ui->pushButton_selectOUTa->setEnabled(true);
    ui->pushButton_selectSCNa->setEnabled(true);
    ui->pushButton_decideInputA->setEnabled(true);

    //radio buttons
    ui->radioButton_level0->setAutoExclusive(false);
    ui->radioButton_level0->setChecked(false);
    ui->radioButton_level0->setAutoExclusive(true);
    ui->radioButton_level0->setEnabled(true);

    ui->radioButton_level1->setAutoExclusive(false);
    ui->radioButton_level1->setChecked(false);
    ui->radioButton_level1->setAutoExclusive(true);
    ui->radioButton_level1->setEnabled(true);

    ui->radioButton_level2->setAutoExclusive(false);
    ui->radioButton_level2->setChecked(false);
    ui->radioButton_level2->setAutoExclusive(true);
    ui->radioButton_level2->setEnabled(true);

    //--- アライメント続行ボタン等を無効化 -----------------------------------------//
    ui->pushButton_continueProgA->setEnabled(false);
    ui->pushButton_quitProgA->setEnabled(false);


    //---　切り出しタブの有効化 --------------------------------------------------//
    ui->tab_detect->setEnabled(true);
}


