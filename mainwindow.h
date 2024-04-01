#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <unistd.h>
#include "webcam.h"
#include "process.h"
#include "server.h"

using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QTimer *tmrTimer;

    Mat matProc, matRotated0, matRotated1, matTest;

    QImage qimage0, qimage1;

public slots:
    void calibrationCalc();
    void CensusCalc();
    void SADCalc();
    void SGBMCalc();
    void selectOption(webcam *cam, QString textSelect);
    void GetImg();
    void ImageShow(Mat imgOrigin, Mat ImgDisparity);

private slots:
    void on_comboBoxImgLeft_currentIndexChanged(const QString &arg1);

    void on_comboBoxImgRight_currentIndexChanged(const QString &arg1);

    void on_actionOpen_triggered();

    void on_actionSave_Disparity_triggered();

    void on_actionLoad_Calibration_triggered();

    void on_actionSaveCalibration_triggered();

    void on_actionStart_Process_triggered();

private:
    bool flagStart = false;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
