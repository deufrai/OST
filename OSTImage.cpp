/*
 * Thanks to
 * original code can be found here :
 * https://www.lost-infinity.com/night-sky-image-processing-part-1-noise-reduction-using-anisotropic-diffusion-with-c/
 * https://github.com/carsten0x51h/astro_tools
 *
 * we might have to give a try with this one, same author :
 * https://github.com/carsten0x51h/focus_finder
 *
 *
 */
#include <basedevice.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <cmath>
#include <fitsio.h>
#include <tuple>
#include <functional>
#include <list>
#include <set>
#include <array>
#include <vector>
#include "OSTImage.h"



OSTImage::OSTImage() {
    ResetData();
}

OSTImage::~OSTImage() {
    ResetData();
}
void OSTImage::ResetData(void) {

}
bool OSTImage::saveToJpeg(QString filename) {

    img.save_jpeg(filename.toStdString().c_str(),100);
    return true;
}

bool OSTImage::LoadFromBlob(IBLOB *bp)
{
    IDLog("IMG readblob %s %s %i \n",bp->label,bp->name,bp->size);
    ResetData();
    int status = 0, anynullptr = 0;
    long naxes[3];

    fitsfile *fptr;  // FITS file pointer
    size_t bsize = static_cast<size_t>(bp->bloblen);
    // load blob to CFITSIO
    if (fits_open_memfile(&fptr,"",READONLY,&bp->blob,&bsize,0,NULL,&status) )
    {
        IDLog("IMG Unsupported type or read error loading FITS blob\n");
        return false;
    } else {
        stats.size = bp->bloblen;
    }

    // Use open diskfile as it does not use extended file names which has problems opening
    // files with [ ] or ( ) in their names.
    /*QString fileToProcess;
    fileToProcess = "/home/gilles/ekos6/Light/lum/M_33_Light_lum_60_secs_2020-11-06T22-25-13_139.fits";
    if (fits_open_diskfile(&fptr,fileToProcess.toLatin1() , READONLY, &status))
    {
        IDLog("Unsupported type or read error loading FITS blob\n");
        return false;
    }
    else
        stats.size = QFile(fileToProcess).size();*/

    if (fits_movabs_hdu(fptr, 1, IMAGE_HDU, &status))
    {
        IDLog("IMG Could not locate image HDU.\n");
        fits_close_file(fptr, &status);
        return false;
    }

    int fitsBitPix = 0;
    if (fits_get_img_param(fptr, 3, &fitsBitPix, &(stats.ndim), naxes, &status))
    {
        IDLog("IMG FITS file open error (fits_get_img_param).\n");
        fits_close_file(fptr, &status);
        return false;
    }

    if (stats.ndim < 2)
    {
        IDLog("IMG 1D FITS images are not supported.\n");
        fits_close_file(fptr, &status);
        return false;
    }

    switch (fitsBitPix)
    {
        case BYTE_IMG:
            stats.dataType      = TBYTE;
            stats.bytesPerPixel = sizeof(uint8_t);
            break;
        case SHORT_IMG:
            // Read SHORT image as USHORT
            IDLog("SHORT_IMG\n");
            stats.dataType      = TUSHORT;
            stats.bytesPerPixel = sizeof(int16_t);
            break;
        case USHORT_IMG:
            stats.dataType      = TUSHORT;
            stats.bytesPerPixel = sizeof(uint16_t);
            break;
        case LONG_IMG:
            // Read LONG image as ULONG
            stats.dataType      = TULONG;
            stats.bytesPerPixel = sizeof(int32_t);
            break;
        case ULONG_IMG:
            stats.dataType      = TULONG;
            stats.bytesPerPixel = sizeof(uint32_t);
            break;
        case FLOAT_IMG:
            stats.dataType      = TFLOAT;
            stats.bytesPerPixel = sizeof(float);
            break;
        case LONGLONG_IMG:
            stats.dataType      = TLONGLONG;
            stats.bytesPerPixel = sizeof(int64_t);
            break;
        case DOUBLE_IMG:
            stats.dataType      = TDOUBLE;
            stats.bytesPerPixel = sizeof(double);
            break;
        default:
            IDLog("IMG Bit depth %i is not supported.\n",fitsBitPix);
            fits_close_file(fptr, &status);
            return false;
    }

    if (stats.ndim < 3)
        naxes[2] = 1;

    if (naxes[0] == 0 || naxes[1] == 0)
    {
        IDLog("IMG Image has invalid dimensions %lix %li\n",naxes[0],naxes[1]);
        return false;
    }

    stats.width               = static_cast<uint16_t>(naxes[0]);
    stats.height              = static_cast<uint16_t>(naxes[1]);
    stats.channels            = static_cast<uint8_t>(naxes[2]);
    stats.samples_per_channel = stats.width * stats.height;

    delete[] m_ImageBuffer;
    m_ImageBuffer = nullptr;

    m_ImageBufferSize = stats.samples_per_channel * stats.channels * static_cast<uint16_t>(stats.bytesPerPixel);
    m_ImageBuffer = new uint8_t[m_ImageBufferSize];
    IDLog("--------------------------\n");
    IDLog("m_ImageBufferSize %i bytes\n"        ,m_ImageBufferSize);
    IDLog("stats.samples_per_channel %i bytes\n",stats.samples_per_channel);
    IDLog("stats.channels %i bytes\n"           ,stats.channels);
    IDLog("stats.bytesPerPixel %i bytes\n"      ,stats.bytesPerPixel);
    IDLog("--------------------------\n");

    if (m_ImageBuffer == nullptr)
    {
        IDLog("IMG FITSData: Not enough memory for image_buffer channel. Requested: %i bytes\n",m_ImageBufferSize);
        delete[] m_ImageBuffer;
        m_ImageBuffer = nullptr;
        fits_close_file(fptr, &status);
        return false;
    }

    long nelements = stats.samples_per_channel * stats.channels;

    //long int firstpixel =1;
    //if (fits_read_img(fptr,TUSHORT,1,nelements,nullptr,m_ImageBuffer,&anynullptr,&status))
    /*if (fits_read_pix(fptr, static_cast<uint16_t>(stats.dataType), &firstpixel, nelements, nullptr, m_ImageBuffer, &anynullptr, &status))
    {
        IDLog("Error reading imag. %i\n",status);
        fits_close_file(fptr, &status);
        return false;
    }*/
    //if (fits_read_pix(fptr, TUSHORT                              , fpixel, xsize*ysize, nullptr, ImageData, nullptr, &status))
    if (fits_read_img(fptr, static_cast<uint16_t>(stats.dataType), 1, nelements, nullptr, m_ImageBuffer, &anynullptr, &status))
    {
        IDLog("IMG Error reading imag. %i\n",status);
        fits_close_file(fptr, &status);
        return false;
    }

    fits_close_file(fptr,&status);
    // Load image into Cimg object
    img=nullptr;
    img.clear();
    img.resize(stats.width ,stats.height,1,1);
    cimg_forXY(img, x, y)
        {
            img(x, img.height() - y - 1) = (reinterpret_cast<uint16_t *>(m_ImageBuffer))[img.offset(x, y)]; // FIXME ???
        }

    CalcStats();
    saveToJpeg("/home/gilles/OST/toto.jpeg");
    FindStars();
    IDLog("IMG readblob done %ix%i\n",stats.width ,stats.height );
    return true;
}

