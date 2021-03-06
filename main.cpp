#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <glob.h>
#include <stdlib.h>
#include "SerialNumber.h"


using namespace std;
using namespace cv;
using namespace std::chrono;

vector<string> get_path(vector<string> image_path);
void system_cmd(char cmd, string path = "");
vector<string> split(const string &s, char delim);

vector<string> get_path(void){
  
    glob_t globbuf;
    vector<string> image_path;
    
    int ret = glob("/home/higaki/china_data/original/*.bmp", 0, NULL, &globbuf);
    
    for(int i = 0; i < globbuf.gl_pathc; i++){
      image_path.push_back(globbuf.gl_pathv[i]);
      //cout << image_path[i] << endl;
    }

    return image_path;
}

void system_cmd(int flag, string path){
    
    string cmd_cp;
    const char* cmd;
    
    if(flag == 0){
      cmd_cp = "cp " + path + " ./";
      cout << cmd_cp << endl;
      cmd = cmd_cp.c_str();
      system(cmd);
    }
    
    else if(flag == 1){
      cmd_cp = "rm ./*.bmp";
      cout << cmd_cp << endl;
      cmd = cmd_cp.c_str();
      system(cmd);
    }

    else {
      cout << "正しいsystem_cmdを入力してください" << endl;
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
    if (!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

int main()
{

    //path を取る
    string path;
    string src_name;
    vector<string> image_path = get_path();
    vector<string> tmp;
    
   
    for(int sheet = 0; sheet < image_path.size(); sheet++){

    path = image_path[sheet];
    cout << "path =  " << path << endl;
    
    //shell cmd 0:cp 1:rm
    system_cmd(0, path);
    
    tmp = split(image_path[sheet], '/');
    src_name = tmp[4];
    cout << "src_name =  " << src_name << endl;
    //path = "0000_SN1_02.0_p.bmp";

    SerialNumber serial(src_name); //要変更
    Mat src_img = serial.GetImage();
    
    //原画像の出力
    //imwrite("src_img.png", src_img);
    
    cout << "計測開始" << endl;
    system_clock::time_point t0, t1, t2, t3, t4, t5;
    t0 = system_clock::now();
    
    //エッジ抽出と回転補正
    Mat edge_img, rotation_img;
    serial.EdgeDetection(src_img, edge_img, rotation_img);
    
    t1 = system_clock::now();
    
    //エッジ画像の出力
    //imwrite("edge_img.png", edge_img);
    //imwrite("rotation_img.png", rotation_img);

    //シリアルナンバー領域の抽出
    Mat extract_img;
    serial.RegionExtraction(rotation_img, edge_img, extract_img);

    t2 = system_clock::now();
    
    //シリアルナンバー切り出し
    Mat segmentation_img;
    serial.RegionSegmentation(extract_img, segmentation_img);
    
    t3 = system_clock::now();
    
    //切り出し画像の出力
    imwrite("./Segmentation/seg_" + src_name , segmentation_img);

    }

    //シリアルナンバーの推論
 
    //shell cmd 0:cp 1:rm
    system_cmd(1, path);
    
    
    //t4 = system_clock::now();
  
    cout << "測定終了" << endl;
    
    double phase1, phase2, phase3, phase4;
    //phase1 = duration_cast<milliseconds>(t1 - t0).count();
    //phase2 = duration_cast<milliseconds>(t2 - t1).count();
    //phase3 = duration_cast<milliseconds>(t3 - t2).count();

    //time_t t = system_clock::to_time_t(t3);
    
    //cout << time(&t) << endl;

      
}   
