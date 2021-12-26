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

    // distance min 값 찾기
    for (int i = 0; i < descriptors[0].rows; i++) {
        tmpDistance = matches[i].distance;
        if (tmpDistance < minDistance) minDistance = tmpDistance;
    }
    printf("minDistance : %f \n", minDistance);
    

    vector<DMatch> finalMatches;
    for (int i = 0; i < matches.size(); i++) {
        if (matches[i].distance == minDistance)
            finalMatches.push_back(matches[i]);
    }
    printf("finalMatches : %ld \n", finalMatches.size());
    return finalMatches;
}

int main()
{
    //두 사진을 넣어준다.
    Mat imgL = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Image_Stitching/imgL.png",IMREAD_COLOR);
    
    Mat imgR = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Image_Stitching/imgR.png",IMREAD_COLOR);
    if ( imgL.empty() || imgR.empty() )
    {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }
    
    // SURF descriptor 생성
    vector<Mat> descriptors;
    descriptors = createSURFDescriptor(600, imgL, imgR);
    
    //두 영상에서 얻은 desciptor match
    vector<DMatch> finalMatches;
    finalMatches = findFinalMatches(descriptors);
    
    
    //finalMatches에 저장된 match 그림으로 저장
    Mat matFinalMatches;
    drawMatches(imgL, keypointsL, imgR, keypointsR, finalMatches, matFinalMatches, Scalar::all(-1), Scalar::all(-1), vector<char>(),DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    imshow("matFinalMatches", matFinalMatches);
    
    
    
    
    //Point2f형으로 변환
    vector <Point2f> obj;
    vector <Point2f> scene;

    // goodmatch에서의 keypoint를 저장
    for (int i = 0; i < finalMatches.size();i++) {
        obj.push_back(keypointsL[finalMatches[i].queryIdx].pt);
        scene.push_back(keypointsR[finalMatches[i].trainIdx].pt);
    }
    Mat HomoMatrix = findHomography(scene, obj, FM_RANSAC);
    //RANSAC기법을 이용하여 첫 번째 매개변수와 두번째 매개변수 사이의 3*3 크기의 투영행렬변환 H를 구한다
    cout << HomoMatrix << endl;

    //Homograpy matrix를 사용하여 이미지를 삐뚤게
    Mat matResult;
    warpPerspective(imgR, matResult, HomoMatrix, Size(imgL.cols*2, imgL.rows*1.2), INTER_CUBIC);

    Mat matPanorama;
    matPanorama = matResult.clone(); //복사본 대입

    imshow("wrap", matResult);
    waitKey(3000);

    Mat matROI(matPanorama, Rect(0, 0, imgL.cols, imgL.rows));
    imgL.copyTo(matROI);
    
    imshow("Panorama", matPanorama);
    //검은 여백 잘라내기
    
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
    
    Mat blackCutPanorama;
    blackCutPanorama = matPanorama(Range(0, colory), Range(0, colorx));
    imshow("cutblack", blackCutPanorama);
    waitKey();
    return 0;
}

