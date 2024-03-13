#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <unistd.h>
#include "webcam.h"
#include "process.h"

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

    Mat matProc, matRotated0, matRotated1;

    QImage qimage0, qimage1;

public slots:
    void calibrationCalc();
    void CensusCalc();
    void SADCalc();
    void SGBMCalc();
    void selectOption(webcam *cam, QString textSelect);
    void GetImg();

private slots:
    void on_pushButtonLoadCalib_clicked();

    void on_pushButtonSaveCalib_clicked();

    void on_pushButtonLoadFiles_clicked();

    void on_comboBoxImgLeft_currentIndexChanged(const QString &arg1);

    void on_comboBoxImgRight_currentIndexChanged(const QString &arg1);

    void on_pushButtonStartStop_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
