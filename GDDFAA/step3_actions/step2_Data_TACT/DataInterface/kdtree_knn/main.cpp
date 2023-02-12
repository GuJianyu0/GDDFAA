/*  =========================================================================
    Author: Leonardo Citraro
    Company:
    Filename: main.cpp
    Last modifed:   09.01.2017 by Leonardo Citraro
    Description:    KDtree example

    =========================================================================

    =========================================================================
*/
#include "KDtree.hpp"
#include <iostream>
#include <array>

int main(int argc, char* argv[]) {

    using TYPE = float;
    
    ////data
    // float data_test[8][3] = {{1.1, 0.6, 0.3},{0.4, 0.5, 0.9},{0.2, 0.6, 0.3},{0.5, 0.9, 0.3},
    //                                         {1.2, 0.3, 0.3},{0.7, 0.4, 0.3},{0.8, 1.0, 0.3},{0.1, 0.2, 0.3}};
    std::array<std::array<TYPE,2>,8> data = {{{{1.1, 0.6}},{{0.4, 0.5}},{{0.2, 0.6}},{{0.5, 0.9}},
                                            {{1.2, 0.3}},{{0.7, 0.4}},{{0.8, 1.0}},{{0.1, 0.2}}}}; //original
    // std::array<std::array<TYPE,3>,8> data = {{{{1.1, 0.6, 0.3}},{{0.4, 0.5, 0.9}},{{0.2, 0.6, 0.3}},{{0.5, 0.9, 0.3}},
    //                                         {{1.2, 0.3, 0.3}},{{0.7, 0.4, 0.3}},{{0.8, 1.0, 0.3}},{{0.1, 0.2, 0.3}}}};
    // Snapshot Data;
    // Data.load("/home/darkgaia/0prog/gadget/gadget-2.0.7/data_process/snapdata_byname_centre1.txt");
    // const int col_num=sizeof(Data.xv_data)/sizeof(Data.xv_data[0]); //colomn length of an array
    // std::array<std::array<TYPE, 3>, col_num> data; //static
    // // for(auto i : (&Data)->xv_data){
    // for(int i=0;i<col_num;i++){
    //     data[i][0] = Data.xv_data[i][0]; data[i][1] = Data.xv_data[i][1]; data[i][2] = Data.xv_data[i][2];
    // }

    // KDtree<TYPE,8,3> kdtree(&data); //??
    
    // // auto node = kdtree.get_node0();
    // // std::cout << "Is root node? " << std::boolalpha << node->is_root() << "\n";
    // // std::cout << "Split point(0)=\n" << node->get_split_point() << "\n";
    // // node = node->go_left();
    // // std::cout << "Split point(1a)=\n" << node->get_split_point() << "\n";
    // // node = node->go_left();
    // // std::cout << "Split point(2a)=\n" << node->get_split_point() << "\n";
    // // node = node->go_back();
    // // node = node->go_right();
    // // std::cout << "Split point(2b)=\n" << node->get_split_point() << "\n";
    
    // // // node_data is an Eigen::Map (view) of the original data
    // // auto node_data = node->get_data_sliced();
    
    // std::cout << "The point nearest to (0.55,0.4) is: \n";
    // std::array<TYPE,3> sample = {0.55, 0.4, 0.3};
    // auto nearest_samples_idx = kdtree.find_k_nearest<Distance::euclidean>(10, sample);
    // for(auto& ns:nearest_samples_idx){
    //     for(auto& v:data[ns])
    //         std::cout << v << " ";
    //     std::cout << "\n";
    // }
    
    return 0;
}
