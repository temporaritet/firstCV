#include "process.h"

#define CSIZE   16

int disp_range = 50;
int win_size = 5;
//StereoSGBM sgbm;

//StereoBM_GPU *bm = new StereoBM_GPU();


ushort imDiff[I_W*I_H*4];
uint imMin[I_W*I_H*4];
uint imColSum[I_W*4];
uchar imCalib[I_W*I_H*4];
uchar imEdge1[I_W*I_H*4];
uchar imEdge2[I_W*I_H*4];
uchar imFill0[I_W*I_H*4];
uchar imFill1[I_W*I_H*4];
uchar tb_range[I_W*4];
uchar imSegm[I_W*I_H*4];
uint imCens1[I_W*I_H*32*4], imCens2[I_W*I_H*32*4];
ushort wordbits[65536*4];

uint Spectr[1024*4];
uchar imTest[I_W*I_H*3*4];


Mat matImPrev, matImMotion;
Ptr<StereoSGBM> sgbm = StereoSGBM::create(0,16,3);

Point2f src0[] = {Point2f(0.0f, 0.0f),Point2f(320.0f, 0.0f),Point2f(0.0f, 240.0f),Point2f(320.0f, 240.0f)};
Point2f src1[] = {Point2f(0.0f, 0.0f),Point2f(320.0f, 0.0f),Point2f(0.0f, 240.0f),Point2f(320.0f, 240.0f)};
Point2f dst[] = {Point2f(0.0f, 0.0f),Point2f(320.0f, 0.0f),Point2f(0.0f, 240.0f),Point2f(320.0f, 240.0f)};

//--------------------------------------------------------------------------------

