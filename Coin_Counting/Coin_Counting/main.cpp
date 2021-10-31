//
//  main.cpp
//  Coin_Counting
//
//  Created by 방진혁 on 2021/10/31.
//

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp> //기본적인 기능
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp> // 이미지를 키우거나 줄이거나 하는
#include <opencv2/highgui.hpp> //highlevel grapic

using namespace cv;
using namespace std;

//      시나리오
//     1. 이미지를 불러온다.
//     2. 각 이미지마다 BilateralFilter를 적용해준뒤 GaussianBlur를 적용하여 노이즈를 없애준다.
//     3. 허프변환 원 검출(HoughCircles)을 해준다.
//     4. 이미지에 검출된 원을 그려준다.
//     5. 원 개수를 출력 해준다.

int main()
{

    // 각 파일을 벡터에 넣어준다.
    vector<Mat> images(6);
    
    images[0] = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Coin_Counting/Coin_Counting/coins0.jpeg",IMREAD_COLOR);
    
    images[1] = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Coin_Counting/Coin_Counting/coins1.jpeg",IMREAD_COLOR);
    
    images[2] = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Coin_Counting/Coin_Counting/coins2.jpeg",IMREAD_COLOR);
    
    images[3] = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Coin_Counting/Coin_Counting/coins3.jpeg",IMREAD_COLOR);
    
    images[4] = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Coin_Counting/Coin_Counting/coins4.jpeg",IMREAD_COLOR);
    
    images[5] = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Coin_Counting/Coin_Counting/coins5.jpeg",IMREAD_COLOR);
    
    if(images[0].empty()||images[1].empty()||images[2].empty()||images[3].empty()||images[4].empty()||images[5].empty()){
    printf(" Error images\n");
    return -1;
    }
    
    Mat gray;
    
    // 각 파일 하나씩 처리해준다.
    for (int count = 0; count < 6; ++count){
        //Gray scale 이미지로 변환 해준다
        cvtColor(images[count], gray, COLOR_BGR2GRAY);

        //GaussianBlur에 사용될 커널 사이즈 결정
        int ker_size=7;
        // gray 이미지 노이즈 제거 이미지 -> blurImage
        Mat blurImage;

        //BilateralFilter에 사용될 시그마 변수 결정
        double sigmaColor=10.0;
        double sigmaSpace=10.0;
        //먼저 BilateralFilter 적용
        bilateralFilter(gray, blurImage,10,sigmaColor,sigmaSpace);
        //GaussianBlur 적용
        GaussianBlur(blurImage, blurImage, Size(ker_size, ker_size),0.0);


        vector<Vec3f> circles;
        //원 검출
        HoughCircles(blurImage, circles, HOUGH_GRADIENT, 2, 30, 200, 90, 30, 80);

        //검출한 원을 원본이미지에 표시
        for( size_t i = 0; i < circles.size(); i++ )
        {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        int radius = c[2];
        circle( images[count], center, radius, Scalar(255,0,255), 3, LINE_AA);
        }
        //원 개수 출력
        cout << "images" << to_string(count) << ": " << to_string(circles.size()) <<endl;
        imshow("detected circles "+to_string(count), images[count]);
    }
    
    waitKey(0);
    return 0;
}
