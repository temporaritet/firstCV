#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <iostream>

using namespace std;
using namespace cv;

/* Create 2 camera objects and initialize camId */
webcam cam0(0);
webcam cam1(1);

QDir dir;
QString path;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tmrTimer = new QTimer(this);
    connect(tmrTimer, SIGNAL(timeout()), this, SLOT(GetImg()));
    //tmrTimer->start(150);

    ui->comboBoxAlg->addItems(QStringList() << "SGBM" << "BM" << "Census" << "SAD");
    ui->comboBoxImgLeft->addItems(QStringList() << "File" << "Stream" << "Cam 0" << "Cam 1"  );
    ui->comboBoxImgRight->addItems(QStringList() << "File" << "Stream" << "Cam 1" << "Cam 0"  );

    path = dir.relativeFilePath("images/woodL.bmp");
    strcpy(cam0.streamSrc, path.toStdString().c_str());
    ui->lineEditLeft->setText(path);

    path = dir.relativeFilePath("images/woodR.bmp");
    strcpy(cam1.streamSrc, path.toStdString().c_str());
    ui->lineEditRight->setText(path);

    cam0.fLoad = true;
    cam1.fLoad = true;

    color_init();
    countbits_init();
}

MainWindow::~MainWindow()
{    
    cam0.thClose();
    cam1.thClose();

    cam0.wait();
    cam1.wait();

    delete tmrTimer;
}

void MainWindow::calibrationCalc()
{
    float ax;

    QImage qimageC0( (uchar*)cam0.matIm.data, cam0.matIm.cols, cam0.matIm.rows, cam0.matIm.step, QImage::Format_RGB888);
    QImage qimageC1( (uchar*)cam1.matIm.data, cam1.matIm.cols, cam1.matIm.rows, cam1.matIm.step, QImage::Format_RGB888);

    Size patternsize(8,5); //interior number of corners
    vector<Point2f> corners; //this will be filled by the detected corners
    vector<Point2f> corners2; //this will be filled by the detected corners
    bool patternfound = findChessboardCorners(cam0.matIm, patternsize, corners, CALIB_CB_ADAPTIVE_THRESH+CALIB_CB_NORMALIZE_IMAGE+CALIB_CB_FAST_CHECK);
    bool patternfound2 = findChessboardCorners(cam1.matIm, patternsize, corners2, CALIB_CB_ADAPTIVE_THRESH+CALIB_CB_NORMALIZE_IMAGE+CALIB_CB_FAST_CHECK);

    if((patternfound) && (patternfound2))
    {
        drawChessboardCorners(cam0.matIm, patternsize, Mat(corners), patternfound);
        drawChessboardCorners(cam1.matIm, patternsize, Mat(corners2), patternfound2);

        ax = corners2[39].x - corners[39].x;
        ax += corners2[32].x - corners[32].x;
        ax += corners2[7].x - corners[7].x;
        ax += corners2[0].x - corners[0].x;
        ax /= 4;

        src0[0].x = corners[39].x+ax; src0[0].y = corners[39].y;
        src0[1].x = corners[32].x+ax; src0[1].y = corners[32].y;
        src0[2].x = corners[7].x+ax;  src0[2].y = corners[7].y;
        src0[3].x = corners[0].x+ax;  src0[3].y = corners[0].y;

        src1[0] = corners2[39];
        src1[1] = corners2[32];
        src1[2] = corners2[7];
        src1[3] = corners2[0];


        qDebug() << src0[0].x << " | " << src0[0].y;
        qDebug() << src0[1].x << " | " << src0[1].y;
        qDebug() << src0[2].x << " | " << src0[2].y;
        qDebug() << src0[3].x << " | " << src0[3].y;

        qDebug() << src1[0].x << " | " << src1[0].y;
        qDebug() << src1[1].x << " | " << src1[1].y;
        qDebug() << src1[2].x << " | " << src1[2].y;
        qDebug() << src1[3].x << " | " << src1[3].y;

        ui->label_Im0->setPixmap(QPixmap::fromImage(qimageC0)); // 2-4ms
        ui->label_Im1->setPixmap(QPixmap::fromImage(qimageC1)); // 2-4ms

    }
}

