/*  =========================================================================
    knn
    =========================================================================
*/

#include <iostream>
#include <random>
#include <vector>
// #include "opencv2/ml/ml.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/flann.hpp>

// #include <algorithm>
// #include <memory>
// #include <functional>
// #include <chrono>
// #include <cstdlib>
// #include <iomanip>
using namespace std;



#define Dim 3
using TYPE = float;
int main(int argc, char* argv[]){

    ////data
    // std::array<std::array<TYPE,2>,8> data = {{{{1.1, 0.6}},{{0.4, 0.5}},{{0.2, 0.6}},{{0.5, 0.9}},
    //                                         {{1.2, 0.3}},{{0.7, 0.4}},{{0.8, 1.0}},{{0.1, 0.2}}}}; //original
    
    // Point<3, double> *pos = new Point<3>[NumPart];
    // for (int i = 0; i < NumPart; i++)
    // {
    //     pos[i][0] = P[i].Pos[0];
    //     pos[i][1] = P[i].Pos[1];
    //     pos[i][2] = P[i].Pos[2];
    // }

    // std::vector<Point<dim>> points_global(posGlobal, posGlobal + NumTotal);
    // kdtree.set_points(points_global);

    int c1, c2;
    string filename = "read.txt";
    FILE* fp = nullptr;
    fp = fopen(filename.data(), "r");
    fscanf(fp, "%d %d \n", &c1, &c2);
    fclose(fp);

    // const volatile long unsigned int count1 = 1;
    // auto ptr1 = (long unsigned int*)(&count1);
    // *ptr1 = c1;
    // const volatile long unsigned int count2 = 1;
    // auto ptr2 = (long unsigned int*)(&count2);
    // *ptr2 = c2;
    // std::cout<<count1<<" "<<count2<<"\n";
    // std::array<std::array<TYPE,count1>,count2> data;
    // // std::array<std::array<TYPE,count1>,count2>* data = new std::array<std::array<TYPE,count1>,count2>;
    
    vector<vector<double>> data;
    data.resize(c2);
    for(int i2=0;i2<c2;i2++){
        data[i2].resize(c1);
        for(int i1=0;i1<c1;i1++){
            data[i2][i1] = 1.*i1*i2;
        }
    }
    std::cout<<data[0][0]<<"\n";

    // cv::flann::Index kdtree;
    // kdtree.build(data, cv::flann::KDTreeIndexParams(1), 
    //     cvflann::FLANN_DIST_EUCLIDEAN);

    // cv::Mat mat_data(data.size(), data[0].size(), CV_32F);
    // for(size_t i = 0; i < mat_data.rows; ++i) {
    //     for (size_t j = 0; j < mat_data.cols; ++j) {
    //         mat_data.at<TYPE>(i,j) = data[i][j];
    //     }
    // }



    // Sample data
    std::vector<cv::Point3d> points;
    int np = (int)1e6;
    for(int i=0;i<np;i++){
        points.emplace_back(438.6, 268.8, 0.);
        std::cout<<i<<" ";
    }
    points.emplace_back(438.6, 268.8, 0.);
    points.emplace_back(439.1, 268.6, 0.);
    points.emplace_back(438.2, 268.1, 0.);
    points.emplace_back(498.3, 285.9, 0.);
    points.emplace_back(312.9, 245.9, 0.);
    points.emplace_back(313.4, 245.7, 0.);
    points.emplace_back(  1.4,   1.3, 0.);
    points.emplace_back(  1.3,   1.6, 0.);
    points.emplace_back(  1.0,   1.0, 0.);
    points.emplace_back(  2.0,   2.0, 0.);

    cv::Mat_<float> features(0,Dim);
    for(auto &&point : points){
        //Fill matrix
        cv::Mat row = (cv::Mat_<float>(1, Dim) << point.x, point.y, point.z);
        features.push_back(row);
    }
    std::cout << features << std::endl;
    cv::flann::Index flann_index(features, cv::flann::KDTreeIndexParams(1));

    unsigned int max_neighbours = 6;
    cv::Mat query = (cv::Mat_<float>(1, 3) << 1., 1., 0.);
    cv::Mat indices, dists; //neither assume type nor size here!
    // double radius= 2.0;
    // flann_index.radiusSearch(query, indices, dists, radius, max_neighbours,
    //     cv::flann::SearchParams(32));
    flann_index.knnSearch(query, indices, dists, max_neighbours,
        cv::flann::SearchParams(32));
    std::cerr<<indices.type()<<": "<<indices<<"\n";
    std::cerr<<dists.type()<<": "<<dists<<"\n";
    
    return 0;
}
