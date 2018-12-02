#ifndef ___Class_SerialNumber
#define ___Class_SerialNumber

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>

class SerialNumber {
private:
    cv::Mat image; //元画像
    int height; //元画像の高さ
    int width; //元画像の幅
    float angle; //回転角度
    std::string angle_direction; //回転する方向 

public:
    //コンストラクタ
    SerialNumber(const std::string& fname);

    // 画像の取得
    cv::Mat& GetImage(){
        return image;
    }

    //エッジ抽出と回転補正
    void EdgeDetection(cv::Mat& src_img, cv::Mat& edge_img, cv::Mat& dst_img);

    //シリアルナンバー領域の抽出(以下のパラメータは変更しても良い)
    //シリアルナンバー領域の幅 : region_cols
    //シリアルナンバー領域の高さ : region_rows
    //X_OFFSETとY_OFFSETの分だけ領域を大きく取る
    void RegionExtraction(cv::Mat& src_img, cv::Mat& edge_img, cv::Mat& dst_img, int region_cols = 114, int region_rows = 16, int X_OFFSET = 3, int Y_OFFSET = 2);

    //シリアルナンバーの切り出し(以下のパラメータは変更しても良い)
    //char_min_size : 文字の最小横幅を7画素と仮定する
    //char_max_size : 文字の最大横幅を11画素と仮定する
    void RegionSegmentation(cv::Mat& src_img, cv::Mat& dst_img, int char_min_size = 7, int char_max_size = 11);
};



#endif