void MainWindow::SGBMCalc()
{
    int preFilterCap = ui->horizontalSlider_preFilterCap->sliderPosition();//63;
    int SADWindowSize = ui->horizontalSlider_SADWindowSize->sliderPosition();
    int minDisparity = ui->horizontalSlider_minDIsparity->sliderPosition();//0;
    int numDisparities = ui->horizontalSlider_numberOfDisparities->sliderPosition();//64;
    int setUniquenessRatio = ui->horizontalSlider_uniquenessRatio->sliderPosition();//0;
    int speckleWindowSize = ui->horizontalSlider_speckleWindowSize->sliderPosition();//0;
    int speckleRange = ui->horizontalSlider_speckleRange->sliderPosition();//0;
    int disp12MaxDiff = ui->horizontalSlider_disp12MaxDiff->sliderPosition();//0;

    sgbm->setPreFilterCap(preFilterCap);
    sgbm->setBlockSize(SADWindowSize);
    sgbm->setP1(8*3*SADWindowSize*SADWindowSize);
    sgbm->setP2(32*3*SADWindowSize*SADWindowSize);
    sgbm->setMinDisparity(minDisparity);
    sgbm->setNumDisparities(numDisparities);
    sgbm->setUniquenessRatio(setUniquenessRatio);
    sgbm->setSpeckleWindowSize(speckleWindowSize);
    sgbm->setSpeckleRange(speckleRange);
    sgbm->setDisp12MaxDiff(disp12MaxDiff);
    sgbm->setMode(StereoSGBM::MODE_SGBM);


    DisparityOCV(matRotated0, matRotated1, &matProc, 1);

    colorDisparity(matProc.data, imTest);

    imgOverlap(matRotated0.data, matRotated1.data, matRotated0.data);

    QImage qimageC2( (uchar*)matRotated0.data, matRotated0.cols, matRotated0.rows, matRotated0.step, QImage::Format_Indexed8);
    QImage qimageC3( (uchar*)imTest, cam0.matIm.cols, cam0.matIm.rows, cam0.matIm.step, QImage::Format_RGB888);

    ui->label_Im0->setPixmap(QPixmap::fromImage(qimageC2)); // 2-4ms
    ui->label_Im1->setPixmap(QPixmap::fromImage(qimageC3)); // 2-4ms
}

void MainWindow::CensusCalc()
{
    getCensus(matRotated0.data, matRotated1.data, matProc.data);
    //imshow("Temp Image",imTemp);

    colorDisparity(matProc.data, imTest);

    imgOverlap(matRotated0.data, matRotated1.data, matRotated0.data);
    //Canny(matProc, matProc, 40, 40, 3, 0);
    //imshow("Disparity2", matProc);

    QImage qimageC2( (uchar*)matRotated0.data, matRotated0.cols, matRotated0.rows, matRotated0.step, QImage::Format_Indexed8);
    QImage qimageC3( (uchar*)imTest, cam0.matIm.cols, cam0.matIm.rows, cam0.matIm.step, QImage::Format_RGB888);

    ui->label_Im0->setPixmap(QPixmap::fromImage(qimageC2)); // 2-4ms
    ui->label_Im1->setPixmap(QPixmap::fromImage(qimageC3)); // 2-4ms

}

void MainWindow::SADCalc()
{
    getDisparity(matRotated0.data, matRotated1.data, matProc.data);
    //imshow("Temp Image",imTemp);

    colorDisparity(matProc.data, imTest);

    imgOverlap(matRotated0.data, matRotated1.data, matRotated0.data);

    QImage qimageC2( (uchar*)matRotated0.data, matRotated0.cols, matRotated0.rows, matRotated0.step, QImage::Format_Indexed8);
    QImage qimageC3( (uchar*)imTest, cam0.matIm.cols, cam0.matIm.rows, cam0.matIm.step, QImage::Format_RGB888);

    ui->label_Im0->setPixmap(QPixmap::fromImage(qimageC2)); // 2-4ms
    ui->label_Im1->setPixmap(QPixmap::fromImage(qimageC3)); // 2-4ms
}

