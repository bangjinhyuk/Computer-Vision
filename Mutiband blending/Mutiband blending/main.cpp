//
//  main.cpp
//  Mutiband blending
//
//  Created by 방진혁 on 2021/09/19.
//

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp> //기본적인 기능
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp> // 이미지를 키우거나 줄이거나 하는
#include <opencv2/highgui.hpp> //highlevel grapic

// Scenario
// 1. 이미지를 불러온다.
// 2. 각 이미지를 단계를 설정하여 가우시안 피라미드를 구한다.
// 3. 가우시안 피라미드를 사용하여 라플라시안 피라미드 생성
// 4. 오렌지와 사과 라플라시안을 주어진 mask에 맞추어 합성
// 5. 라플라시안 합성본들을 다시 합성시킨다.

using namespace cv;

const int PYR_LEVEL = 9;


/** @brief 가우시안 피라미드 생성

@param src 가우시안 피라미드를 생성할 이미지 Mat
 */
std::vector<Mat> buildGaussianPyramid(const Mat& src) {
    std::vector<Mat> GaussianPyramid(PYR_LEVEL);
    GaussianPyramid[0] = src.clone();
    
    for (auto i=1; i<PYR_LEVEL; i++)
        // 다운 샘플링 피라미드 위로
        pyrDown(GaussianPyramid[i-1], GaussianPyramid[i]);
    
    return GaussianPyramid;
}


/** @brief 가우시안 피라미드를 통해 라플라시안 피라미드 계산

@param src 가우시안 피라미드
 */
std::vector<Mat> GaussianPyramidToLaplacianPyramid(const std::vector<Mat>& src) {
    
    std::vector<Mat> LaplacianPyramid(PYR_LEVEL);
    LaplacianPyramid = src;

    // 라플라시안 i 번째 = octave i 번째 가우시안 - octave i+1 번째 가우시안 업 샘플링
    for (auto i=0; i<PYR_LEVEL-1; i++){
        Mat temp;
        pyrUp(LaplacianPyramid[i+1], temp, LaplacianPyramid[i].size());
        LaplacianPyramid[i] -=temp;
    }

    return LaplacianPyramid;
}


/** @brief 두 라플라시안 결합 이미지 배열 생성 (orange + apple)

@param whiteArea 흰 영역에 넣을 이미지 벡터
@param blackArea 검정 영역에 넣을 이미지 벡터
@param whitePixelNum calculateWhitePixel 함수에서 구한 whitepixel 개수
 */
std::vector<Mat> combineTwoLaplacianImage(const std::vector<Mat>& whiteArea,const std::vector<Mat>& blackArea,const std::vector<int>& whitePixelNum){
    std::vector<Mat> combineImageVector(PYR_LEVEL);
    
    for (auto i=0; i<PYR_LEVEL; i++){
        Mat combine;
        hconcat(whiteArea[i].colRange(0, whitePixelNum[i]), blackArea[i].colRange(whitePixelNum[i], blackArea[i].cols),combineImageVector[i]);
    }
    return combineImageVector;

}


/** @brief mask에서 각 피라미드 별 whitepixel 개수 계산

@param src mask 이미지 벡터
 */
std::vector<int> calculateWhitePixel(const std::vector<Mat>& src){
    //PYR_LEVEL에 맞게 배열 생성
    std::vector<int> WhitePixelNumVector(PYR_LEVEL);

    for (auto i=0; i<PYR_LEVEL; i++){
        int whiteNum = 0;
        for (auto j=0; j<src[i].cols;j++){
            // (128,128,128) 보다 어두우면 검정이라 간주
            if(src[i].at<Vec3b>(1, j)[0]<128){
                WhitePixelNumVector[i] = whiteNum;
                break;
            }
            //(128,128,128) 보다 밝으면 흰색이라 간주
            else{
                whiteNum ++;
            }
        }
    }
    return WhitePixelNumVector;
}


/** @brief 두 라플라시안 이미지를 합성한 Mat를 재합성: reconstruct

@param src combineTwoLaplacianImage 함수에서 구한 벡터
 */
Mat reconstructImages(std::vector<Mat>& src){
    Mat out = src.back();
    for(int i = int(src.size())-2; i>=0; i--){
        pyrUp(out, out, src[i].size());
        out += src[i];
    }
    return out;
}

int main()
{
    // Scenario 1
    Mat apple = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Mutiband blending/Mutiband blending/burt_apple.png",1);
    Mat orange = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Mutiband blending/Mutiband blending/burt_orange.png",1);
    Mat mask = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Mutiband blending/Mutiband blending/burt_mask.png",1);

    apple.convertTo(apple, CV_32F, 1/255.f);
    orange.convertTo(orange, CV_32F, 1/255.f);

    // Scenario 2
    std::vector<Mat> appleGaussianPyramid = buildGaussianPyramid(apple);
    std::vector<Mat> orangeGaussianPyramid = buildGaussianPyramid(orange);
    std::vector<Mat> maskGaussianPyramid = buildGaussianPyramid(mask);

    // Scenario 3
    std::vector<Mat> appleLaplacianPyramid = GaussianPyramidToLaplacianPyramid(appleGaussianPyramid);
    std::vector<Mat> orangeLaplacianPyramid = GaussianPyramidToLaplacianPyramid(orangeGaussianPyramid);

    // Scenario 4
    std::vector<int> maskWhitePixelNum = calculateWhitePixel(maskGaussianPyramid);
    std::vector<Mat> combineImageVector = combineTwoLaplacianImage(appleLaplacianPyramid,orangeLaplacianPyramid,maskWhitePixelNum);

    // Scenario 5
    Mat reconstruct;
    reconstruct = reconstructImages(combineImageVector);
    imshow("reconstruct", reconstruct);

    waitKey();
    return 0;
}
