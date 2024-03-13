#ifndef WEBCAM_H
#define WEBCAM_H

#include <QThread>
#include <QTime>
#include <QDebug>
#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#define PATH_MAX    200

using namespace cv;

class webcam : public QThread
{
public:
    cv::VideoWriter writerCam;
    cv::VideoCapture captureCam;

    cv::Mat matIm;        
    int option;
    bool fLoad;
    char *streamSrc;
    int devSrc;

    bool isReady;
    bool evClose;
    uint camId;
    //QImage qimage;

    webcam();
    webcam(uint camId);
    webcam(uint camId, const char *streamSource);
    ~webcam();
    void run();
    void motion();
    void openStream();
    void thClose();
};

#endif // WEBCAM_H
