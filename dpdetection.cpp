
/***********************
 * Automatic detection using DP
 * *********************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>

#include <opencv2/opencv.hpp>
#include <openslide.h>

//===================
// Grobal variables
//===================
std::string ERROR_STR;


//=========================
// SUBROUTINES : Set workranges
//========================
void setWorkRange(std::string imagefolderpath, std::string imageextension, std::vector<std::string> imagelist, roiRect firstroi, roiRect lastroi, std::vector<cv::Rect>& workRanges)
{

    int roiW = firstroi.roi_rect.width;
    int roiH = firstroi.roi_rect.height;
    int fx = firstroi.roi_rect.x;
    int fy = firstroi.roi_rect.y;
    int lx = lastroi.roi_rect.x;
    int ly = lastroi.roi_rect.y;

    int minx = (fx < lx) ? fx : lx;
    int maxx = (fx + roiW > lx + roiW) ? fx + roiW : lx + roiW;
    int miny = (fy < ly) ? fy : ly;
    int maxy = (fy + roiH > ly + roiH) ? fy + roiH : ly + roiH;



    //=== LOOP ===//
    int startx, stopx, starty, stopy;
    for (int id = 0; id < (int)imagelist.size(); id++){

        //load an image
        std::string imagepath = imagefolderpath + "/" + imagelist[id] + imageextension;
        cv::Mat img = cv::imread(imagepath, 1);
        if(img.empty()){
            exit(-1);
        }

        //if minx and miny are out of inputimage's range, then return values will be -1
        if(minx > img.cols || miny > img.rows){
            startx = stopx = starty = stopy = -1;
        }
        else{   //otherwise, calculate coordinates of a workrange

            //set a ROI
            startx = minx - roiW / 2;				if(startx < 0)              startx = 0;
            stopx = maxx + roiW / 2;				if (stopx > img.cols)		stopx = img.cols;
            starty = miny - roiH;					if (starty < 0)				starty = 0;
            stopy = maxy + roiH;					if (stopy > img.rows)       stopy = img.rows;

            //If the area of set ROI smaller than default ROI size, then return values will be -1
            int area = (stopx - startx) * (stopy - starty);
            if(area < roiW*roiH){
                startx = stopx = starty = stopy = -1;
            }
        }

        //set
        cv::Rect roirect = cv::Rect(cv::Point(startx, starty), cv::Point(stopx, stopy));

        //save a work range
        workRanges.push_back(roirect);

    }//id loop

}



//=================================
// SUBROUTINES: Check keypoints whose radius is over the threshold
//=================================
void threshKeyPoints(std::vector<cv::KeyPoint> kp, cv::Mat desc, std::vector<cv::KeyPoint>& nkp, cv::Mat& ndesc, float kpThreshold)
{
    //=== LOOP ===//
    float min = 10000000.0;
    float max = 0.0;
    for (int id = 0; id < kp.size(); id++){
        //check a keypoint radius
        float radius = kp[id].size;

        //save kp & desc if the keypoint radius is lager than threshold
        if (radius - kpThreshold > 1E-10){
            nkp.push_back(kp[id]);
            ndesc.push_back(desc.row(id));

            //find min&max
            if (radius - min < 1E-10)	min = radius;
            if (radius - max > 1E-10)	max = radius;
        }
    }//id end

}



//=====================================
// SUBROUTINES: Normalize keypoint distances
//=====================================
void normalizeDistance(std::vector<cv::DMatch> matches, std::vector<float>& matchedDis, std::vector<float>& normalizedDis)
{

    std::vector<cv::DMatch>::iterator itM = matches.begin();

    //store
    float  minDis = 10000.0;
    float maxDis = 0.0;
    while (itM != matches.end()){
        float dis = (*itM).distance;
        if (dis - maxDis > 1E-10)	maxDis = dis;
        if (dis - minDis < 1E-10)	minDis = dis;
        matchedDis.push_back(dis);
        ++itM;
    }

    //normalization
    //% Distance range will be changed into 0 to 1.
    //% About the distance values, 1 will be the best evaluation and 0 will be the worst.
    for (int i = 0; i < (int)matchedDis.size(); i++){
        normalizedDis.push_back(1.0 - (matchedDis[i] - minDis) * (1.0 / (maxDis - minDis)));
    }

}




//===============================
// SUBROUTINES: Set new matching points according to the normalized distances
//=================================
void countNewmatchpoint(std::vector<cv::KeyPoint> ckp, std::vector<cv::KeyPoint> pkp, std::vector<cv::Point2f>& nckp, std::vector<cv::Point2f>& npkp, std::vector<cv::DMatch> matches, std::vector<cv::DMatch>& nmatches, std::vector<float> normalizedDis, float thresholdLower, float thresholdUpper)
{
    float disThresholdU = thresholdUpper;
    float disThresholdL = thresholdLower;

    for (int i = 0; i < (int)normalizedDis.size(); i++){
        float dis = normalizedDis[i];
        if (dis - disThresholdU < 1E-10 && dis - disThresholdL > 1E-10){
            int cid = matches[i].queryIdx;
            int pid = matches[i].trainIdx;
            nckp.push_back(ckp[cid].pt);
            npkp.push_back(pkp[pid].pt);
            //for check
            //cv::DMatch match(cid, pid, dis);
            cv::DMatch match(cid, pid, matches[i].distance);
            nmatches.push_back(match);
        }
    }

}



//=================================
// SUBROUTINES: Get keypoint's IDs inside a specific region
//=====================================
void getPointID(std::vector<cv::Point2f> kp, std::vector<int>& kpIDs, cv::Rect roi, int minx, int miny)
{
    for (int i = 0; i < (int)kp.size(); i++){
        if (kp[i].inside(cv::Rect(roi.x - minx, roi.y - miny, roi.width, roi.height)))
            kpIDs.push_back(i);
    }
}



//======================================
// SUBROUTINES: Get same IDs inside specific regions
//=======================================
void getSameID(std::vector<int> ckpIDs, std::vector<int> pkpIDs, std::vector<int>& sameIDs)
{
    //store same IDs
    for (int i = 0; i < (int)ckpIDs.size(); i++){
        int cID = ckpIDs[i];
        for (int j = 0; j < (int)pkpIDs.size(); j++){
            if (cID == pkpIDs[j]){
                sameIDs.push_back(cID);
                break;
            }
        }//j loop end
    }//i loop end

}




//======================================
// SUBROUITNES: Count the number of keypoint which the distance between two keypoints is lower the threshold
//========================================
float countKeypointDistance(std::vector<cv::Point2f> rckp, std::vector<cv::Point2f> rpkp, std::vector<int> sameIDs, int croix, int croiy, int proix, int proiy, float threshDis)
{

    //calculate similarity[p][c]
    float sim = 0;
    for (int id = 0; id < (int)sameIDs.size(); id++){

        //get an ID
        int kpID = sameIDs[id];

        //get coordinates
        float cx = rckp[kpID].x - (float)croix;
        float cy = rckp[kpID].y - (float)croiy;
        float px = rpkp[kpID].x - (float)proix;
        float py = rpkp[kpID].y - (float)proiy;

//        if (cx < 0 || cy < 0 || px < 0 || py < 0){
//            std::cout << cx << " " << cy << " " << px << " " << py << endl;
//            getchar();
//        }


        //calculate a distance between two points
        float dis = sqrt((cx - px) * (cx - px) + (cy - py) * (cy - py));


        //calculate the similarity
        if (dis - threshDis < 1E-10)
            sim++;

    }//id loop end

    return sim;

}



//================================
//  Main of DP alignment
//================================
bool MainWindow::alignmentDP0722(roiRect Firstroi_rect, roiRect Lastroi_rect, std::vector<std::string> imagelist, std::vector<int> useID, std::vector<cv::Rect> workranges, std::vector<roiRect>& roipath, float hessianThreshold, float kpThreshold)
{
    //Variables
    int count_roirects;
    int startpic = 0;
    int stoppic = (int)imagelist.size() - 1;
    int usedid;
    int roiW = Firstroi_rect.roi_rect.width;
    int roiH = Firstroi_rect.roi_rect.height;
    int fminx, fmaxx, fminy, fmaxy;
    int lminx, lmaxx, lminy, lmaxy;
    int cminx, cmaxx, cminy, cmaxy;
    int pminx, pmaxx, pminy, pmaxy;
    int froix, froiy, lroix, lroiy;

    std::vector<std::vector<float> >	similarity;

    bool								firstCalcSimCheck = false;

    std::string imagepath;

    //Variables of OPENCV
    cv::Mat pworkimg, cworkimg, fworkimg, lworkimg;
    std::vector<std::vector<int> >			DPPairs;									//DP Pair information
    cv::Mat								cSim, pSim;								//similarity information

    cv::SURF	surf(hessianThreshold, 4, 2, false);
    std::vector<cv::KeyPoint>	fkp, lkp;
    cv::Mat fdesc, ldesc;

    cv::BFMatcher	matcher(cv::NORM_L2, true);


    //Variables of roiRect
    std::vector<std::vector<roiRect> > picROIrects;

    //Parameters
    float disThresholdU = 1.0;
    float disThresholdL = 0.0;
    float	epsilonDis = roiH * 0.2;			//2014/07/01 FOR "ELASTIX" CHECK
    int cut_interval = roiW / 10 - 5;		    //cut interval


    //=== PIC LOOP ===//
    count_roirects = 0;

    //for skip process
    this->skipimagepathes.clear();
    this->skipimages.clear();

    for (int pic = startpic; pic <= stoppic; pic++){


        //--- show the status -------------------------------------//
        QString status = "process : auto detection...No." + QString::number(this->auto_detection_counter);
        this->dialog->setLabelText(status);
        this->dialog->setValue(this->auto_detection_counter);
        qApp->processEvents();
        this->auto_detection_counter++;

        //check "cancel"
        if(this->dialog->wasCanceled()){
            this->auto_detection_canceled = true;
            ERROR_STR = "Automatic detection has been canceled";
            return false;
        }


        //--- load an image ---------------------------------------//
        //create a path
        imagepath = this->workfolderpath_d + "/" + imagelist[pic] + ".tiff";
        usedid = useID[pic];
        cv::Rect work = workranges[pic];

        //load
        cv::Mat img = cv::imread(imagepath, 0);

        //skip case1: No image
        if (img.empty()){
            //continue process
            continue;
        }

        //skip case2: A workrange doesn't exist
        if(work.x == -1 || work.y == -1 || work.width*work.height == 0){
            //save images
            this->skipimagepathes.push_back(imagelist[pic]);
            this->skipimages.push_back(img);
            //save skipped image information as a roiRect data
            roiRect roi;
            roi.imagename = imagelist[pic];
            roi.picNo = useID[pic];
            roi.rotated = false;
            this->skippedRects.push_back(roi);
            //continue process
            continue;
        }

        //skip case3: There is not an area which has a same size as the ROI
        cv::Mat workarea = img(work);
        cv::Mat workbin;
        cv::threshold(workarea, workbin, 0, 255, CV_THRESH_BINARY);
        int warea = cv::countNonZero(workbin);
        if(warea < roiW*roiH){
            //save images
            this->skipimagepathes.push_back(imagelist[pic]);
            this->skipimages.push_back(img);
            //save skipped image information as a roiRect data
            roiRect roi;
            roi.imagename = imagelist[pic];
            roi.picNo = useID[pic];
            roi.rotated = false;
            this->skippedRects.push_back(roi);
            //continue process
            continue;
        }



        //blur
        cv::GaussianBlur(img, img, cv::Size(5, 5), 1.0);



        //--- save "roiRect" information a --------------------------//
        std::vector<roiRect> picROIRect;
        if (pic == startpic){

            //set a work range
            pworkimg = cv::Mat(img, work);

            //save coordinate
            pminx = work.x;
            pminy = work.y;
            pmaxx = work.x + work.width - 1;
            pmaxy = work.y + work.height - 1;

            //save coordinate for first
            fminx = work.x;
            fminy = work.y;
            fmaxx = work.x + work.width - 1;
            fmaxy = work.y + work.height - 1;

            //get froi's left-upper coordinate
            froix = Firstroi_rect.roi_rect.x - fminx;
            froiy = Firstroi_rect.roi_rect.y - fminy;

            //save ROI information
            Firstroi_rect.imagename = imagelist[pic];
            picROIRect.push_back(Firstroi_rect);
            picROIrects.push_back(picROIRect);
            count_roirects++;

            //clear
            std::vector<roiRect>().swap(picROIRect);

            //SURF
            std::vector<cv::KeyPoint> kp;
            cv::Mat desc;
            surf(pworkimg, cv::Mat(), kp, desc);
            threshKeyPoints(kp, desc, fkp, fdesc, kpThreshold);
            fworkimg = pworkimg.clone();

            //for lastROI
            std::string limagepath = this->workfolderpath_d + "/" + imagelist[stoppic] + ".tiff";
            cv::Mat limg = cv::imread(limagepath, 0);
            if(limg.empty()){
                ERROR_STR = "An image is not available : " + limagepath;
                return false;
            }


            cv::GaussianBlur(limg, limg, cv::Size(5, 5), 1.0);
            cv::Rect lwork = workranges[stoppic];
            lworkimg = cv::Mat(limg, lwork);

            //SURF
            kp.clear(); desc.release();
            surf(lworkimg, cv::Mat(), kp, desc);
            threshKeyPoints(kp, desc, lkp, ldesc, kpThreshold);
            limg.release();
            //lworkimg.release();

            //save coordinate for last
            lminx = lwork.x;
            lminy = lwork.y;
            lmaxx = lwork.x + lwork.width - 1;
            lmaxy = lwork.y + lwork.height - 1;

            //get froi's left-upper coordinate
            lroix = Lastroi_rect.roi_rect.x - lminx;
            lroiy = Lastroi_rect.roi_rect.y - lminy;

            continue;
        }
        else if (pic == stoppic){

            //set a work range
            cworkimg = cv::Mat(img, work);

            //save coordinate
            cminx = work.x;
            cminy = work.y;
            cmaxx = work.x + work.width - 1;
            cmaxy = work.y + work.height - 1;

            //save ROI information
            Lastroi_rect.imagename = imagelist[pic];
            picROIRect.push_back(Lastroi_rect);
            picROIrects.push_back(picROIRect);
            count_roirects++;

            //clear
            std::vector<roiRect>().swap(picROIRect);

        }
        else{

            //set a work range
            cworkimg = cv::Mat(img, work);

            //save coordinate
            cminx = work.x;
            cminy = work.y;
            cmaxx = work.x + work.width - 1;
            cmaxy = work.y + work.height - 1;


            //cut ROIs
            int startx = work.x;
            int starty = work.y;
            int stopx = startx + work.width - 1;
            int stopy = starty + work.height - 1;

            for (int y = starty; y <= stopy; y += cut_interval){
                if (y + roiH > stopy) break;

                for (int x = startx; x <= stopx; x += cut_interval){
                    if (x + roiW > stopx)	break;

                    //cut a ROI
                    roiRect roi;
                    roi.imagename = imagelist[pic];
                    roi.picNo = usedid;
                    roi.roi_rect = cv::Rect(x, y, roiW, roiH);
                    roi.rotated = false;
                    picROIRect.push_back(roi);

                }//x loop end
            }//y loop end

            //save ROI information
            picROIrects.push_back(picROIRect);
            count_roirects++;

            //clear
            std::vector<roiRect>().swap(picROIRect);

        }

        if (count_roirects < 2)	continue;
        qApp->processEvents();


        //--- SURF matching ---------------------------------------------------------------//
        std::vector<cv::KeyPoint>	kp, ckp, pkp;
        cv::Mat desc, cdesc, pdesc;

        //extract keypoints from current image
        surf(cworkimg, cv::Mat(), kp, desc);
        threshKeyPoints(kp, desc, ckp, cdesc, kpThreshold);

        //extract keypoitns from past image
        kp.clear(); desc.release();
        surf(pworkimg, cv::Mat(), kp, desc);
        threshKeyPoints(kp, desc, pkp, pdesc, kpThreshold);

        //skip check
        if (ckp.size() == 0 || pkp.size() == 0 || fkp.size() == 0 || lkp.size() == 0){
            //save
            this->skipimagepathes.push_back(imagelist[pic]);
            this->skipimages.push_back(img);
            //save skipped image information as a roiRect data
            roiRect roi;
            roi.imagename = imagelist[pic];
            roi.picNo = useID[pic];
            roi.rotated = false;
            this->skippedRects.push_back(roi);
            //continue process
            continue;
        }

        //surf matching between CURRENT and PAST
        std::vector<cv::DMatch>	matches;
        matcher.match(cdesc, pdesc, matches);

        //skip check
        if (matches.size() == 0){
            //save
            this->skipimagepathes.push_back(imagelist[pic]);
            this->skipimages.push_back(img);
            //save skipped image information as a roiRect data
            roiRect roi;
            roi.imagename = imagelist[pic];
            roi.picNo = useID[pic];
            roi.rotated = false;
            this->skippedRects.push_back(roi);
            //continue process
            continue;
        }


        //surf matching between CURRENT and FIRST
        std::vector<cv::DMatch> fmatches;
        matcher.match(cdesc, fdesc, fmatches);
        if (fmatches.size() == 0){
            //save
            this->skipimagepathes.push_back(imagelist[pic]);
            this->skipimages.push_back(img);
            //save skipped image information as a roiRect data
            roiRect roi;
            roi.imagename = imagelist[pic];
            roi.picNo = useID[pic];
            roi.rotated = false;
            this->skippedRects.push_back(roi);
            //continue process
            continue;
        }

        //surf matching between CURRENT and LAST
        std::vector<cv::DMatch> lmatches;
        matcher.match(cdesc, ldesc, lmatches);
        if (lmatches.size() == 0){
            //save
            this->skipimagepathes.push_back(imagelist[pic]);
            this->skipimages.push_back(img);
            //save skipped image information as a roiRect data
            roiRect roi;
            roi.imagename = imagelist[pic];
            roi.picNo = useID[pic];
            roi.rotated = false;
            this->skippedRects.push_back(roi);
            //continue process
            continue;
        }


        //--- Normalization of distance ---//
        std::vector<float>	matchedDis, fmatchedDis, lmatchedDis;
        std::vector<float>	normalizedDis, fnormalizedDis, lnormalizedDis;

        //normalization
        //% Distance range will be changed into 0 to 1.
        //% About the distance values, 1 will be the best evaluation and 0 will be the worst.
        normalizeDistance(matches, matchedDis, normalizedDis);
        normalizeDistance(fmatches, fmatchedDis, fnormalizedDis);
        normalizeDistance(lmatches, lmatchedDis, lnormalizedDis);

        //--- remove some matches whose distance are less than a threshold ---//
        //--- save all keypoint coordinates which are used as matching pairs ---//
        std::vector<cv::Point2f> rckp, rpkp, rckpf, rfkp, rckpl, rlkp;
        std::vector<cv::DMatch> nmatches, nfmatches, nlmatches;

        countNewmatchpoint(ckp, pkp, rckp, rpkp, matches, nmatches, normalizedDis, disThresholdL, disThresholdU);
        countNewmatchpoint(ckp, fkp, rckpf, rfkp, fmatches, nfmatches, fnormalizedDis, disThresholdL, disThresholdU);
        countNewmatchpoint(ckp, lkp, rckpl, rlkp, lmatches, nlmatches, lnormalizedDis, disThresholdL, disThresholdU);

        //store the number of matching
        int matchingNum = (int)nmatches.size();
        int fmatchingNum = (int)nfmatches.size();
        int lmatchingNum = (int)nlmatches.size();

/*
//        //FOR MASTER THESIS: DRAWING MATCHES
//        //2015/02/10
//        cv::Mat outImgcp, outImgcf, outImgcl;
//        cv::KeyPoint tmpkp;
//        std::vector<cv::KeyPoint> KP1, KP2;
//        std::vector<cv::DMatch> tmpmatch;
//        //current vs past
//        for(int i=0; i<(int)nmatches.size(); i++){
//            int cid = nmatches[i].queryIdx;
//            int pid = nmatches[i].trainIdx;
//            KP1.push_back(ckp[cid]);
//            KP2.push_back(pkp[pid]);
//            tmpmatch.push_back(cv::DMatch(i, i, nmatches[i].distance));
//        }
////        cv::drawMatches(cworkimg, KP1, pworkimg, KP2, tmpmatch, outImgcp);//, cv::Scalar::all(-1), cv::Scalar::all(-1), 0);
//        //cv::drawMatches(cworkimg, ckp, pworkimg, pkp, matches, outImgcp);//, cv::Scalar::all(-1), cv::Scalar::all(-1), 0);
//        //current vs past
//        KP1.clear();
//        KP2.clear();
//        tmpmatch.clear();
//        //current vs past
//        for(int i=0; i<(int)nfmatches.size(); i++){
//            int cid = nfmatches[i].queryIdx;
//            int pid = nfmatches[i].trainIdx;
//            KP1.push_back(ckp[cid]);
//            KP2.push_back(fkp[pid]);
//            tmpmatch.push_back(cv::DMatch(i, i, nfmatches[i].distance));
//        }
////        cv::drawMatches(cworkimg, KP1, fworkimg, KP2, tmpmatch, outImgcf);//, cv::Scalar::all(-1), cv::Scalar::all(-1), 0);
//        //cv::drawMatches(cworkimg, ckp, fworkimg, fkp, fmatches, outImgcf);//, cv::Scalar::all(-1), cv::Scalar::all(-1), 0);
//        //current vs last
//        KP1.clear();
//        KP2.clear();
//        tmpmatch.clear();
//        //current vs past
//        for(int i=0; i<(int)nlmatches.size(); i++){
//            int cid = nlmatches[i].queryIdx;
//            int pid = nlmatches[i].trainIdx;
//            KP1.push_back(ckp[cid]);
//            KP2.push_back(lkp[pid]);
//            tmpmatch.push_back(cv::DMatch(i, i, nlmatches[i].distance));
//        }
////        cv::drawMatches(cworkimg, KP1, lworkimg, KP2, tmpmatch, outImgcl);//, cv::Scalar::all(-1), cv::Scalar::all(-1), 0);
//        //cv::drawMatches(cworkimg, ckp, lworkimg, lkp, lmatches, outImgcl);//, cv::Scalar::all(-1), cv::Scalar::all(-1), 0);

////        std::string matchsavepath = "M:\\WORKS\\MatchImages\\surfmatch_" + std::to_string(pic-1) + "vs" + std::to_string(pic) + ".tiff";
////        cv::imwrite(matchsavepath, outImgcp);
////        matchsavepath = "M:\\WORKS\\MatchImages\\surfmatch_" + std::to_string(pic) + "vs" + "first" + ".tiff";
////        cv::imwrite(matchsavepath, outImgcf);
////        matchsavepath = "M:\\WORKS\\MatchImages\\surfmatch_" + std::to_string(pic) + "vs" + "last" + ".tiff";
////        cv::imwrite(matchsavepath, outImgcl);
*/

        //--- DP matching -----------------------------------------------------------//
        std::vector<roiRect> pROIrects = picROIrects[count_roirects - 2];
        std::vector<roiRect> cROIrects = picROIrects[count_roirects - 1];
        int preNum = (int)pROIrects.size();
        int curNum = (int)cROIrects.size();

        std::vector<int> picDPPair;

        //initialization
        if (!cSim.empty())		cSim.release();
        cSim = cv::Mat::zeros(curNum, 1, CV_32FC1);


        //=== LOOP ===//
        float		maxSim;
        int         maxSimID;

        //--- C LOOP ---//
        std::vector<float>	cursimilarity;


        int skipNum1, skipNum2;
        skipNum1 = skipNum2 = 0;

        for (int c = 0; c < curNum; c++){


            //initialization
            maxSim = -10000000.0;// 0.0;
            maxSimID = -1;

            //get current keypoint information inside the cROIrects[c] from 3 states(v.s. PAST, FIRST and LAST)
            std::vector<int>	ckpIDs, fckpIDs, lckpIDs;
            cv::Rect croi = cROIrects[c].roi_rect;
            getPointID(rckp, ckpIDs, croi, cminx, cminy);       //v.s. PAST
            getPointID(rckpf, fckpIDs, croi, cminx, cminy);     //v.s. FIRST
            getPointID(rckpl, lckpIDs, croi, cminx, cminy);     //v.s. LAST

            //get croi's left-upper coordinate
            int croix = croi.x - cminx;
            int croiy = croi.y - cminy;

            //get keypoint information inside the firstROI & lastROI
            std::vector<int> fkpIDs, lkpIDs;
            cv::Rect froi = Firstroi_rect.roi_rect;
            cv::Rect lroi = Lastroi_rect.roi_rect;
            getPointID(rfkp, fkpIDs, froi, fminx, fminy);
            getPointID(rlkp, lkpIDs, lroi, lminx, lminy);

            //get sameID
            std::vector<int> fsameIDs, lsameIDs;
            getSameID(fckpIDs, fkpIDs, fsameIDs);
            getSameID(lckpIDs, lkpIDs, lsameIDs);

            //calculate similarity
            float fsim, lsim;
            fsim = countKeypointDistance(rckpf, rfkp, fsameIDs, croix, croiy, froix, froiy, epsilonDis);
            lsim = countKeypointDistance(rckpl, rlkp, lsameIDs, croix, croiy, lroix, lroiy, epsilonDis);
            fsim /= (float)fmatchingNum;
            lsim /= (float)lmatchingNum;

            //find maximum
            float stsim = fsim;
            if (stsim - lsim < 1E-10)
                stsim = lsim;


            //--- P LOOP ---//
            bool pcheck = true;
            for (int p = 0; p < preNum; p++){

                //add check
                float g;
                if (!firstCalcSimCheck){
                    g = 0.0;
                }
                else{
                    g = pSim.at<float>(p);
                }
                if (g < 0.0)
                    continue;


                //get keypoint information inside the pROIrects[p]
                std::vector<int> pkpIDs;
                cv::Rect proi = pROIrects[p].roi_rect;
                getPointID(rpkp, pkpIDs, proi, pminx, pminy);

                //get proi's left-upper coordinate
                int proix = proi.x - pminx;
                int proiy = proi.y - pminy;


                //store same IDs
                std::vector<int> sameIDs;
                getSameID(ckpIDs, pkpIDs, sameIDs);


                //calculate similarity[p][c]
                float dsim = countKeypointDistance(rckp, rpkp, sameIDs, croix, croiy, proix, proiy, epsilonDis);

                //normalization
                dsim /= (float)matchingNum;


                //set similarity
                float sim;
                float alpha = 0.1f;
                sim = dsim + g;
                float addsim = (1.0 - alpha)*stsim + alpha*sim;


                //find max val
                if (pcheck){
                    maxSim = addsim;
                    maxSimID = p;
                    pcheck = false;
                    continue;
                }
                if (maxSim - addsim < 1E-10){
                    maxSim = addsim;
                    maxSimID = p;
                }

            }// p loop end


            //save the similarity for test
            cSim.at<float>(c) = maxSim;
            cursimilarity.push_back(maxSim);
            //save a pair
            picDPPair.push_back(maxSimID);
            //count
            if (maxSimID == -1)
                skipNum2++;

        }// c loop end



        //set a flag
        firstCalcSimCheck = true;

        //--- Path check -------------------------------------------------------------//
        if (skipNum1 + skipNum2 == curNum){
            //save
            this->skipimagepathes.push_back(imagelist[pic]);
            this->skipimages.push_back(img);
            //save skipped image information as a roiRect data
            roiRect roi;
            roi.imagename = imagelist[pic];
            roi.picNo = useID[pic];
            roi.rotated = false;
            this->skippedRects.push_back(roi);

            //release paths
            std::vector<int>().swap(picDPPair);

            //continue process
            continue;
        }


        //--- Save information -------------------------------------------------------//
        //save DP pair
        DPPairs.push_back(picDPPair);
        std::vector<int>().swap(picDPPair);

        //save similarity
        if (!pSim.empty())
            pSim.release();
        pSim = cSim.clone();
        cSim.release();
        similarity.push_back(cursimilarity);

        //save image
        cworkimg.copyTo(pworkimg);
        cworkimg.release();

        //save coordinate
        pminx = cminx;
        pmaxx = cmaxx;
        pminy = cminy;
        pmaxy = cmaxy;


    }//pic loop end


    //--- Execute Back-Track -------------------------------------------------//
    //--- Get ROI information -------------------------------------------------//

    std::vector<roiRect> dpresult;

    //save the last ROI information
    dpresult.push_back(picROIrects[count_roirects - 1][0]);
    //get last ROI's pair ID
    int pairID = DPPairs[(count_roirects - 1) - 1][0];
    if (pairID == -1) {
        ERROR_STR = "Back tracking doesn't work";
        return false;
    }

    //show similarity
    qDebug() << similarity.size() << " " << count_roirects;
    qDebug() << similarity[0].size();
    qDebug() << count_roirects - 1-1 << ":" << similarity[count_roirects - 1 - 1][0];

    //print similarity
    for (int pic = (count_roirects - 1) - 1; pic>0; pic--){

        //save ROI information
        dpresult.push_back(picROIrects[pic][pairID]);

        //show similarity
        qDebug() << pic-1 << ":" << similarity[pic-1][pairID];


        //get next ID
        int ppairID = pairID;
        pairID = DPPairs[pic - 1][ppairID];
        if (pairID == -1) {
            ERROR_STR = "Back tracking doesn't work";
            return false;
        }
    }

    //--- Set a return value -------------------------------------------------//
    std::vector<roiRect>::reverse_iterator rit = dpresult.rbegin();
    while (rit != dpresult.rend()){
        roipath.push_back(*rit);
        ++rit;
    }


    //--- Celar variables ----------------------------------------------------//
    dpresult.clear();
    picROIrects.clear();
    DPPairs.clear();

    return true;
}




