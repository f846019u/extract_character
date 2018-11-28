#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "SerialNumber.h"

using namespace std;
using namespace cv;

int main()
{
    SerialNumber serial("0750_SN1_02.5_p.bmp"); //要変更
    Mat src_img = serial.GetImage();

    //エッジ抽出と回転補正
    Mat edge_img, rotation_img;
    serial.EdgeDetection(src_img, edge_img, rotation_img);
   
    //シリアルナンバー領域の抽出
    Mat extract_img;
    serial.RegionExtraction(rotation_img, edge_img, extract_img);

    //シリアルナンバー切り出し
    Mat segmentation_img;
    serial.RegionSegmentation(extract_img, segmentation_img);

    //切り出し画像の出力
    imwrite("segmentation_img.png", segmentation_img);
}