void OSTImage::CalcStats(void)
{
    stats.min[0] =img.min();
    stats.max[0] =img.max();
    stats.mean[0]=img.mean();
    stats.median[0]=img.median();
    stats.stddev[0]=sqrt(img.variance(1));
    IDLog("IMG Min=%f Max=%f Avg=%.2f Med=%f StdDev=%.2f\n",stats.min[0],stats.max[0],stats.mean[0],stats.median[0],stats.stddev[0]);
}

void OSTImage::FindStars(void)
{
    HFRavg=99;
    stellarSolver = new StellarSolver(stats, m_ImageBuffer,this);
    stellarSolver->moveToThread(this->thread());
    stellarSolver->setParent(this);
    connect(stellarSolver,&StellarSolver::logOutput,this,&OSTImage::sslogOutput);
    connect(stellarSolver,&StellarSolver::ready,this,&OSTImage::ssReady);
    stellarSolver->setLogLevel(LOG_ALL);
    stellarSolver->setSSLogLevel(LOG_VERBOSE);
    //printf("--------%s\n",stellarSolver->getCommandString().toStdString().c_str());
    //printf("%s\n",stellarSolver->getCommandString().data());
    //printf("%s\n",stellarSolver->getCommandString().toUtf8().data());
    //stellarSolver.m_ExtractorType
    /*typedef enum { EXTRACT,            //This just sextracts the sources
                   EXTRACT_WITH_HFR,   //This sextracts the sources and finds the HFR
                   SOLVE                //This solves the image
                 } ProcessType;

    typedef enum { EXTRACTOR_INTERNAL, //This uses internal SEP to Sextract Sources
                   EXTRACTOR_EXTERNAL,  //This uses the external sextractor to Sextract Sources.
                   EXTRACTOR_BUILTIN  //This uses whatever default sextraction method the selected solver uses
                 } ExtractorType;

    typedef enum { SOLVER_STELLARSOLVER,    //This uses the internal build of astrometry.net
                   SOLVER_LOCALASTROMETRY,  //This uses an astrometry.net or ANSVR locally on this computer
                   SOLVER_ASTAP,            //This uses a local installation of ASTAP
                   SOLVER_ONLINEASTROMETRY  //This uses the online astrometry.net or ASTAP
                 } SolverType;    */
    stellarSolver->setProperty("ProcessType",EXTRACT_WITH_HFR);
    stellarSolver->setProperty("ExtractorType",EXTRACTOR_INTERNAL);
    stellarSolver->setProperty("SolverType",SOLVER_STELLARSOLVER);
    //stellarSolver->clearSubFrame();
    //stellarSolver->extract(true);
    stellarSolver->start();
    IDLog("IMG stellarSolver Start\n");


}

void OSTImage::sslogOutput(QString text)
{
    IDLog("IMG Stellarsolver log : %s\n",text.toUtf8().data());
}
void OSTImage::ssReady(void)
{
    IDLog("IMG stellarSolver ready\n");
    stars = stellarSolver->getStarList();
    IDLog("IMG got %i stars\n",stars.size());
    for (int i=0;i<stars.size();i++)
    {
        //IDLog("IMG HFR %i %f\n",i,stars[i].HFR);
        HFRavg=(i*HFRavg + stars[i].HFR)/(i+1);
    }
    IDLog("IMG HFRavg %f\n",HFRavg);
    emit success();

}

