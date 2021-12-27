//
//  main.cpp
//  Image_Stitching
//
//  Created by 방진혁 on 2021/12/21.
//

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp> //기본적인 기능
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp> // 이미지를 키우거나 줄이거나 하는
#include <opencv2/highgui.hpp> //highlevel grapic
#include <opencv2/core/utility.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/xfeatures2d.hpp>

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;
using std::cout;
using std::endl;

vector<KeyPoint> keypointsL, keypointsR;

/** @brief SURF descriptor 생성
 */
vector<Mat> createSURFDescriptor(int minHessian, Mat imgL, Mat imgR){
    Ptr<SURF> detector = SURF::create(minHessian);
    vector<Mat> descriptors(2);
    detector->detectAndCompute(imgL, noArray(), keypointsL, descriptors[0]);
    detector->detectAndCompute(imgR, noArray(), keypointsR, descriptors[1]);
    return descriptors;
}

/** @brief desciptor match 하여 finalMatches 찾기
 */
vector<DMatch> findFinalMatches(vector<Mat> descriptors){
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
    vector< DMatch > matches;
    matcher->match(descriptors[0], descriptors[1], matches);
    
    double minDistance = matches[0].distance;
    double tmpDistance;

    for (int i = 0; i < descriptors[0].rows; i++) {
        tmpDistance = matches[i].distance;
        if (tmpDistance < minDistance) minDistance = tmpDistance;
    }
    printf("minDistance : %f \n", minDistance);
    
    double value = 4.0;
    double result = 100;
    while(true){
        vector<DMatch> finalMatches;
        for (int i = 0; i < matches.size(); i++) {
            if (matches[i].distance < minDistance*value)
                finalMatches.push_back(matches[i]);
        }
        printf("finalMatches (value=%lf) : %ld \n", value, finalMatches.size());
        result = finalMatches.size();
        value = value - 0.1;
        if(result<60){
            return finalMatches;
        }
    }
}

/** @brief match를 이용하여 Homography를 계산
 */
Mat calHomography(vector<DMatch> finalMatches){
    vector <Point2f> obj;
    vector <Point2f> scene;

    for (int i = 0; i < finalMatches.size();i++) {
        obj.push_back(keypointsL[finalMatches[i].queryIdx].pt);
        scene.push_back(keypointsR[finalMatches[i].trainIdx].pt);
    }
    Mat HomoMatrix = findHomography(scene, obj, FM_RANSAC);
    cout << HomoMatrix << endl;
    return HomoMatrix;
}

/** @brief Homograph를 이용하여 한 개의 image를 warping하고, 나머지 image는 copy
 */
Mat getResultImage(Mat Homography, Mat imgL, Mat imgR){
    Mat matResult;
    warpPerspective(imgR, matResult, Homography, Size(imgL.cols*2, imgL.rows*1.2),INTER_CUBIC);

    Mat matPanorama;
    matPanorama = matResult.clone();

    Mat matROI(matPanorama, Rect(0, 0, imgL.cols, imgL.rows));
    imgL.copyTo(matROI);

    int colorx = 0, colory = 0;
    for (int y = 0; y < matPanorama.rows; y++) {
        for (int x = 0; x < matPanorama.cols; x++) {
            if (matPanorama.at<Vec3b>(y, x)[0] == 0 && matPanorama.at<Vec3b>(y, x)[1] == 0 && matPanorama.at<Vec3b>(y, x)[2] == 0) {
                continue;
            }
            if (colorx < x) {
                colorx = x;
            }
            if (colory < y){
                colory = y;
            }
        }
    }

    Mat resultImage;
    resultImage = matPanorama(Range(0, colory), Range(0, colorx));
    return resultImage;
}

int main()
{
    //두 사진을 넣어준다.
    Mat imgL = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Image_Stitching/IMG_3639.jpg",IMREAD_COLOR);

    Mat imgR = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Image_Stitching/IMG_3640.jpg",IMREAD_COLOR);
    
    
    // 두 사진 유효성 체크
    if ( imgL.empty() || imgR.empty() )
    {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }
    
    // 사진 resize 해주기
    Size size(imgL.cols/2, imgL.rows/2);
    resize(imgL, imgL, size);
    resize(imgR, imgR, size);

    
    // SURF descriptor 생성
    vector<Mat> descriptors;
    descriptors = createSURFDescriptor(700, imgL, imgR);
    
    //두 영상에서 얻은 desciptor match
    vector<DMatch> finalMatches;
    finalMatches = findFinalMatches(descriptors);
    

    //finalMatches에 저장된 match 그림으로 저장
    Mat matFinalMatches;
    drawMatches(imgL, keypointsL, imgR, keypointsR, finalMatches, matFinalMatches, Scalar::all(-1), Scalar::all(-1), vector<char>(),DrawMatchesFlags::DEFAULT);
    imshow("matFinalMatches", matFinalMatches);
    
    
    //match를 이용하여 Homography를 계산
    Mat Homography;
    Homography = calHomography(finalMatches);
    
    
    //Homograph를 이용하여 한 개의 image를 warping하고, 나마지 image는 copy
    Mat resultImage;
    resultImage = getResultImage(Homography, imgL, imgR);
    
    //결과 영상 출력
    imshow("resultImage", resultImage);
    
    waitKey();
    return 0;
}

