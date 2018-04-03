#ifndef __POINT_CLOUD_ESTIMATE_H
#define __POINT_CLOUD_ESTIMATE_H

#include "MeshDoc.h"


#define _USE_OPENMP_

struct SRT {
    double scale;
    vcg::Matrix44f rotation;
    vcg::Point3f translation;
};

bool PCEst(const char *SPath, const char *RPath, const char *OutPath, const bool bReg = true);

MeshModel* PCLoad(const char *Path);
bool PCSave(MeshModel *MM, const char *Path);
void PCTransform(MeshModel *MM, const SRT &T);

MeshModel* PCSample(
    MeshModel *SrtMM, 
    const int SampleNum, const char *_OutPath = 0);

SRT PCRegister(
    MeshModel *SMM, MeshModel *RMM,
    const int ND = 0, const int NR = 0);
double PCNearestDist(
    MeshModel *SMM, MeshModel *RMM,
    double *sigma = 0, const char *OutFile = 0);
void PCSaveErrorCoded(
    MeshModel *SMM, MeshModel *RMM, const double maxe, 
    const char *OutFileJet, const char *OutFileClr = 0);
#endif // __POINT_CLOUD_ESTIMATE_H
