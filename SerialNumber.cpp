#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "SerialNumber.h"

using namespace std;
using namespace cv;

SerialNumber::SerialNumber(const string& fname)
{
    image = imread(fname, 0);
    height = image.rows;
    width = image.cols;
    angle = stof(fname.substr(9,4));
    angle_direction = fname.substr(14, 1);
}

void SerialNumber::EdgeDetection(Mat& src_img, Mat& edge_img, Mat& dst_img)
{
    //エッジ画像の作成
    Mat tmp_img;
    Mat sobel_x, sobel_y, sobel_img;
    Sobel(src_img, sobel_x, CV_32F, 1, 0, 3);
    Sobel(src_img, sobel_y, CV_32F, 0, 1, 3);
    pow(sobel_x, 2.0, sobel_x);
    pow(sobel_y, 2.0, sobel_y);
    sqrt(sobel_x + sobel_y, sobel_img);
    sobel_img.convertTo(sobel_img, CV_8U);
    threshold(sobel_img, sobel_img, 200, 255, CV_THRESH_TOZERO);

    //回転補正
    float agl;
    Point2f center(width * 0.5, height * 0.5);
    agl = angle_direction == "n" ? angle * (-1.0) : angle;
    const Mat affine_matrix = getRotationMatrix2D(center, agl, 1.0);
    warpAffine(sobel_img, edge_img, affine_matrix, sobel_img.size());
    warpAffine(src_img, dst_img, affine_matrix, src_img.size());
}

void SerialNumber::RegionExtraction(Mat& src_img, Mat& edge_img, Mat& dst_img, int region_cols, int region_rows, int X_OFFSET, int Y_OFFSET)
{
    //シリアルナンバー縦領域抽出
    //1. 縦方向へ射影したヒストグラム
    vector<int> hist_rows(height, 0);
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            hist_rows[i] += edge_img.at<uchar>(i,j);
        } 
    }
    
    //2. 縦幅をregion_rowsと仮定して，輝度値の和を計算する
    vector<int> sum_hist_rows(height - region_rows, 0);
    for(int i = 0; i < height - region_rows; i++){
        for(int j = 0; j < region_rows; j++){
            sum_hist_rows[i] += hist_rows[i + j];
        }
    }

    //3. 輝度値の和が最も高い領域を抽出する
    int max_rows = sum_hist_rows[0];
    int upper; //上端
    for(int i = 0; i < height - region_rows; i++){
        if(max_rows < sum_hist_rows[i]){
            max_rows = sum_hist_rows[i];
            upper = i;
        } 
    }

    //シリアルナンバー横領域抽出
    //4. 横方向へ射影したヒストグラム
    vector<int> hist_cols(width, 0);
    for(int i = 0; i < width; i++){
        for(int j = 0; j < region_rows; j++){
            hist_cols[i] += edge_img.at<uchar>(j+upper, i);
        }
    }

    //5. 横幅をregion_colsと仮定して，輝度値の和を計算する
    vector<int> sum_hist_cols(width - region_cols, 0);
    for(int i = 0; i < width - region_cols; i++){
        for(int j = 0; j < region_cols; j++){
            sum_hist_cols[i] += hist_cols[i + j];
        }
    }

    //6. 輝度値の和が最も高い領域を抽出する
    int max_cols = sum_hist_cols[0];
    int left; //左端
    for(int i = 0; i < width - region_cols; i++){
        if(max_cols < sum_hist_cols[i]){
            max_cols = sum_hist_cols[i];
            left = i;
        } 
    }

    //7. シリアルナンバー領域抽出
    dst_img = Mat(src_img, Rect(left-X_OFFSET, upper-Y_OFFSET, region_cols+X_OFFSET*2, region_rows+Y_OFFSET*2));
}


void SerialNumber::RegionSegmentation(Mat& src_img, Mat& dst_img, int char_min_size, int char_max_size)
{
    int H = src_img.rows; //シリアルナンバー領域の高さ
    int W = src_img.cols; //シリアルナンバー領域の幅
    dst_img = src_img.clone();

    //1.前半分と後半分に分けて，判別分析法で二値化処理を行い，最後に統合する
    Mat src_img_1, src_img_2;
    Mat bin_img_1, bin_img_2, bin_img;
    src_img_1 = Mat(src_img, Rect(0, 0, W/2, H));
    src_img_2 = Mat(src_img, Rect(W/2, 0, W/2, H));
    threshold(src_img_1, bin_img_1, 0, 255, THRESH_BINARY | THRESH_OTSU);
    threshold(src_img_2, bin_img_2, 0, 255, THRESH_BINARY | THRESH_OTSU);
    hconcat(bin_img_1, bin_img_2, bin_img);


    //imwrite("src_img_1.png", src_img_1);
    //imwrite("bin_img_1.png", bin_img_1);
    //imwrite("bin_img_2.png", bin_img_2);
    
    //2.ラベリングでノイズ除去
    Mat label_img, stats, centroids;
    bin_img = ~bin_img;
    int nlabels = connectedComponentsWithStats(bin_img, label_img, stats, centroids);
    vector<uchar> mask(nlabels);
    mask[0] = 0;
    for(int i = 1; i < nlabels; i++){
        int *param = stats.ptr<int>(i);
        int h = param[ConnectedComponentsTypes::CC_STAT_HEIGHT];
        int size = param[ConnectedComponentsTypes::CC_STAT_AREA];
        if(h < 6 || size < 6) mask[i] = 0; //縦6画素または面積6以下は除去
        else mask[i] = 255;
    }
    Mat noise_removal_img(bin_img.size(), CV_8UC1);
    for(int y = 0; y < H; y++){
        for(int x = 0; x < W; x++){
            int label = label_img.at<int>(y, x);
            uchar &pixel = noise_removal_img.at<uchar>(y, x);
            pixel = mask[label];
        }
    }
    noise_removal_img = ~noise_removal_img;

    //3.画像処理による切り出し
    vector<int> hist(W, 0);
    for(int i = 0; i < W; i++){
        for(int j = 0; j < H; j++){
            hist[i] += noise_removal_img.at<uchar>(j,i); //切り出し位置に縦線を引く
        }
    }
    int loc = 2;
    int thre = H;
    int flag = 0;
    while(loc < W){
        if(hist[loc] >=  255 * thre){
            for(int y = 0; y < H; y++){
                dst_img.at<uchar>(y,loc) = 255;
            }
            loc += char_max_size;
            thre = 20;
            flag = 0;
        }else{
            flag += 1;
            loc++;
        }
        
        if(flag == char_min_size){
            loc -= char_min_size;
            thre -= 1;
        }
    }   
}
