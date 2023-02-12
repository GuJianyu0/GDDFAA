/*  =========================================================================
    knn
    =========================================================================
*/
#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main(int, char *argv[])
{
    Mat img;
        
    //读取原始图像
    img = imread(argv[1]);
    if (img.empty()) {
        //检查是否读取图像
        cout << "Error! Input image cannot be read...\n";
        return -1;
    }

    //创建窗口
    namedWindow(argv[1]);

    //显示图片
    imshow(argv[1], img);

    // 退出
    cout << "Press any key to exit...\n";
    waitKey(); // Wait for key press
    destroyAllWindows();

    // return
    return 0;
}