//=======================
// Control function of detection
//=========================
void MainWindow::on_pushButton_exeProgD_clicked()
{

    //--- Disable a button --------------------------//
    ui->pushButton_exeProgD->setEnabled(false);

    //--- Creare display -----------------------------//
    ui->label_showROIcoordD->clear();
    ui->label_showROIcoordD->repaint();


    //--- Set variables ------------------------------//
    const int roiNum = (int)this->ROIRects.size();
    const int allNum = (int)this->inputfiles.size();


    //--- Save the first ROI information ------------//
    roiRect roi_first = this->ROIRects[0];
    this->allROIpath.clear();
    this->allROIpath.push_back(roi_first);


    //--- Set a progress bar & status bar -----------//
    //progress bar
    this->dialog->reset();
    this->dialog->show();
    this->dialog->setMinimum(0);
    this->dialog->setMaximum(allNum+(int)this->ROIRects.size()-2);

    //status
    this->dialog->setLabelText(tr("process : auto detection...start"));
    qApp->processEvents();


    //=== ROI LOOP ===//
    this->auto_detection_counter = 0;
    int startid = 0;
    int stopid = 0;
    for(int no = 0; no < roiNum - 1; no++){

        //--- set first & last ROIs ---//
        roiRect firstroi = this->ROIRects[no];
        roiRect lastroi = this->ROIRects[no+1];

        //--- find start & stop positions ---//
        std::string fname = firstroi.imagename;
        std::string lname = lastroi.imagename;
        bool start_selected = false;
        bool stop_selected = false;

        for(int i=stopid; i<allNum; i++){
            //get a filename
            std::string filename = this->inputfiles[i];

            //check
            if(filename == fname){
                startid = i;
                start_selected = true;
            }
            else if(filename == lname){
                stopid = i;
                stop_selected = true;
            }

            //break
            if(start_selected && stop_selected)
                break;
        }

        //--- set an imagelist ---//
        std::vector<std::string> imagelist;
        std::vector<int>         usedidlist;
        for(int i = startid; i<=stopid; i++){
            imagelist.push_back(this->inputfiles[i]);
            usedidlist.push_back(this->inputIDs[i]);
        }

        //--- set workranges ---//
        std::vector<cv::Rect> workranges;
        setWorkRange(this->workfolderpath_d, ".tiff", imagelist, firstroi, lastroi, workranges);

        //--- call an automatic detection function ---//
        std::vector<roiRect> roipathes;
        this->auto_detection_canceled = false;
        bool align_check = alignmentDP0722(firstroi, lastroi, imagelist, usedidlist, workranges, roipathes, 300, 0);

        //error check
        if(!align_check && !this->auto_detection_canceled){
            QMessageBox::information(this, "FATAL ERROR", "!! FATAL !!" + QString::fromStdString(ERROR_STR));
            exit(-1);
        }
        else if(!align_check && this->auto_detection_canceled){
            QMessageBox::information(this, "CAUTION", QString::fromStdString(ERROR_STR));

            ui->pushButton_exeProgD->setEnabled(true);

            this->dialog->reset();
            this->dialog->hide();
            qApp->processEvents();

            return;
        }

        //--- save roipathes ---//
        for(int i=0; i<(int)roipathes.size(); i++){
            this->allROIpath.push_back(roipathes[i]);
        }

    }//loop end

    //show the status
    this->dialog->setLabelText(tr("process : auto detection...end"));
    this->dialog->repaint();
    this->dialog->reset();
    this->dialog->hide();
    qApp->processEvents();

    qDebug() << "auto detection counter: " << allNum << " " << this->auto_detection_counter;


    //--- Save result images ------------------------------------------------//
    const int allpathNum = (int)this->allROIpath.size();

    //initialize dialog
    this->dialog->reset();
    this->dialog->setMinimum(0);
    this->dialog->setMaximum(allpathNum-1);
    this->dialog->setLabelText("process : saving result images");
    this->dialog->repaint();
    this->dialog->show();
    qApp->processEvents();


    for(int id=0; id<allpathNum; id++){

        //show the status
        this->dialog->setValue(id);
        qApp->processEvents();

        //create a scnpath
        std::string scnpath = this->scnfolderpath_d+ "/" + this->allROIpath[id].imagename + ".scn";

        //get width & height of the current image
        openslide_t *obj = openslide_open(scnpath.c_str());
        int64_t width, height;
        if(obj != NULL){
            openslide_get_layer_dimensions(obj, 3, &width, &height);
        }
        else{
            width = height = 0;
        }
        int dx = (this->campusW-width)/2;
        int dy = (this->campusH -height)/2;

        //adjusting coordinates of ROIRECT
        this->allROIpath[id].roi_rect.x -= dx;
        this->allROIpath[id].roi_rect.y -= dy;

        //calculate new coordinate
        this->allROIpath[id].roi_rect.x *= 4;
        this->allROIpath[id].roi_rect.y *= 4;
        this->allROIpath[id].roi_rect.width *= 4;
        this->allROIpath[id].roi_rect.height *= 4;


        //call a function
        cv::Mat img;
        bool checked = showROI(scnpath, img, 2, this->allROIpath[id].roi_rect.x, this->allROIpath[id].roi_rect.y, this->allROIpath[id].roi_rect.width, this->allROIpath[id].roi_rect.height);

        //save
        if(checked){
            std::string savepath = this->resultfolderpath_d + "/" + this->allROIpath[id].imagename + ".tiff";
            cv::imwrite(savepath, img);
        }

    }

    //hide
    qApp->processEvents();
    this->dialog->hide();


    //--- Save ROI information if skip files didn't exist ---------------------------------------//
    if((int)this->skippedRects.size() == 0 && (int)this->skipimagepathes.size() == 0){
        //open a file
        std::string infpath = this->inffolderpath + "/roiInf_level2.tsv";
        std::ofstream ofs(infpath, ios::out);
        if(ofs.fail()){
            QMessageBox::information(this, "CAUTION", "file open error in doElastix :" + QString::fromStdString(infpath));
            ui->pushButton_quitProgD->setEnabled(true);
            return;
        }

        //write information
        for(int no = 0; no<allpathNum; no++){

            //get an object
            roiRect obj = this->allROIpath[no];

            //output to a file
            ofs << obj.picNo << "\t" << obj.imagename << "\t" << obj.roi_rect.x << "\t" << obj.roi_rect.y << "\t" << obj.roi_rect.width << "\t" << obj.roi_rect.height << "\t" << obj.rotated << std::endl;

        }

        //file close
        ofs.close();
    }


    //--- Save skipped image & files information ---------------------//
    //--- and Enable buttons for skip image processing ---------------//
    if((int)this->skippedRects.size() !=0 || (int)this->skipimagepathes.size() != 0){

        //save skipped images
        for(int i=0; i<(int)this->skipimages.size(); i++){
            //get a filename
            std::string skipfilename = this->skipimagepathes[i];

            //create a savepath
            std::string skipsavepath = this->skipfolderpath_d + "/" + skipfilename + ".tiff";

            //save
            cv::imwrite(skipsavepath, this->skipimages[i]);
        }

        //enable titles
        ui->label_step7->setEnabled(true);
        ui->label_showSkipImageS->setEnabled(true);

        //enable buttons
        ui->pushButton_selectSkipImageS->setEnabled(true);
        ui->pushButton_quitSkipImageS->setEnabled(true);

        //enable a line
        ui->lineEdit_showSkipNameD->setEnabled(true);

    }


    //--- Show result information ----------------------------------------------//
    QString status = "success: " + QString::number(allpathNum) + " skipped: " + QString::number((int)this->skippedRects.size());
    ui->label_reportResultD->setText(status);
    ui->label_reportResultD->repaint();


    //--- Show a button for quiting this program -------------------//
    if((int)this->skippedRects.size() == 0 && (int)this->skipimagepathes.size() == 0){
        ui->pushButton_quitProgD->setEnabled(true);
    }



}

