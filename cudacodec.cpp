#include<iostream>
#include<opencv2/core/core.hpp>
//#include <opencv2/core/opengl.hpp>
#include <opencv2/cudacodec.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
    std::cout << "hello" << std::endl;
    std::string fileName = "video.avi";

#if not defined(HAVE_OPENCV_CUDACODEC)
    VideoCapture cap;
    cap.open(fileName);
#else
//    cv::namedWindow("GPU", cv::WINDOW_OPENGL);
//    cv::cuda::setGlDevice();
    cv::cuda::GpuMat d_frame;
    cv::Ptr<cv::cudacodec::VideoReader> d_reader = cv::cudacodec::createVideoReader(fileName);
#endif

    while(1)
    {
#if not defined(HAVE_OPENCV_CUDACODEC)
        Mat frame;

        bool bSuccess = cap.read(frame); // read a new frame from video


         if (!bSuccess) //if not success, break loop
        {
                        cout << "Cannot read the frame from video file" << endl;
                       break;
        }

        imshow("MyVideo", frame); //show the frame in "MyVideo" window
#else
        if (!d_reader->nextFrame(d_frame))
            break;
        Mat frame;
        d_frame.upload(frame);
        cv::imshow("CPU", frame);
#endif
        if(waitKey(30) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
       {
                cout << "esc key is pressed by user" << endl;
                break;
       }
    }

    return 0;
}