void MainWindow::selectOption(webcam *cam, QString textSelect)
{
    if(textSelect == "Cam 0")
    {
        cam->option = 0;
    }
    else if(textSelect == "Cam 1")
    {
        cam->option = 1;
    }
    else if(textSelect == "File")
    {
        cam->option = 2;
        strcpy(cam->streamSrc, ui->lineEditLeft->text().toStdString().c_str());
    }
    else if(textSelect == "Stream")
    {
        cam->option = 3;
        strcpy(cam->streamSrc, ui->lineEditLeft->text().toStdString().c_str());
    }
    else
    {
        cam->option = 4;
        strcpy(cam->streamSrc, "");
    }
}

void MainWindow::GetImg()
{

    if(!cam0.isRunning())
    {
        cam0.start();
    }
    if(!cam1.isRunning())
    {
        cam1.start();
    }

    /* Check images are in buffer */
    if(cam0.isReady && cam1.isReady)
    {
        QTime timer;

        cam0.matIm.copyTo(matProc);

        if(ui->CheckBox_Calib->isChecked())
        {
            calibrationCalc();
        }
        else
        {
            cvtColor(cam0.matIm, matRotated0, COLOR_BGR2GRAY);
            cvtColor(cam1.matIm, matRotated1, COLOR_BGR2GRAY);
            matRotated0.copyTo(matProc);
            //matRotated0.copyTo(imTemp);

            Mat M0 = getPerspectiveTransform(src1, src0);
            warpPerspective(matRotated1, matRotated1, M0, matRotated1.size(),INTER_LINEAR, BORDER_CONSTANT,0);

            timer.start();

            if(ui->comboBoxAlg->currentText() == "BM")
            {
                DisparityOCV(matRotated0, matRotated1, &matProc, 0);
            }
            else if(ui->comboBoxAlg->currentText() == "SGBM")
            {
                SGBMCalc();
            }
            else if(ui->comboBoxAlg->currentText() == "Census")
            {
                CensusCalc();
            }
            else if(ui->comboBoxAlg->currentText() == "SAD")
            {
                SADCalc();
            }

            qDebug() << "main process: " << timer.elapsed();
        }

        cam0.isReady = false;
        cam1.isReady = false;
    }
}

void MainWindow::on_pushButtonLoadCalib_clicked()
{
    loadCalib();
}

void MainWindow::on_pushButtonSaveCalib_clicked()
{
    saveCalib();
}

void MainWindow::on_pushButtonLoadFiles_clicked()
{

    tmrTimer->stop();
    ui->pushButtonStartStop->setText("Start");

    path = dir.relativeFilePath("images/");
    QString fn = QFileDialog::getOpenFileName(this, tr("Open image"), path, tr("Image Files(*.png *.jpg *.bmp)"));
    strcpy(cam0.streamSrc, fn.toStdString().c_str());
    ui->lineEditLeft->setText(cam0.streamSrc);
    cam0.fLoad = true;


    fn = QFileDialog::getOpenFileName(this, tr("Open image"), path, tr("Image Files(*.png *.jpg *.bmp)"));
    strcpy(cam1.streamSrc, fn.toStdString().c_str());
    ui->lineEditRight->setText(cam1.streamSrc);
    cam1.fLoad = true;
    tmrTimer->start(100);
    ui->pushButtonStartStop->setText("Stop");
}

void MainWindow::on_comboBoxImgLeft_currentIndexChanged(const QString &arg1)
{
    selectOption(&cam0, ui->comboBoxImgLeft->currentText());
}

void MainWindow::on_comboBoxImgRight_currentIndexChanged(const QString &arg1)
{
    selectOption(&cam1, ui->comboBoxImgRight->currentText());
}

void MainWindow::on_pushButtonStartStop_clicked()
{
    if(ui->pushButtonStartStop->text() == "Start")
    {
        tmrTimer->start(150);
        ui->pushButtonStartStop->setText("Stop");
    }
    else if(ui->pushButtonStartStop->text() == "Stop")
    {
        tmrTimer->stop();
        ui->pushButtonStartStop->setText("Start");
    }
    else{}
}

void MainWindow::on_pushButton_clicked()
{
    tmrTimer->stop();
    ui->pushButtonStartStop->setText("Start");

    path = dir.relativeFilePath("images/");

    QString fn = QFileDialog::getSaveFileName(this, tr("Save image"), path, tr("Image Files(*.png *.jpg *.bmp)"));

    imwrite(fn.toStdString(), matProc);

    tmrTimer->start(150);
    ui->pushButtonStartStop->setText("Stop");
}
