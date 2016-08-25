//
//  main.cpp
//  Barcode
//
//  Created by Caspar Wylie on 25/07/16.
//  Copyright (c) 2016 Caspar Wylie. All rights reserved.
//


/*
 {code:, itemDesc:, itemName:}
 */

#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

using namespace std;
using namespace cv;

bool isApprox(int value1, int value2, int tol){
    if( ((value1 - tol) <= value2) && ((value1 + tol) >= value2 ) ){
        return true;
    }else{
        return false;
    }
}

int main(int argc, const char * argv[]) {
    
    srand(time(0));
    
    int action;
    string homePath = "images/";
    
    cout << "1. Create New Barcode" << endl;
    cout << "2. Scan Barcode" << endl;
    cin >> action;
    
    
    if(action==1){
        //create
        
        int elementHeight = 100;
        string itemName;
        string itemDesc;
        
        cout << "Enter Item Name: ";
        cin >> itemName;
        cout << endl << "Enter Item Description: ";
        cin >> itemDesc;
        Mat Bar(elementHeight, 400, CV_8UC1, 255);
        
        int newCode[10] = {};
        int newX = 50;
        string strCode = "";
        for(int i = 0; i<10; i++){
            if(i==0){
                newCode[i] = 1;
            }else{
                newCode[i] = rand() % 10 + 1;
            }
            strCode = strCode + to_string(newCode[i]) + " ";
            int elementWidth = newCode[i] * 3; // x1.5 for px width
            Mat element = Bar(Rect(newX,0,elementWidth, elementHeight));
            element.setTo(Scalar(0));
            newX = newX + elementWidth + 5;
        }
        
      
        imshow("Display", Bar);
        bool saved = imwrite(homePath + itemName + ".jpg", Bar);
        
        string record = strCode + "|" + itemName + "|" + itemDesc;
        ofstream dataFile;
        dataFile.open(homePath + "records.txt", ios::app);
        dataFile << record << endl;
        dataFile.close();
        
        if(saved){
            cout << endl << endl << "Your Item Code Is " << strCode;
            cout << endl << "...And your barcode has been successfully saved (status: " << saved << "). ";
        }
        
        waitKey(0);
        
    }else{
        //scan
        int firstEleFrameWidth = 1;
        int firstEleFrameHeight = 100;
        int firstEleFrameX = 400;
        int firstEleFrameY = 300;
        int firstEleDetectOffset = firstEleFrameWidth*firstEleFrameHeight - 10;
        int extraCPcolor;
        Point extraCheckPoint = Point(135,350);
        Mat frame, grayFrame, threshFrame, foundBar;
        
        VideoCapture cap(0);
        
        while(true){
            cap.read(frame);
            cvtColor(frame, grayFrame, cv::COLOR_RGB2GRAY);
            threshold(grayFrame, threshFrame, 100, 255, THRESH_BINARY);
            putText(frame, "Place Barcode top left corner in square", Point(400,200), FONT_HERSHEY_PLAIN, 2, Scalar(0,255,0));
            rectangle(frame, Point(firstEleFrameX,firstEleFrameY), Point(firstEleFrameX + firstEleFrameWidth,firstEleFrameY + firstEleFrameHeight), Scalar(0,255,0),1);
            Mat firstElement = threshFrame(Rect(firstEleFrameX,firstEleFrameY,firstEleFrameWidth,firstEleFrameHeight));
            
            int count = 0;
           
            for(int i=0;i<firstElement.rows;i++){
                for(int x=0;x<firstElement.cols;x++){
                    if(firstElement.at<Vec3b>(i,x)[0]==0){
                        count++;
                    }
                 }
            }
           
            extraCPcolor = threshFrame.at<Vec3b>(extraCheckPoint)[0];
            if(count>firstEleDetectOffset&&extraCPcolor==255){
                rectangle(frame, Point(firstEleFrameX, firstEleFrameY), Point(firstEleFrameX+300,firstEleFrameY+firstEleFrameHeight), Scalar(255,0,0), 3);
                foundBar = threshFrame(Rect(firstEleFrameX, firstEleFrameY, 300, 100));
                Mat stripBar = foundBar(Rect(0,20,300,1));
                int codeFound[10] = {};
                string codeFoundStr = "";
                int bCount = 0;
                int eleCount = 0;
                
                for(int x=0;x<stripBar.cols;x++){
                    int color = stripBar.at<uchar>(0,x);
                    if(color==0){
                        bCount++;
                    }else{
                        if(bCount!=0){
                            int adjustedBcount = bCount/3;
                            codeFound[eleCount] = adjustedBcount;
                            codeFoundStr = codeFoundStr + to_string(adjustedBcount) +  " ";
                            //cout << bCount/3 << " ";
                            eleCount++;
                            bCount = 0;
                        }
                    }
                }
                
                ifstream file(homePath + "records.txt");
                string str;
                
                while (getline(file, str)) {
                    
                    size_t pipePos = str.find("|");
                    string posCode = str.substr(0,pipePos);
                    string posInfo = str.substr(pipePos, str.length());
                    int posCodeArr[10] = {};
                    istringstream iss(posCode, istringstream::in);
                    
                    string codeNum;
                    int similarityTestCount = 0;
                    int posCount = 0;
                    //cout << "comparing" << posCode << " to the detected " << codeFoundStr << ", similarity: " << endl;
                    while(iss >> codeNum){
                        int codeNumInt = stoi(codeNum);
                        if(isApprox(codeNumInt, codeFound[posCount], 2)){
                            similarityTestCount++;
                        }
                        posCodeArr[posCount] = codeNumInt;
                        posCount++;
                    }
                   // cout << similarityTestCount << endl;
                    
                    if(similarityTestCount>=8){
                        cout << posInfo;
                        putText(frame, "Item Found: " + posInfo, Point(100,100),FONT_HERSHEY_PLAIN, 2, Scalar(0,255,0));
                        break;
                    }
                    
                    //cout << posCode << endl;
                }
                
                cout << endl;
            }
           // cout << count << "-" << extraCPcolor << endl;
            count = 0;
            imshow("frame", frame);
            if(waitKey(33)==27){
                
                destroyAllWindows();
                break;
            }
        }
    }
    return 0;
}
