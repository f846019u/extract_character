#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <chrono>
//#include <sstream>
#include <glob.h>
#include "SerialNumber.h"


using namespace std;
using namespace cv;
using namespace std::chrono;

int main()
{
  
    string path;
    vector<string> tmp;
    glob_t globbuf;
    
    int ret = glob("/home/higaki/china_data/original/*.bmp", 0, NULL, &globbuf);
    
    for(int i = 0; i < globbuf.gl_pathc; i++){
      tmp.push_back(globbuf.gl_pathv[i]);
      cout << tmp[i] << endl;
    }
  
    SerialNumber serial("0750_SN1_02.5_p.bmp"); //要変更
    Mat src_img = serial.GetImage();
    
    //原画像の出力
    imwrite("src_img.png", src_img);
    
    cout << "計測開始" << endl;
    system_clock::time_point t0, t1, t2, t3, t4, t5;
    t0 = system_clock::now();
    
    //エッジ抽出と回転補正
    Mat edge_img, rotation_img;
    serial.EdgeDetection(src_img, edge_img, rotation_img);
    
    t1 = system_clock::now();
    
    //エッジ画像の出力
    imwrite("edge_img.png", edge_img);
    imwrite("rotation_img.png", rotation_img);

    //シリアルナンバー領域の抽出
    Mat extract_img;
    serial.RegionExtraction(rotation_img, edge_img, extract_img);

    t2 = system_clock::now();
    
    //シリアルナンバー切り出し
    Mat segmentation_img;
    serial.RegionSegmentation(extract_img, segmentation_img);
    
    t3 = system_clock::now();
    
    //切り出し画像の出力
    imwrite("segmentation_img.png", segmentation_img);

    //シリアルナンバーの推論
 
    
    
    t4 = system_clock::now();
  
    cout << "測定終了" << endl;
    
    double phase1, phase2, phase3, phase4;
    phase1 = duration_cast<milliseconds>(t1 - t0).count();
    phase2 = duration_cast<milliseconds>(t2 - t1).count();
    phase3 = duration_cast<milliseconds>(t3 - t2).count();

    time_t t = system_clock::to_time_t(t3);
    
    cout << time(&t) << endl;

    
    
}   
