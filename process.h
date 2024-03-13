#ifndef PROCESS_H
#define PROCESS_H

#include <QDebug>
#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/gpu/gpu.hpp>
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

#define IMG_HEIGHT 	240
#define IMG_WIDTH	320

#define I_H         IMG_HEIGHT
#define I_W         IMG_WIDTH

#define CONC_RANGE	60	//maximum values for shift
#define VECT_WIDTH 	8	//number of compared square
#define STEP        1
#define SAD_SIZE    9

#define RGB(r, g, b)    (int)(r|(g<<8)|(b<<16))
#define sqr(x)  (x*x)

extern int disp_range;
extern int win_size;

extern Point2f src0[];
extern Point2f src1[];
extern Point2f dst[];

extern ushort imDiff[I_W*I_H*4];
extern uint imMin[I_W*I_H*4];
extern uint imColSum[I_W*4];
extern uchar imCalib[I_W*I_H*4];
extern uchar imEdge1[I_W*I_H*4];
extern uchar imEdge2[I_W*I_H*4];
extern uchar imFill0[I_W*I_H*4];
extern uchar imFill1[I_W*I_H*4];

extern uchar imTest[I_W*I_H*3*4];
extern Mat imTemp;

extern Mat matImPrev, matImMotion;

extern Ptr<StereoSGBM> sgbm;

void motion(Mat *matIm);

void getDisparity(uchar *im1, uchar *im2, uchar *imDisp);
void getEdge(uchar *im1, uchar *im2);
void getCalib(uchar *im1, uchar *im2);
void convertToGray(QImage src, QImage *dst);
void loadCalib();
void saveCalib();
void color_init();
void countbits_init();
void colorDisparity(uchar *Disp, uchar *img);
void DisparityOCV(Mat img1, Mat img2, Mat *disp8, uchar method);
void getCensus(uchar *im1, uchar *im2, uchar *imDisp);
void imgOverlap(uchar *im1, uchar *im2, uchar *imResult);

template <class anyType>
anyType abs(anyType value);

#endif // PROCESS_H
