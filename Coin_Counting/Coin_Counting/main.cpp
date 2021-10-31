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

int main()
{
    // Scenario 1
    Mat apple = imread("/Users/bangjinhyuk/Documents/개발/Git/Computer-Vision/Multi-band_blending/Multi-band_blending/burt_apple.png",1);
    
    
    imshow("apple", apple);

    waitKey();
    return 0;
}
