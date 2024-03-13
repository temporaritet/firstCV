#include "webcam.h"

webcam::webcam()
{
    option = 0;
    streamSrc = new char[PATH_MAX];
    strcpy(streamSrc, "");
    isReady = false;
    fLoad = 0;
}
webcam::webcam(uint camId) : camId(camId)
{
    option = 0;
    streamSrc = new char[PATH_MAX];
    strcpy(streamSrc, "");
    isReady = false;
    fLoad = 0;
}
webcam::webcam(uint camId, const char *streamSource) : camId(camId), streamSrc((char*)streamSource)
{
    option = 0;
    streamSrc = new char[PATH_MAX];
    strcpy(streamSrc, streamSource);
    isReady = false;
    fLoad = 0;
}
webcam::~webcam()
{
    captureCam.release();
    delete []streamSrc;
    evClose = true;
    qDebug() << "thread exit: " << camId;
}

void webcam::openStream()
{
    /* wait 30ms */
    msleep(30);

    /* open video stream */
    if(fLoad == 0)
    {
        if(camId == 0)
        {
            strcpy(streamSrc, "images/woodL.bmp");
        }
        else
        {
            strcpy(streamSrc, "images/woodR.bmp");
        }
    }

    switch (option)
    {
        case 0:
            captureCam.open("/dev/video1");
            break;
        case 1:
            captureCam.open("/dev/video2");
            break;
        case 2:
        case 3:
            captureCam.open(streamSrc);
            break;
        default:break;
    }
    /* Open webcam */
    //captureCam.open(streamSrc/*"http://192.168.0.11:8081/?action=stream"*/);

    if(captureCam.isOpened())
    {
        captureCam.set(CV_CAP_PROP_FRAME_WIDTH,320);
        captureCam.set(CV_CAP_PROP_FRAME_HEIGHT,240);
    }
}

void webcam::run()
{
    evClose = false;
    while(!evClose)
    {
        if(!captureCam.isOpened())
        {
            openStream();
        }
        else
        {
            /* check image frame was processed */
            if(isReady == false)
            {
                if(captureCam.read(matIm))
                {
                    if(!matIm.empty())
                    {
                        //GaussianBlur(matIm, matIm, Size(3, 3), 0, 0);
                        //cvtColor(matIm, matIm, COLOR_BGR2RGB);
                        //qimage = QImage((uchar*)matIm.data, matIm.cols, matIm.rows, matIm.step, QImage::Format_RGB888);

                        isReady = true;
                    }
                }
                else
                {
                    captureCam.release();
                }
            }
        }
        msleep(4);
    }
}

void webcam::thClose()
{
    evClose = true;
}