template <typename anyType>
anyType abs(anyType val)
{
    return(val < 0)? -val:val;
}
template <typename countType>
uint countbits(countType value)
{
    value = value - ((value >> 1) & 0x55555555);
    value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
    return (((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}
inline int countbits32(ushort value)
{
    return( wordbits[value] );
}
void countbits_init()
{
    for(int i = 0; i < 65536; i ++)
    {
        wordbits[i] = countbits<ushort>((ushort)i);
    }
}
//--------------------------------------------------------------------------------
void motion(Mat *matIm)
{
    if(matIm->empty()==true)
    {
        return;
    }
    Mat matImCurr;
    matIm->copyTo(matImCurr);

    cvtColor(*(matIm), matImCurr, COLOR_BGR2GRAY);

    static bool isStarted = false;
    if(isStarted == false)
    {
        matImCurr.copyTo(matImPrev);
        isStarted = true;
    }
    matImCurr.copyTo(matImMotion);
    uchar *imCurr, *imPrev, *imMot;

    int x, y, count = 0;

    for(y = 0; y < matImCurr.rows; y ++)
    {
        imCurr = (uchar*)&(matImCurr.data)[y*(matImCurr.cols)];
        imPrev = (uchar*)&matImPrev.data[y*matImPrev.cols];
        imMot = (uchar*)&matImMotion.data[y*matImMotion.cols];

        for(x = 0; x < matImCurr.cols; x ++)
        {
            if(abs((int)(imCurr[x] - imPrev[x])) > 50)
            {
                imMot[x] = 255;
                count ++;
            }
            else
            {
                imMot[x] = 0;
            }
            imPrev[x] = imCurr[x];
        }
    }
}
//--------------------------------------------------------------------------------
void getCalib(uchar *im1, uchar *im2)
{

}
//--------------------------------------------------------------------------------
void getDisparity(uchar *im1, uchar *im2, uchar *imDisp)
{
    uchar *pim1 = im1;
    uchar *pim2 = im2;
    uint *pMin;
    uchar *pDisp;
    ushort *pimDiff1, *pimDiff2;
    uint *pimCol1, *pimCol2;
    int x, y, d, i;
    uint sum;
    int p = 1;

    memset(imMin, -1, sizeof(imMin));//set maximum posible value of uint

    getEdge(im1, im2);

    for(d = 0; d < disp_range; d ++)
    {
        tb_range[d] = (d * 255)/(disp_range);
    }

    for (d = 0; d < disp_range; d ++)
    {
        p = ((d*4)/80)+1;

        //get the difference between left and right images
        for (y = win_size; y < (IMG_HEIGHT - win_size); y += STEP)
        {
            pim1 = (uchar*)&im1[I_W*y];
            pim2 = (uchar*)&im2[I_W*(y)]+d-6;

            for (x = 0; x < IMG_WIDTH; x ++)
            {
                imDiff[x+(I_W*y)] = (ushort)abs<short>( ((short)*pim1) - ((short)*pim2) );
                pim1 ++;
                pim2 ++;

                if(d == 0)
                {
//                    if(imDiff[x+(I_W*y)] > 20)
//                    {
//                        imEdge1[x+(I_W*y)] = 0;
//                    }
                }
            }
        }

        //get the sum of culumn for first line
        for (x = 0; x < IMG_WIDTH; x ++)
        {
            imColSum[x] = 0;
            for(i = 0; i <= (win_size*2); i ++)
            {
                imColSum[x] += imDiff[x+(I_W*(i))];
            }
        }

        pimCol1 = (uint*)(&imColSum)+win_size+1;
        pimCol2 = (uint*)(&imColSum)-win_size;
        for (y = win_size; y < (IMG_HEIGHT - win_size); y ++)
        {
            sum = 0;
            for(i = 0; i <= (win_size*2); i ++)
            {
                sum += imColSum[i];
            }

            pMin = (uint*)&imMin[I_W*y];
            pDisp = (uchar*)&imDisp[I_W*y];

            //get the sum of calculated column to get window sum only for first pixel on row
            for (x = win_size; x < (IMG_WIDTH - win_size - d); x ++)
            {
                //if the sum of window less than sum in buffer assign new point
                if(sum < *pMin)
                {
                    *pMin = sum;

                    //if the difference is too big exclude this point
                    if( 1/*(imEdge1[x+(I_W*y)] != 0)*/ )
                    {
                      pDisp[x] = tb_range[d];
                    }
                    else
                    {
                        pDisp[x] = 0;
                    }
                }
                //pDisp ++;
                pMin ++;

                //subtract the last column from queue and add next column
                sum += pimCol1[x] - pimCol2[x];
            }
            //get the sum of line for next step
            pimDiff1 = (ushort*)&imDiff[I_W*(y-win_size)];//previous line have to be substracted
            pimDiff2 = (ushort*)&imDiff[I_W*(y+win_size+1)];//next line have to be added
            for (x = 0; x < (IMG_WIDTH); x ++)
            {
                imColSum[x] += ((*pimDiff2) - (*pimDiff1));
                pimDiff1 ++;
                pimDiff2 ++;
            }
        }
      }
    for (int y = 6; y < (IMG_HEIGHT-6); y +=1)
    {
        for (int x = 6; x < IMG_WIDTH-6; x +=1)
        {
            //imTemp.data[x + y*I_W] = abs((im1[x + y*I_W -imDisp[x + y*I_W]] - im2[x + y*I_W]));
//            if(imTemp.data[x + y*I_W] >= 10)
//            {
//                imDisp[x + y*I_W] = 0;
//            }
        }
    }
    //memcpy(im1, imCalib, I_H*I_W*sizeof(uchar));

}
//--------------------------------------------------------------------------------
void imgOverlap(uchar *im1, uchar *im2, uchar *imResult)
{
    for(int y = 0; y < I_H; y ++)
    {
        //im1 = (uchar*)&matRotated0.data[y*I_W];
        //im2 = (uchar*)&matRotated1.data[y*I_W];
        for(int x = 0; x < I_W; x ++)
        {
            imResult[x + y*I_W] = ((int)im1[x + y*I_W] + im2[x + y*I_W]) / 2;
        }
    }
}
//--------------------------------------------------------------------------------
void getCensus(uchar *im1, uchar *im2, uchar *imDisp)
{
    uchar *pim1 = im1;
    uchar *pim2 = im2;
    uint *pMin;
    uchar *pDisp;
    uint *pimC1, *pimC2;
    uint *pimCol1, *pimCol2;
    int x, y, d, i, j;
    uint cnt, cnt2, cnt3, cnt4;
    uint wins, offs;
    uint Sens1, sum1;
    uint Sens2, sum2;
    int c = 1;

    offs = 6;
    wins = (2*offs+1)*(2*offs+1);

    //getEdge(im1, im2);
    //getDisparity(im1, im2, imFill0);
    //memset(imDisp, 0, I_W*I_H);
    //memset(imFill0, 0, I_W*I_H);
    memset(imMin, 0xFFFFFF, sizeof(imMin));
    //memset(imCens1, 0, I_W*I_H);
    //memset(imCens2, 0, I_W*I_H);

//    for (y = win_size; y < (IMG_HEIGHT - win_size); y ++)
//    {
//        pim1 = (uchar*)&im1[3*I_W*y];
//        pim2 = (uchar*)&im2[3*I_W*(y)];

//        for (x = 0; x < (IMG_WIDTH*3); x +=3)
//        {
////            if((ushort)abs<short>( ((short)*pim1) - ((short)*pim2) ) > 40 )
////            {
////                imEdge1[x+(I_W*y)] = 0;
////            }
////            *pim1 &= 0xF0;
////            *pim2 &= 0xF0;
//            imTemp0[x/3+(I_W*y)] = pim1[0]*pim1[1]*pim1[2];
//            imTemp1[x/3+(I_W*y)] = pim2[0]*pim2[1]*pim2[2];
//            pim1 += 3;
//            pim2 += 3;
//        }
//    }

    for(d = 0; d < disp_range; d ++)
    {
        tb_range[d] = (d * 255)/(disp_range-4);
    }

//    for (y = offs; y < (IMG_HEIGHT-offs); y +=1)
//    {
//        pim1 = (uchar*)&im1[I_W*y];
//        pim2 = (uchar*)&im2[I_W*(y)];

//        for (x = offs; x < IMG_WIDTH-offs; x +=1)
//        {
//            for(i = y-8, c = 0; i <= y+8; i +=1, c ++)
//            {
//                Sens1 = Sens2 = 0;
//                for(j = x-8; j < x+8; j +=1)
//                {
//                    imFill0[x+I_W*y] = im1[j+(i*I_W)];
//                    imFill1[x+I_W*y] = im2[j+(i*I_W)];
//                }
//            }
//        }
//    }

    for (y = offs; y < (IMG_HEIGHT-offs); y +=1)
    {
        pim1 = (uchar*)&im1[I_W*y];
        pim2 = (uchar*)&im2[I_W*(y)];

        for (x = offs; x < IMG_WIDTH-offs; x +=1)
        {
            /* get average value from 3x3 window (image 1) */
            Sens1 = 0;
            sum1 = pim1[x-1-I_W] + pim1[x-I_W] + pim1[x+1-I_W];
            sum1 += pim1[x-1] + pim1[x] + pim1[x+1];
            sum1 += pim1[x-1+I_W] + pim1[x+I_W] + pim1[x+1+I_W];
            sum1 /= 9;

            /* get average value from 3x3 window (image 2) */
            Sens2 = 0;
            sum2 = pim2[x-1-I_W] + pim2[x-I_W] + pim2[x+1-I_W];
            sum2 += pim2[x-1] + pim2[x] + pim2[x+1];
            sum2 += pim2[x-1+I_W] + pim2[x+I_W] + pim2[x+1+I_W];
            sum2 /= 9;

            pimC1 = (uint*)&imCens1[(x*CSIZE) + (CSIZE*I_W*y)];
            pimC2 = (uint*)&imCens2[(x*CSIZE) + (CSIZE*I_W*y)];

            /* Calculate census transform for both images */
            for(i = y-8, c = 0; i <= y+8; i +=2, c ++)
            {
                Sens1 = Sens2 = 0;
                for(j = x-8; j < x+8; j +=2)
                {
                    Sens1 <<= 1;
                    if(sum1 > im1[j+(i*I_W)])
                    {
                        /* if pixel less than average */
                         Sens1 |= 1;
                    }
                    Sens2 <<= 1;
                    if(sum2 > im2[j+(i*I_W)])
                    {
                        /* if pixel less than average */
                         Sens2 |= 1;
                    }
                }
                /* store bit values for each row in window */
                pimC1[c] = Sens1;
                pimC2[c] = Sens2;
            }

        }
    }

    for (y = offs; y < (IMG_HEIGHT-offs); y +=1)
    {
        for (x = offs; x < IMG_WIDTH-offs; x +=1)
        {
            if(im1[x + (I_W*y)] > 4)
            {
                pimC1 = (uint*)&imCens1[(x*CSIZE) + (CSIZE*I_W*y)];
                pimC2 = (uint*)&imCens2[((x-4)*CSIZE) + (CSIZE*I_W*y)];
                uint* pMin = (uint*)&imMin[x + y*I_W];
                uchar* pDisp = (uchar*)&imDisp[x + y*I_W];
                uchar pDMin = 0;
                for(d = 0; d < disp_range; d +=1)
                {
                    //calculate for odd numbers
                    if(1/*abs(imSeg0[x+(y*I_W)] - imSeg1[x+(y*I_W)]) < 50 */)
                    {
                        cnt = 0;
                        for(c = 0; c <= 8; c ++)
                        {
                            cnt += wordbits[(pimC1[c])^(pimC2[c])];
                        }

                        pimC2 += CSIZE;
                        if( cnt < *pMin )
                        {
                            *pMin = cnt;
                            pDMin = d;
    //                        if(cnt < 160)
    //                        {
                                *pDisp = tb_range[d];
    //                        }
    //                        else
    //                        {
    //                            *pDisp = 0;
    //                        }
                        }
                    }
                    else
                    {
                        *pDisp = 0;
                    }
                }
//                //check for previous pixel
//                pimC2 = (uint*)&imCens2[((x+pDMin-5)*CSIZE) + (CSIZE*I_W*y)];

//                cnt = 0;
//                for(c = 0; c <= 8; c ++)
//                {
//                    cnt += countbits32((pimC1[c])^(pimC2[c]));
//                }

//                if( cnt < *pMin )
//                {
//                    *pMin = cnt;
//                    *pDisp = tb_range[pDMin-1];
//                }

//                //check for next pixel
//                pimC2 = (uint*)&imCens2[((x+pDMin-3)*CSIZE) + (CSIZE*I_W*y)];

//                cnt = 0;
//                for(c = 0; c <= 8; c ++)
//                {
//                    cnt += countbits32((pimC1[c])^(pimC2[c]));
//                }

//                if( cnt < *pMin )
//                {
//                    *pMin = cnt;
//                    *pDisp = tb_range[pDMin+1];
//                }

            }
            else
            {
                imDisp[x + y*I_W] = 0;
            }
        }
    }
//    for (int y = 6; y < (IMG_HEIGHT-6); y +=1)
//    {
//        for (int x = 6; x < IMG_WIDTH-6; x +=1)
//        {
//            imTemp.data[x + y*I_W] = abs((im1[x + y*I_W -imDisp[x + y*I_W]] - im2[x + y*I_W]));
//            if(imTemp.data[x + y*I_W] >= 100)
//            {
//                imDisp[x + y*I_W] = 0;
//            }
//        }
//    }


}
//--------------------------------------------------------------------------------
void getEdge(uchar *im1, uchar *im2)
{
    int diff1;
    int x, y, i, j;

    for (y = 0; y < IMG_HEIGHT; y ++)
    {
        for (x = win_size; x < (IMG_WIDTH - win_size); x ++)
        {
            diff1 = 0;
            for(i = (y-1); i <= (y+1); i ++)
            {
                for(j = (x-2); j <= (x+2); j ++)
                {
                    diff1 += abs<short>(im1[(x)+(I_W*y)] - im1[(j)+(I_W*(i))]);
                }
            }

            if(diff1 > 10)
            {
                imEdge1[x + (I_W*y)] = 255;
            }
            else
            {
                imEdge1[x + (I_W*y)] = 0;
            }
        }
    }
}
//--------------------------------------------------------------------------------
void convertToGray(QImage src, QImage *dst)
{
    *dst = QImage(src.width(), src.height(), QImage::Format_Indexed8);
    QVector<QRgb> table (256);
    for (int i = 0; i < 256; ++i)
    {
        table[i] = qRgb(i, i, i);
    }
    dst->setColorTable(table);
    for(int i = 0; i < src.width(); i ++)
    {
        for(int j = 0; j < src.height(); j ++)
        {
            QRgb value = src.pixel(i, j);
            dst->setPixel(i, j, qGray(value));
        }
    }
    *dst = dst->copy(0, 0, src.width(), src.height());
}
//--------------------------------------------------------------------------------
void loadCalib()
{
    FILE *fcfg;
    float ftemp = 0.0;
    int i, j;

    fcfg = fopen("calib.txt", "r");

    if(fcfg != NULL)
    {
        for(i = 0; i < 4; i ++)
        {
            (void)fscanf(fcfg, "%f", &ftemp);
            src0[i].x = ftemp;
            //qDebug() <<"Read value X1:" << src0[i].x <<"    i: "<< i;

            (void)fscanf(fcfg, "%f", &ftemp);
            src1[i].x = ftemp;
            //qDebug() <<"Read value X2:" << src1[i].x <<"    i:"<< i;

            (void)fscanf(fcfg, "%f", &ftemp);
            src0[i].y = ftemp;
            //qDebug() <<"Read value Y1:" << src0[i].y <<"    i:"<< i;

            (void)fscanf(fcfg, "%f", &ftemp);
            src1[i].y = ftemp;
            //qDebug() <<"Read value Y2:" << src1[i].y <<"    i:"<< i;
        }
        fclose(fcfg);
    }
    else
    {
        qDebug() << "there is no calibration file";
    }
}
//--------------------------------------------------------------------------------
void saveCalib()
{
    FILE *fcfg;
    int i, j;
    fcfg = fopen("calib.txt", "w");

    for(i = 0; i < 4; i ++)
    {
        fprintf(fcfg, " %6.3f %6.3f %6.3f %6.3f \n", src0[i].x, src1[i].x, src0[i].y, src1[i].y);
    }

    fclose(fcfg);
}
//--------------------------------------------------------------------------------
void color_init()
{
    int i, j;
    Mat colorTbl = imread("images/color_table.bmp", IMREAD_COLOR);

    if(!colorTbl.empty())
    {
        for(i = 0, j = 0; i < 255; i ++, j += 3)
        {
            Spectr[i] = colorTbl.data[j] + (colorTbl.data[j+1]<<8) + (colorTbl.data[j+2]<<16);

        }
        //imshow("spectrum", colorTbl);
        qDebug() << "Spectr colors initialized" << colorTbl.data[0];
    }
    else
    {
        //qDebug() << "calculated colors" << colorTbl.data[0];
        int temp[100];

        for ( i = 0; i < 1024; i ++)
        {
            if (i < 256)
            {
                temp[i*disp_range/1024] = RGB(0,i,255);//i=[0:255]
            }
            else if(i < 512)
            {
                temp[i*disp_range/1024] = RGB(0 ,255, (255-(i-256)));//i=[256:511]
            }
            else if(i < 768)
            {
                temp[i*disp_range/1024] = RGB((i-512) ,255, 0);//i=[512:767]
            }
            else
            {
                temp[i*disp_range/1024] = RGB(255 ,(255-(i-768)), 0);//i=[768:1024]
            }
        }
        for(i = 1; i < disp_range; i ++)
        {
            Spectr[i] = temp[i];
        }
        Spectr[0] = 0;

    }
    Spectr[0] = 0;
}
//--------------------------------------------------------------------------------
void colorDisparity(uchar *Disp, uchar *img)
{
    int x, y, d;
    uchar *pD, *pC;

    for (y = win_size; y<(IMG_HEIGHT-win_size); y ++)
    {
        pD = (uchar*)&Disp[y*I_W]; // black-white disparity image
        pC = (uchar*)&img[y*I_W*3];// colored disparity image

        for (x = win_size; x<((IMG_WIDTH-win_size)); x ++)
        {
            //d = pD[x]*(disp_range)/255;
            pC[0] = Spectr[pD[x]]&0xFF;
            pC[1] = (Spectr[pD[x]]&0xFF00)>>8;
            pC[2] = (Spectr[pD[x]]&0xFF0000)>>16;

            pC += 3;
        }
    }
}
//--------------------------------------------------------------------------------
void DisparityOCV(Mat img1, Mat img2, Mat *disp8, uchar method)
{
    Mat disp;
    Mat im1, im2, imd;

    if(img1.empty() || img2.empty())
    {
        return;
    }

    if (method == 0)
    {
        /*StereoBM sbm;
        sbm.state->SADWindowSize = 9;
        sbm.state->numberOfDisparities = 112;
        sbm.state->preFilterSize = 5;
        sbm.state->preFilterCap = 61;
        sbm.state->minDisparity = -39;
        sbm.state->textureThreshold = 507;
        sbm.state->uniquenessRatio = 0;
        sbm.state->speckleWindowSize = 0;
        sbm.state->speckleRange = 8;
        sbm.state->disp12MaxDiff = 1;
        sbm(img1, img2, disp);*/
//        bm->preset = gpu::StereoBM_GPU::BASIC_PRESET;
//        bm->ndisp = 48;
//        bm->winSize = 5;
//        //bm->operator()(img1, img2, disp);
//        normalize(disp, *disp8, 0, 255, CV_MINMAX, CV_8U);
    }
    else
    {
        sgbm->compute(img2, img1, disp);
    }
    normalize(disp, *disp8, 0, 255, CV_MINMAX, CV_8U);
//    for (int y = 6; y < (IMG_HEIGHT-6); y +=1)
//    {
//        for (int x = 6; x < IMG_WIDTH-6; x +=1)
//        {
//            imTemp.data[x + y*I_W] = abs((img1.data[x + y*I_W -disp8->data[x + y*I_W]] - img2.data[x + y*I_W]));
//            if(imTemp.data[x + y*I_W] >= 50)
//            {
//                disp8->data[x + y*I_W] = 0;
//            }
//        }
//    }

}

