
/****************************
 * Elastix alignment
 * **************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "rigid_parameter.h"

#include <openslide.h>

//============================
//
//=============================
void MainWindow::on_pushButton_exeProgA_clicked()
{

    //Variables
    int adjx, adjy;
    float factorx, factory;

    std::string parameterpath;

    std::vector<roiRect> upperROIRects((int)this->adjustROIRects.size());
    std::vector<cv::Mat> upperROIRectMatrix((int)this->adjustROIRects.size());
    std::vector<cv::Mat> uupperROIRectMatrix((int)this->adjustROIRects.size());

    cv::Mat rigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat rotate_rigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat prigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat rotate_prigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);

    cv::Mat urigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat rotate_urigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat purigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat rotate_purigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);

    cv::Mat uurigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat rotate_uurigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat puurigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
    cv::Mat rotate_puurigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);

    //FOR CHECK OF METRIC VALUE
    std::vector<double> metrics;


    //--- アライメント実行ボタンを無効化 ----------------------------------------------//
    ui->pushButton_exeProgA->setEnabled(false);


    //---作業用フォルダの作成 -----------------------------//
    std::string elastix_workfolderpath_a = this->roifolderpath_a + "/elastix";
    bool folder_checked = createFolder(elastix_workfolderpath_a);

    std::string elastix_outfolderpath_a = elastix_workfolderpath_a + "/out";
    folder_checked = createFolder(elastix_outfolderpath_a);

    std::string elastix_originalfolderpath_a = this->level_resultfolderpath_a + "/original";
    folder_checked = createFolder(elastix_originalfolderpath_a);




    //--- 作業用ROI読み込み用変数の初期化 ----------------------------------//
    int64_t width, height;
    if(this->adjust_target_level == 2)
    {
        width = this->adjustROIRects[0].roi_rect.width;
        height = this->adjustROIRects[0].roi_rect.height;
    }
    else if(this->adjust_target_level == 1){
        width = this->sliceROIRects[0].roi_rect.width;
        height = this->sliceROIRects[0].roi_rect.height;
    }
    else{
        width = this->uppersliceROIRects[0].roi_rect.width;
        height = this->uppersliceROIRects[0].roi_rect.height;
    }
    int64_t workw = width * 2;
    int64_t workh = height * 2;
    int64_t upper_width, upper_height;

    uint32_t *dest;

    //get a memory
    try{
        dest = new uint32_t[workw*workh];
    }
    catch(bad_alloc){
        QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! We can't prepare enough memory");
        exit(-1);
    }




    //--- ELASTIXの実行 -------------------------------------------------------//
    //進行状況ダイアログの初期化
    this->dialog->reset();
    this->dialog->setMinimum(0);
    this->dialog->setMaximum((int)this->adjustROIRects.size());
    this->dialog->show();
    qApp->processEvents();


    //=== LOOP ===//
    for(int id = 0; id<(int)this->adjustROIRects.size(); id++){

        //--- 進行状況を表示 ---//
        QString status = "process : rigid...No." + QString::number(id);
        this->dialog->setLabelText(status);
        this->dialog->repaint();
        this->dialog->setValue(id);
        qApp->processEvents();

        //check "cancel"
        if(this->dialog->wasCanceled()){
            //release memory
            delete[] dest;

            //hide
            this->dialog->reset();
            //this->dialog->setVisible(false);

            //enable a button
            ui->pushButton_exeProgA->setEnabled(true);

            //return
            return;
        }


        //--- 処理対象のインデックスを取得 ---//
        int currentID = this->align_roi_id - id;

        //修正
        if(currentID < 0){
            int tmpid = -1*currentID;
            currentID = this->align_roi_id + tmpid;
        }


        //--- 処理対象画像の読み込み ---//
        std::string filename = this->adjustROIRects[currentID].imagename;
        std::string imagepath =this->roifolderpath_a + "/" + filename + ".tiff";
        cv::Mat img = cv::imread(imagepath ,1);
        if(img.empty()){
            QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! An image is not available for alignment: " + QString::fromStdString(imagepath));
            exit(-1);
        }


        //--- 画像をリサイズ ----------//
        if(id == 0){
            int roiw = img.cols;
            int roih = img.rows;
            factorx = factory = 1.0;
            if (roiw > 500 || roih > 500){
                float froiw = (float)roiw;
                float froih = (float)roih;
               while (froiw > 500 || froih > 500){
                    froiw /= 2.0;
                    froih /= 2.0;
                    factorx *= 2.0;
                    factory *= 2.0;
                }
            }
        }

        cv::Mat resizeimg;
        int nw = (int)((float)img.cols / factorx + 0.5);
        int nh = (int)((float)img.rows / factory + 0.5);
        cv::resize(img, resizeimg, cv::Size(nw, nh));
        qApp->processEvents();


        //--- ELASTIX実行用パラメータを画像サイズによって設定 ---//
        if (id == 0){

            //overtype
            const std::string *parameter = RIGID_PARAMETERS;
            std::string strp;
            std::vector<std::string> pcomps;
            while (*parameter != "END"){
                strp = *parameter;
                std::string::size_type idxn = strp.find("(NumberOfSpatialSamples");
                if (idxn != std::string::npos){
                    int nidx = (int)(std::floor)(std::log10((double)nw*nh / 10));
                    int num = (int)(std::floor((double)(nw*nh / 10) / std::pow(10, nidx)) * std::pow(10, nidx)) * 1;    //10% pixel data is used for calculation
                    if (num < 2000)         num = 2000;
                    else if (num > 10000)    num = 10000;
                    std::string nstr = "(NumberOfSpatialSamples " + std::to_string(num) + ")";
                    pcomps.push_back(nstr);
                }
                else{
                    pcomps.push_back(strp);
                }

                parameter++;

            }//while end

            parameterpath = this->roifolderpath_a + "/parameters_rigid3.txt";
            std::ofstream ofsp(parameterpath, ios::out);
            if (ofsp.fail()){
                exit(-1);
            }
            for (int i = 0; i < (int)pcomps.size(); i++){
                ofsp << pcomps[i] << std::endl;
            }
            ofsp.close();
        }


        //--- アライメントの基準画像または変形画像として保存 ---//
        //保存用パスを作成
        std::string savepath;
        if(id == 0){
            savepath = elastix_workfolderpath_a + "/fixed.tiff";
        }
        else{
            savepath = elastix_workfolderpath_a + "/moving.tiff";
        }

        //画像を保存
        bool img_saved = cv::imwrite(savepath, resizeimg);
        if(!img_saved){
            QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! Image save error before doing ELASTIX");
            exit(-1);
        }
        qApp->processEvents();


        //--- １枚目の画像はそのまま結果画像として保存、その後２枚目の処理へと移る---//
        if(id == 0){
            //save
            std::string resultsavepath = this->level_resultfolderpath_a + "/" + filename + ".tiff";
            cv::imwrite(resultsavepath, img);

            //次の解像度でのアライメント処理用に矩形情報を変化して保存
            if(this->adjust_target_level != 0){

                if(this->adjust_target_level == 2)
                {

                    //ROI情報の保存
                    roiRect roi;
                    roi.imagename = this->adjustROIRects[currentID].imagename;
                    roi.picNo = this->adjustROIRects[currentID].picNo;
                    roi.roi_rect.x = this->adjustROIRects[currentID].roi_rect.x * 4;
                    roi.roi_rect.y = this->adjustROIRects[currentID].roi_rect.y * 4;
                    roi.roi_rect.width = this->adjustROIRects[currentID].roi_rect.width * 4;
                    roi.roi_rect.height = this->adjustROIRects[currentID].roi_rect.height * 4;
                    roi.rotated = this->adjustROIRects[currentID].rotated;

                    upperROIRects[currentID] = roi;
                }
                else
                {
                    //スライス画像上での座標を取得
                    int nx = this->adjustROIRects[currentID].roi_rect.x;
                    int ny = this->adjustROIRects[currentID].roi_rect.y;

//                    //元画像での座標を計算
//                    cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
//                    origin.at<double>(0, 0) = (double)x;
//                    origin.at<double>(1, 0) = (double)y;
//                    origin.at<double>(2, 0) = 1.0;
//                    cv::Mat matrix = this->past_matricies[currentID];
//                    if(this->adjustROIRects[currentID].rotated)
//                    {
//                        matrix.at<double>(0,2) *= -1;
//                        matrix.at<double>(1,2) *= -1;
//                    }
//                    cv::Mat coord = matrix * origin;
//                    int nx = (int)((double)this->sliceROIRects[currentID].roi_rect.x + coord.at<double>(0, 0) + 0.5);
//                    int ny = (int)((double)this->sliceROIRects[currentID].roi_rect.y + coord.at<double>(0, 0) + 0.5);

                    //ROI情報の保存
                    roiRect roi;
                    roi.imagename = this->adjustROIRects[currentID].imagename;
                    roi.picNo = this->adjustROIRects[currentID].picNo;
                    roi.roi_rect.x = nx*4;//this->adjustROIRects[currentID].roi_rect.x * 4;
                    roi.roi_rect.y = ny*4;//this->adjustROIRects[currentID].roi_rect.y * 4;
                    roi.roi_rect.width = this->adjustROIRects[currentID].roi_rect.width * 4;
                    roi.roi_rect.height = this->adjustROIRects[currentID].roi_rect.height * 4;
                    roi.rotated = this->adjustROIRects[currentID].rotated;

                    upperROIRects[currentID] = roi;
                }

                //変換行列情報を保存
                cv::Mat firstmat = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
                if(this->adjust_target_level==1)
                {
                    firstmat.at<double>(0,0) = this->past_level0_matricies[currentID].at<double>(0, 0);
                    firstmat.at<double>(0,1) = this->past_level0_matricies[currentID].at<double>(0, 1);
                    firstmat.at<double>(0,2) = this->past_level0_matricies[currentID].at<double>(0, 2);
                    firstmat.at<double>(1,0) = this->past_level0_matricies[currentID].at<double>(1, 0);
                    firstmat.at<double>(1,1) = this->past_level0_matricies[currentID].at<double>(1, 1);
                    firstmat.at<double>(1,2) = this->past_level0_matricies[currentID].at<double>(1, 2);
//                    firstmat.at<double>(0,0) = this->past_matricies[currentID].at<double>(0, 0);
//                    firstmat.at<double>(0,1) = this->past_matricies[currentID].at<double>(0, 1);
//                    firstmat.at<double>(0,2) = this->past_matricies[currentID].at<double>(0, 2)*4;
//                    firstmat.at<double>(1,0) = this->past_matricies[currentID].at<double>(1, 0);
//                    firstmat.at<double>(1,1) = this->past_matricies[currentID].at<double>(1, 1);
//                    firstmat.at<double>(1,2) = this->past_matricies[currentID].at<double>(1, 2)*4;
                }

                upperROIRectMatrix[currentID]= firstmat;
                if(this->adjust_target_level == 2)
                    uupperROIRectMatrix[currentID] = firstmat;

            }

            //continue
            continue;
        }


        //--- ELASTIXをコマンドで呼び出し、アライメントを実行 ---/
        QString command = "elastix.exe -f " + QString::fromStdString(elastix_workfolderpath_a) + "/fixed.tiff -m " + QString::fromStdString(elastix_workfolderpath_a) + "/moving.tiff -out " + QString::fromStdString(elastix_outfolderpath_a) + " -p " + QString::fromStdString(parameterpath);
        QProcess::execute(command);
        cv::waitKey(3000);
        qApp->processEvents();


        //--- 変換行列を取得 ---//
        //ELASTIXのログを読み込み
        std::string transparameterpath = elastix_outfolderpath_a + "/TransformParameters.0.txt";
        int count = 0;
        bool open_checked = false;
        std::ifstream ifs;

        while(1){
            ifs.open(transparameterpath, ios::in);
            if(!ifs.fail()){
                open_checked = true;
                break;
            }

            //add counter
            count++;

            //break
            if(count>10000000000000)
                break;
        }

        //load check
        if(!open_checked){
            QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! An transform parameter is not available");
            //release
            delete[] dest;
            exit(-1);
        }

        //平行移動量、回転角度、回転中心座標の取得
        std::string str;
        double dx, dy, theta;
        double centerx, centery;
        bool transc = false, rotatec = false;
        while (std::getline(ifs, str)){

            //find a specific line
            std::string::size_type idx = str.find("(TransformParameters");
            std::string::size_type idxr = str.find("(CenterOfRotationPoint");

            if (idx != std::string::npos){
                //get parameters
                str.erase(0, sizeof("(TransformParameters"));
                str.pop_back();
                sscanf_s(str.c_str(), "%lf%lf%lf", &theta, &dx, &dy);
                transc = true;
            }
            else if(idxr != std::string::npos){
                str.erase(0, sizeof("(CenterOfRotationPoint"));
                str.pop_back();
                sscanf_s(str.c_str(), "%lf%lf", &centerx, &centery);
                rotatec = true;
            }
            if(transc && rotatec)
                break;
        }
        ifs.close();


        //---アライメント結果のELASTIX内での評価値（アライメントの確からしさ）を取得 ---------------------------------------------------//
        std::string logfilepath = elastix_outfolderpath_a + "/elastix.log";
        ifs.open(logfilepath, ios::in);
        if(!ifs.fail()){
            std::string str;
            while(std::getline(ifs, str)){
                //find a specific line
                std::string::size_type idx = str.find("Final metric value");
                //get
                if(idx != std::string::npos){
                    str.erase(0, sizeof("Final metric value = "));
                    metrics.push_back(std::atof(str.c_str()));
                }
            }
            ifs.close();
        }




        //---　次の処理のために今回の結果を削除 ---//
        //パラメータファイルを削除
        std::remove(transparameterpath.c_str());

        //結果画像を読み込み
        std::string elimgpath = elastix_outfolderpath_a + "/result.0.tiff";
        cv::Mat elimg = cv::imread(elimgpath, 0);

        //確認用に保存
        std::string elsavepath = elastix_originalfolderpath_a + "/" + filename + ".jpg";
        bool el_saved = cv::imwrite(elsavepath, elimg);
        if(!el_saved){
            QString error_str = "Elastix would be broken: " + QString::fromStdString(elsavepath) + " \n";
            error_str += "dx:" + QString::number(dx) + " dy:" + QString::number(dy) + " theta:" + QString::number(theta) + " cx:"
                         + QString::number(centerx) + " cy:" + QString::number(centery);
            QMessageBox::information(this, "CAUTION", error_str);
        }

        //結果画像を削除
        std::remove(elimgpath.c_str());

        qApp->processEvents();


        //---　剛体変換用行列の作成 ---//
        //平行移動、回転行列をそれぞれ作成
        cv::Mat transm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
        cv::Mat rotatem = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
        cv::Mat rotatemm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);

        transm.at<double>(0, 0) = cos(theta); transm.at<double>(0, 1) = -1 * sin(theta); transm.at<double>(0, 2) = dx*factorx;
        transm.at<double>(1, 0) = sin(theta); transm.at<double>(1, 1) = cos(theta); transm.at<double>(1, 2) = dy*factory;
        rotatem.at<double>(0, 2) = centerx*factorx; rotatem.at<double>(1, 2) = centery*factory;
        rotatemm.at<double>(0, 2) = -centerx*factorx; rotatemm.at<double>(1, 2) = -centery*factory;


        //変換行列の導出
        rigidm = rotatem * transm * rotatemm;


        //上下反転画像用の変換行列も作成
        transm.at<double>(0, 2) *= -1;
        transm.at<double>(1, 2) *= -1;
        rotate_rigidm = rotatem * transm * rotatemm;



        //---　SCNファイルよりピクセル情報を取得 ---//
        //open
        std::string scnpath = this->scnfolderpath_a + "/" + filename + ".scn";
        if(!openslide_can_open(scnpath.c_str())){
            QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! An scn file is not available: " + QString::fromStdString(scnpath));
            //release
            delete[] dest;
            exit(-1);
        }
        openslide_t *obj = openslide_open(scnpath.c_str());

        //解像度に合わせた倍率を設定
        int64_t factor;
        if(this->adjust_target_level == 2)      factor = 16;
        else if(this->adjust_target_level == 1) factor = 4;
        else                                    factor = 1;

        //座標計算
        int64_t CX, CY;
        if(this->adjust_target_level == 2)
        {
            CX = this->adjustROIRects[currentID].roi_rect.x + width/2;
            CY = this->adjustROIRects[currentID].roi_rect.y + height/2;
        }
        else if(this->adjust_target_level == 1){
            CX = this->sliceROIRects[currentID].roi_rect.x + width/2;//this->adjustROIRects[currentID].roi_rect.x + width/2;
            CY = this->sliceROIRects[currentID].roi_rect.y + height/2;//this->adjustROIRects[currentID].roi_rect.y + height/2;
        }
        else{
            CX = this->uppersliceROIRects[currentID].roi_rect.x + width/2;//this->adjustROIRects[currentID].roi_rect.x + width/2;
            CY = this->uppersliceROIRects[currentID].roi_rect.y + height/2;//this->adjustROIRects[currentID].roi_rect.y + height/2;
        }
        int64_t WX = (CX - workw / 2)*factor;   if(WX<0)    WX = 0;
        int64_t WY = (CY - workh / 2)*factor;   if(WY<0)    WY = 0;

        //倍率により座標を修正
        adjx = WX / factor;
        adjy = WY / factor;

        //画素情報を取得
        openslide_read_region(obj, dest, WX, WY, this->adjust_target_level, workw, workh);

        //１つ上の解像度の画素情報を取得
        openslide_get_level_dimensions(obj, this->adjust_target_level-1, &upper_width, &upper_height);


        //close
        openslide_close(obj);


        //--- 結果画像の作成 ---//
        int roiw = this->adjustROIRects[currentID].roi_rect.width;
        int roih = this->adjustROIRects[currentID].roi_rect.height;
        cv::Mat resultimg = cv::Mat(cv::Size(roiw, roih), CV_8UC3, cv::Scalar::all(0));

        //画像の連続性を考慮した変換行列の設定
        cv::Mat past_rigidm, current_rigidm;
        if(this->adjustROIRects[currentID].rotated){
            past_rigidm = rotate_prigidm.clone();
            current_rigidm = rotate_rigidm.clone();
        }
        else{
            past_rigidm = prigidm.clone();
            current_rigidm = rigidm.clone();
        }

        //現在の画像にかかっている行列を取得
        cv::Mat prev_rigidm;// = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
        if(this->adjust_target_level != 2)
        {
            if(this->adjust_target_level == 1)
                prev_rigidm = this->past_level1_matricies[currentID].clone();
            else
                prev_rigidm = this->past_matricies[currentID].clone();
            if(this->adjustROIRects[currentID].rotated)
            {
                prev_rigidm.at<double>(0,2) *= -1;
                prev_rigidm.at<double>(1,2) *= -1;
            }
        }

        if(this->adjust_target_level == 1)
        {

            //修正用座標の設定
            double LeftUpperX, LeftUpperY;
            LeftUpperX = this->sliceROIRects[currentID].roi_rect.x;
            LeftUpperY = this->sliceROIRects[currentID].roi_rect.y;

#ifdef _OPENMP
#pragma omp parallel for private(x)
#endif
       for(int y=this->adjustROIRects[currentID].roi_rect.y; y<(this->adjustROIRects[currentID].roi_rect.y+roih); y++){
           for(int x=this->adjustROIRects[currentID].roi_rect.x; x<(this->adjustROIRects[currentID].roi_rect.x+roiw); x++){

               //新しい座標を計算
               cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
               origin.at<double>(0, 0) = (double)x;
               origin.at<double>(1, 0) = (double)y;
               origin.at<double>(2, 0) = 1.0;

               cv::Mat coord = past_rigidm * current_rigidm * prev_rigidm * origin;

               //画素情報を取得するために計算した座標を修正
               bool out_of_range = false;
               int nx = (int)(LeftUpperX + coord.at<double>(0, 0) + 0.5) - adjx;//((double)this->adjustROIRects[currentID].roi_rect.x + coord.at<double>(0, 0) + 0.5) - adjx;
               int ny = (int)(LeftUpperY + coord.at<double>(1, 0) + 0.5) - adjy;//((double)this->adjustROIRects[currentID].roi_rect.y + coord.at<double>(1, 0) + 0.5) - adjy;
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
               cv::Vec3b *ptr = resultimg.ptr<cv::Vec3b>(y-this->adjustROIRects[currentID].roi_rect.y);
               ptr[x-this->adjustROIRects[currentID].roi_rect.x] = cv::Vec3b(b, g, r);

               }
           }

        }
        else if(this->adjust_target_level == 0)
        {

            //修正用座標の設定
            double LeftUpperX, LeftUpperY;
            LeftUpperX = this->uppersliceROIRects[currentID].roi_rect.x;
            LeftUpperY = this->uppersliceROIRects[currentID].roi_rect.y;

#ifdef _OPENMP
#pragma omp parallel for private(x)
#endif
       for(int y=this->adjustROIRects[currentID].roi_rect.y; y<(this->adjustROIRects[currentID].roi_rect.y+roih); y++){
           for(int x=this->adjustROIRects[currentID].roi_rect.x; x<(this->adjustROIRects[currentID].roi_rect.x+roiw); x++){

               //新しい座標を計算
               cv::Mat origin = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
               origin.at<double>(0, 0) = (double)x + (double)this->sliceROIRects[currentID].roi_rect.x;
               origin.at<double>(1, 0) = (double)y + (double)this->sliceROIRects[currentID].roi_rect.y;
               origin.at<double>(2, 0) = 1.0;

               cv::Mat coord = past_rigidm * current_rigidm * prev_rigidm * origin;

               //画素情報を取得するために計算した座標を修正
               bool out_of_range = false;
               int nx = (int)(LeftUpperX + coord.at<double>(0, 0) + 0.5) - adjx;//((double)this->adjustROIRects[currentID].roi_rect.x + coord.at<double>(0, 0) + 0.5) - adjx;
               int ny = (int)(LeftUpperY + coord.at<double>(1, 0) + 0.5) - adjy;//((double)this->adjustROIRects[currentID].roi_rect.y + coord.at<double>(1, 0) + 0.5) - adjy;
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
               cv::Vec3b *ptr = resultimg.ptr<cv::Vec3b>(y-this->adjustROIRects[currentID].roi_rect.y);
               ptr[x-this->adjustROIRects[currentID].roi_rect.x] = cv::Vec3b(b, g, r);

               }
           }

        }
        else{

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

           cv::Mat coord = past_rigidm * current_rigidm * origin;

           //画素情報を取得するために計算した座標を修正
           bool out_of_range = false;
           int nx = (int)((double)this->adjustROIRects[currentID].roi_rect.x + coord.at<double>(0, 0) + 0.5) - adjx;
           int ny = (int)((double)this->adjustROIRects[currentID].roi_rect.y + coord.at<double>(1, 0) + 0.5) - adjy;
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
        }
       qApp->processEvents();


       //--- 結果画像の保存 ---//
       //上下反転
       if(this->adjustROIRects[currentID].rotated){
           cv::Mat tmpimg = resultimg.clone();
           resultimg.setTo(0);
           cv::flip(tmpimg, resultimg, -1);
           tmpimg.release();
       }

       //save
       std::string resultsavepath = this->level_resultfolderpath_a + "/" + filename + ".tiff";
       cv::imwrite(resultsavepath, resultimg);
       qApp->processEvents();


       //--- 現在の画像を次のアライメントの基準画像として保存 ---//
       std::string fixedsavepath = elastix_workfolderpath_a + "/fixed.tiff";
       if(currentID != 0){
           cv::imwrite(fixedsavepath, resizeimg);
       }
       else{    //if currentID == 0, then save "align_roi_id" image as a fixed image
           //load
           std::string roiimagepath = this->roifolderpath_a + "/" + this->adjustROIRects[this->align_roi_id].imagename + ".tiff";
           cv::Mat roiimg = cv::imread(roiimagepath, 1);

           //resize
           cv::Mat roiresize;
           cv::resize(roiimg, roiresize, cv::Size(nw, nh));

           //save
           cv::imwrite(fixedsavepath, roiresize);
       }
       qApp->processEvents();


       //--- 現在の変換行列を次のアライメントのために保存 ---//
       if(currentID != 0){
           prigidm *= rigidm;
           rotate_prigidm *= rotate_rigidm;
       }
       else{

           //reset prigidm
           prigidm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           rotate_prigidm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
       }


       //--- 現在注目している解像度の１つ上の解像度における作業用ROIを作成 ---//
       if(this->adjust_target_level == 1){


           //平行移動、回転行列をそれぞれ作成
           cv::Mat utransm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           cv::Mat urotatem = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           cv::Mat urotatemm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);

           utransm.at<double>(0, 0) = cos(theta); utransm.at<double>(0, 1) = -1 * sin(theta); utransm.at<double>(0, 2) = dx*factorx*4;
           utransm.at<double>(1, 0) = sin(theta); utransm.at<double>(1, 1) = cos(theta); utransm.at<double>(1, 2) = dy*factory*4;
           urotatem.at<double>(0, 2) = centerx*factorx*4; urotatem.at<double>(1, 2) = centery*factory*4;
           urotatemm.at<double>(0, 2) = -centerx*factorx*4; urotatemm.at<double>(1, 2) = -centery*factory*4;


           //変換行列の導出
           urigidm = urotatem * utransm * urotatemm;


           //上下反転画像用の変換行列も作成
           utransm.at<double>(0, 2) *= -1;
           utransm.at<double>(1, 2) *= -1;
           rotate_urigidm = urotatem * utransm * urotatemm;

           //現在の画像に元々かかっている変換行列を１つ上の解像度用に変形
           cv::Mat prev_upper_rigidm = this->past_level0_matricies[currentID];
//           cv::Mat prev_upper_rigidm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
//               prev_upper_rigidm.at<double>(0,0) = this->past_matricies[currentID].at<double>(0, 0);
//               prev_upper_rigidm.at<double>(0,1) = this->past_matricies[currentID].at<double>(0, 1);
//               prev_upper_rigidm.at<double>(0,2) = this->past_matricies[currentID].at<double>(0, 2);
//               prev_upper_rigidm.at<double>(1,0) = this->past_matricies[currentID].at<double>(1, 0);
//               prev_upper_rigidm.at<double>(1,1) = this->past_matricies[currentID].at<double>(1, 1);
//               prev_upper_rigidm.at<double>(1,2) = this->past_matricies[currentID].at<double>(1, 2);


           //画像の連続性を考慮した変換行列の設定
           cv::Mat past_urigidm, current_urigidm;
           if(this->adjustROIRects[currentID].rotated){
               past_urigidm = rotate_purigidm.clone();
               current_urigidm = rotate_urigidm.clone();
               prev_upper_rigidm.at<double>(0,2)*= -1;
               prev_upper_rigidm.at<double>(1,2)*= -1;
           }
           else{
               past_urigidm = purigidm.clone();
               current_urigidm = urigidm.clone();
           }

           cv::Mat upper_rigidm = past_urigidm * current_urigidm * prev_upper_rigidm;


           //スライス画像上での座標を取得
           int nx = this->adjustROIRects[currentID].roi_rect.x;
           int ny = this->adjustROIRects[currentID].roi_rect.y;


           //保存
           roiRect uroi;
           uroi.imagename = this->adjustROIRects[currentID].imagename;
           uroi.picNo = this->adjustROIRects[currentID].picNo;
           uroi.rotated = this->adjustROIRects[currentID].rotated;
           uroi.roi_rect.x = nx*4;//this->adjustROIRects[currentID].roi_rect.x * 4;
           uroi.roi_rect.y = ny*4;//this->adjustROIRects[currentID].roi_rect.y * 4;
           uroi.roi_rect.width = this->adjustROIRects[currentID].roi_rect.width * 4;
           uroi.roi_rect.height = this->adjustROIRects[currentID].roi_rect.height * 4;

           upperROIRects[currentID] = uroi;
           upperROIRectMatrix[currentID] = upper_rigidm;


           //save matrix
           if(currentID != 0){
               purigidm *= urigidm;
               rotate_purigidm *= rotate_urigidm;
           }
           else{
               purigidm =  cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
               rotate_purigidm =  cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
           }
        }
       else if(this->adjust_target_level == 2){


           //平行移動、回転行列をそれぞれ作成
           cv::Mat utransm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           cv::Mat urotatem = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           cv::Mat urotatemm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           cv::Mat uutransm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           cv::Mat uurotatem = cv::Mat::eye(cv::Size(3,3), CV_64FC1);
           cv::Mat uurotatemm = cv::Mat::eye(cv::Size(3,3), CV_64FC1);

           utransm.at<double>(0, 0) = cos(theta); utransm.at<double>(0, 1) = -1 * sin(theta); utransm.at<double>(0, 2) = dx*factorx*4;
           utransm.at<double>(1, 0) = sin(theta); utransm.at<double>(1, 1) = cos(theta); utransm.at<double>(1, 2) = dy*factory*4;
           urotatem.at<double>(0, 2) = centerx*factorx*4; urotatem.at<double>(1, 2) = centery*factory*4;
           urotatemm.at<double>(0, 2) = -centerx*factorx*4; urotatemm.at<double>(1, 2) = -centery*factory*4;

           uutransm.at<double>(0, 0) = cos(theta); uutransm.at<double>(0, 1) = -1 * sin(theta); uutransm.at<double>(0, 2) = dx*factorx*16;
           uutransm.at<double>(1, 0) = sin(theta); uutransm.at<double>(1, 1) = cos(theta); uutransm.at<double>(1, 2) = dy*factory*16;
           uurotatem.at<double>(0, 2) = centerx*factorx*16; uurotatem.at<double>(1, 2) = centery*factory*16;
           uurotatemm.at<double>(0, 2) = -centerx*factorx*16; uurotatemm.at<double>(1, 2) = -centery*factory*16;


           //変換行列の導出
           urigidm = urotatem * utransm * urotatemm;
           uurigidm = uurotatem * uutransm * uurotatemm;


           //上下反転画像用の変換行列も作成
           utransm.at<double>(0, 2) *= -1;
           utransm.at<double>(1, 2) *= -1;
           rotate_urigidm = urotatem * utransm * urotatemm;

           uutransm.at<double>(0, 2) *= -1;
           uutransm.at<double>(1, 2) *= -1;
           rotate_uurigidm = uurotatem * uutransm * uurotatemm;


           //画像の連続性を考慮した変換行列の設定
           cv::Mat past_urigidm, current_urigidm;
           cv::Mat past_uurigidm, current_uurigidm;
           if(this->adjustROIRects[currentID].rotated){
               past_urigidm = rotate_purigidm.clone();
               current_urigidm = rotate_urigidm.clone();
               past_uurigidm = rotate_puurigidm.clone();
               current_uurigidm = rotate_uurigidm.clone();
           }
           else{
               past_urigidm = purigidm.clone();
               current_urigidm = urigidm.clone();
               past_uurigidm = puurigidm.clone();
               current_uurigidm = uurigidm.clone();
           }

           cv::Mat upper_rigidm = past_urigidm * current_urigidm;
           cv::Mat uupper_rigidm = past_uurigidm * current_uurigidm;


           //スライス画像上での座標を取得
           int nx = this->adjustROIRects[currentID].roi_rect.x;
           int ny = this->adjustROIRects[currentID].roi_rect.y;


           //保存
           roiRect uroi;
           uroi.imagename = this->adjustROIRects[currentID].imagename;
           uroi.picNo = this->adjustROIRects[currentID].picNo;
           uroi.rotated = this->adjustROIRects[currentID].rotated;
           uroi.roi_rect.x = nx*4;//this->adjustROIRects[currentID].roi_rect.x * 4;
           uroi.roi_rect.y = ny*4;//this->adjustROIRects[currentID].roi_rect.y * 4;
           uroi.roi_rect.width = this->adjustROIRects[currentID].roi_rect.width * 4;
           uroi.roi_rect.height = this->adjustROIRects[currentID].roi_rect.height * 4;

           upperROIRects[currentID] = uroi;
           upperROIRectMatrix[currentID] = upper_rigidm;
           uupperROIRectMatrix[currentID] = uupper_rigidm;

           //save matrix
           if(currentID != 0){
               purigidm *= urigidm;
               rotate_purigidm *= rotate_urigidm;
               puurigidm *= uurigidm;
               rotate_puurigidm *= rotate_uurigidm;
           }
           else{
               purigidm =  cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
               rotate_purigidm =  cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
               puurigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
               rotate_puurigidm = cv::Mat::eye(cv::Size(3, 3), CV_64FC1);
           }
        }

    }//loop end

    //hide dialog
    this->dialog->setValue((int)this->adjustROIRects.size() - 1);
    qApp->processEvents();
    this->dialog->hide();
    qApp->processEvents();

    //---　画素情報配列を開放 ----------------------------------------------//
    delete[] dest;


    //--- すべてのROI画像を削除 -------------------------------------------//
    for(int i=0; i<(int)this->adjustROIRects.size(); i++){

        //get a filename
        std::string filename = this->adjustROIRects[i].imagename;

        //make a path
        std::string roiimagepath = this->roifolderpath_a + "/" + filename + ".tiff";

        //remove
        std::remove(roiimagepath.c_str());
    }


    //--- １つ上の解像度における作業用ROI情報を保存 ------------//
    if(this->adjust_target_level != 0){
        //open
        std::string uslicepath = this->infresultfolderpath_a + "/roiInf_level" + std::to_string(this->adjust_target_level-1) + "_slice.tsv";
        std::ofstream ofs(uslicepath, ios::out);
        if(ofs.fail()){
            uslicepath = this->inffolderpath_a + "/roiInf_level" +  std::to_string(this->adjust_target_level-1) + "_slice.tsv";
            ofs.open(uslicepath, ios::out);

            if(ofs.fail()){
                QMessageBox::information(this, "FATAL ERROR", "!! FATAL !! An tsv file can't be written" );
                exit(-1);
            }
        }

        //write
        for(int i=0; i<(int)upperROIRects.size(); i++){
            roiRect uroi = upperROIRects[i];
            cv::Mat upper_rigidm = upperROIRectMatrix[i];
            cv::Mat uupper_rigidm = uupperROIRectMatrix[i];

            //矩形情報の書き込み
            ofs << uroi.picNo << "\t" << uroi.imagename << "\t" << uroi.roi_rect.x << "\t" << uroi.roi_rect.y << "\t" << uroi.roi_rect.width << "\t" << uroi.roi_rect.height << "\t" << uroi.rotated << "\t";

            //levelの場合のみsliceroirects情報を記載
            if(this->adjust_target_level == 1)
            {
                ofs << this->sliceROIRects[i].roi_rect.x*4 << "\t" << this->sliceROIRects[i].roi_rect.y*4 << "\t" << this->sliceROIRects[i].roi_rect.width*4 << "\t" << this->sliceROIRects[i].roi_rect.height*4 << "\t";
            }
            else
            {
                ofs << uupper_rigidm.at<double>(0, 0) << "\t" <<  uupper_rigidm.at<double>(0, 1) << "\t" << uupper_rigidm.at<double>(0, 2) << "\t"
                    << uupper_rigidm.at<double>(1, 0) << "\t" << uupper_rigidm.at<double>(1, 1) << "\t" << uupper_rigidm.at<double>(1, 2) << "\t"
                    << uupper_rigidm.at<double>(2, 0) << "\t" << uupper_rigidm.at<double>(2, 1) << "\t" << uupper_rigidm.at<double>(2, 2) << "\t";
            }

            //変換行列情報の書き込み
            ofs << upper_rigidm.at<double>(0, 0) << "\t" <<  upper_rigidm.at<double>(0, 1) << "\t" << upper_rigidm.at<double>(0, 2) << "\t"
                << upper_rigidm.at<double>(1, 0) << "\t" << upper_rigidm.at<double>(1, 1) << "\t" << upper_rigidm.at<double>(1, 2) << "\t"
                << upper_rigidm.at<double>(2, 0) << "\t" << upper_rigidm.at<double>(2, 1) << "\t" << upper_rigidm.at<double>(2, 2) << std::endl;


        }


        //close
        ofs.close();
    }


    //--- FOR CHECK: ELASTIXでの評価値を出力 ----------------------------------//
    std::string metricfilepath = this->infresultfolderpath_a + "metric_level" + std::to_string(this->adjust_target_level) + ".csv";
    std::ofstream ofs(metricfilepath, ios::out);
    if(!ofs.fail()){
        for(int i=1; i<(int)metrics.size(); i++){
            std::string filename = this->adjustROIRects[i].imagename;
            ofs << filename << "," << metrics[i] << std::endl;
        }
        ofs.close();
    }


    //---　ボタンを有効化 --------------------------------------------------//
    ui->pushButton_continueProgA->setEnabled(true);
    ui->pushButton_quitProgA->setEnabled(true);


}

