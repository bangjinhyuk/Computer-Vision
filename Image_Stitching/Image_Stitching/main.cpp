//
//  main.cpp
//  Image_Stitching
//
//  Created by 방진혁 on 2021/12/21.
//

#include <iostream>
//#ifdef HAVE_OPENCV_XFEATURES2D
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

int main()
{

    // 각 파일을 벡터에 넣어준다.
    vector<Mat> images(6);
    
    Mat img1 = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Image_Stitching/imgL.png",IMREAD_GRAYSCALE);
    
    Mat img2 = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Image_Stitching/imgR.png",IMREAD_GRAYSCALE);
    if ( img1.empty() || img2.empty() )
    {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }
    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
    int minHessian = 6000;
    Ptr<SURF> detector = SURF::create( minHessian );
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute( img1, noArray(), keypoints1, descriptors1 );
    detector->detectAndCompute( img2, noArray(), keypoints2, descriptors2 );
    

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
    std::vector< DMatch > matches;
    matcher->match( descriptors1, descriptors2, matches );
    
    
    double dMaxDist = matches[0].distance;
    double dMinDist = matches[0].distance;
    double dDistance;

    // 두 개의 keypoint 사이에서 min-max를 계산한다 (min값만 사용)
    for (int i = 0; i < descriptors1.rows; i++) {
        dDistance = matches[i].distance;
        if (dDistance < dMinDist) dMinDist = dDistance;
        if (dDistance > dMaxDist) dMaxDist = dDistance;
    }
    printf("max_dist : %f \n", dMaxDist);
    printf("min_dist : %f \n", dMinDist);
    
    //match의 distance 값이 작을수록 matching이 잘 된 것
    //min의 값의 3배 또는 good_matches.size() > 60 까지만 goodmatch로 인정해준다.

    vector<DMatch> good_matches;
    for (int i = 0; i < descriptors1.rows; i++) {
        if (matches[i].distance == dMinDist)
            good_matches.push_back(matches[i]);
    }
    printf("good_matches : %d \n", good_matches.size());
    
    
    //keypoint들과 matching 결과 ("good" matched point)를 선으로 연결하여 이미지에 그려 표시
    Mat matGoodMatches;
    drawMatches(img1, keypoints1, img2, keypoints2, good_matches, matGoodMatches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    imshow("good-matches", matGoodMatches);
    
    
    //Point2f형으로 변환
    vector <Point2f> obj;
    vector <Point2f> scene;

    // goodmatch에서의 keypoint를 저장
    for (int i = 0; i < good_matches.size();i++) {
        obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
    }
    Mat HomoMatrix = findHomography(scene, obj, FM_RANSAC);
    //RANSAC기법을 이용하여 첫 번째 매개변수와 두번째 매개변수 사이의 3*3 크기의 투영행렬변환 H를 구한다
    cout << HomoMatrix << endl;

    //Homograpy matrix를 사용하여 이미지를 삐뚤게
    Mat matResult;
    warpPerspective(img2, matResult, HomoMatrix, Size(img1.cols*2, img1.rows*1.2), INTER_CUBIC);

    Mat matPanorama;
    matPanorama = matResult.clone(); //복사본 대입

    imshow("wrap", matResult);
    waitKey(3000);

    Mat matROI(matPanorama, Rect(0, 0, img1.cols, img1.rows));
    img1.copyTo(matROI);
    
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

