//
//  main.cpp
//  opencvExam
//
//  Created by 방진혁 on 2021/09/14.
//



#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp> //기본적인 기능
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp> // 이미지를 키우거나 줄이거나 하는
#include <opencv2/highgui.hpp> //highlevel grapic

using namespace cv;

const int PYR_LEVEL = 5;

std::vector<Mat> buildGaussianPyramid(const Mat& src) {
    
    //가우시안 피라미드
    std::vector<Mat> GaussianPyramid(PYR_LEVEL);
    GaussianPyramid[0] = src.clone();
    
    for (auto i=1; i<PYR_LEVEL; i++)
        // 다운 샘플링 피라미드 위로 ㄱ
        pyrDown(GaussianPyramid[i-1], GaussianPyramid[i]);
    
    return GaussianPyramid;
}


std::vector<Mat> buildLaplacianPyramid(const Mat& src) {
    
    //라플라시안 피라미드
    std::vector<Mat> LaplacianPyramid(PYR_LEVEL);
    LaplacianPyramid[0] = src.clone();
    
    for (auto i=1; i<PYR_LEVEL; i++){
        // 다운 샘플링 피라미드 위로 ㄱ
        pyrDown(LaplacianPyramid[i-1], LaplacianPyramid[i]);
        Mat temp;
        pyrUp(LaplacianPyramid[i],temp, LaplacianPyramid[i-1].size());
        LaplacianPyramid[i-1] -=temp;
    }
    
    
    return LaplacianPyramid;
}

Mat reconstruct(std::vector<Mat>& pyramid){
    Mat out = pyramid.back();
    for(int i = int(pyramid.size())-2; i>=0; i--){
        pyrUp(out, out, pyramid[i].size());
        out += pyramid[i];
    }
    return out;
}

int main()
{
    //2번째 파라미터가 0 이면 흑백, 없거나 흑백이면 1
    Mat input = imread("/Users/bangjinhyuk/Documents/Ajou/opencvExam/opencvExam/lena.png",1);
    Mat src;
    
//    CV_32F float
//    CV_64F double
//    CV_32S int

    input.convertTo(src, CV_32F, 1/255.f);
    
    std::vector<Mat> GaussianPyramid = buildGaussianPyramid(src);
    std::vector<Mat> LaplacianPyramid = buildLaplacianPyramid(src);

    imshow("input",input);
    imshow("src",src); // clone을 해줘서 원본이 살아있다..

//    for (auto i = 0; i< GaussianPyramid.size(); i++){
//        std::string windowName = "Octave"+ std::to_string(i);
//        imshow(windowName, GaussianPyramid[i]);
//    }
    for (auto i = 0; i< LaplacianPyramid.size(); i++){
        std::string windowName = "Octave"+ std::to_string(i);
        imshow(windowName, LaplacianPyramid[i]+((i<LaplacianPyramid.size()-1)?Scalar(0.5,0.5,0.5):Scalar(0,0,0)));
    }
    Mat res = reconstruct(LaplacianPyramid);
    imshow("reconstruct",res);
    waitKey();
    return 0;
}


