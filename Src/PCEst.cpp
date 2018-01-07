#include "PCEst.h"

#include <cpd/cpd.h>
#include "flog.h"
#include "JetColorMap.h"

#ifdef _USE_OPENMP_
#include <omp.h>
#endif

#include <QTime>

#include <wrap/io_trimesh/export.h>
#include <wrap/io_trimesh/import.h>
#include <vcg/space/index/kdtree/kdtree.h>
#include <vcg/complex/algorithms/point_sampling.h>
#include <vcg/complex/algorithms/update/normal.h>
#include <vcg/complex/algorithms/update/position.h>

using namespace vcg;
using namespace tri;

namespace GlobParam
{
    const int    _GSampleNum     = 1000;
    const int    _GTryDirectionN = 6;
    const int    _GTryRotationN  = 4;
    const double _GMaxError      = 0.05;
    //==============================
    QTime GTimer;
    //===============================
    void printLogo()
    {
        FLOG::flog(
            "\n\n"
            "    ______  ______         ______  ______  _______   |  PCEst - Point Cloud Estimation   \n"
            "   /  _  / / ____/  ___   / ____/ /  ___/ /__  __/   |  Ver. 1.0.0                       \n"
            "  / ____/ / /___   /__/  / ___/  /___  /    / /      |  January 2018 @ IPC.BUAA          \n"
            " /_/     /_____/        /_____/ /_____/    /_/       |  By WeiQM                         \n"
            "                                                     |  Email: weiqm@buaa.edu.cn         \n"
            "\n"
            "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
            "\n"
        );
    }
    void printParams()
    {
        FLOG::flog(
            " +------------------------------------------------------+\n"
            " | PARAMETERS IN USE:                                   |\n"
            " |------------------------------------------------------|\n"
            " | [_GSampleNum                 ]:       < %000008d >   |\n"
            " | [_GTryDirectionN             ]:       < %000008d >   |\n"
            " | [_GTryRotationN              ]:       < %000008d >   |\n"
            " | [_GMaxError                  ]:       < %0008.3f >   |\n"
            " +------------------------------------------------------+\n",
            _GSampleNum, _GTryDirectionN, _GTryRotationN,
            _GMaxError);
    }
    void setThreadNum(const int nThread)
    {
#ifdef _USE_OPENMP_
        int nThreadMax = omp_get_max_threads();
        int nThreadUsed = (nThread <= 0 || nThread > nThreadMax) ? nThreadMax : nThread;
        omp_set_num_threads(nThreadUsed);
        FLOG::flog(
            " +------------------------------------------------------+\n"
            " |          USE OPENMP: %02d THREAD(s) ARE USED.          |\n"
            " +------------------------------------------------------+\n",
            nThreadUsed);
#endif 
    }
}

cpd::Matrix _PC2Matrix(MeshModel *MM)
{
    if (0 == MM || MM->cm.vn == 0)
        return cpd::Matrix(0, 0);

    CMeshO &mesh = MM->cm;
    const int N = mesh.vert.size();
    cpd::Matrix A(N, 3);

#ifdef _USE_OPENMP_
#pragma omp parallel for
#endif
    for (int i = 0; i < N; i++) {
        vcg::Point3f P = mesh.vert[i].cP();
        A(i, 0) = P.X();
        A(i, 1) = P.Y();
        A(i, 2) = P.Z();
    }

    return A;
}
vcg::Matrix44f _RotMatrix(double AngZ, double AngA)
{
    vcg::Point3f ZAxis(0.0, 0.0, 1.0);
    vcg::Point3f RAxis(1.0, 0.0, 0.0);

    vcg::Matrix33f RZ;
    RZ.SetRotateDeg(AngZ, ZAxis);
    RAxis = RZ*RAxis;

    vcg::Matrix44f RA;
    RA.SetRotateDeg(AngA, RAxis);

    return RA;
}
SRT _PCRegister(MeshModel *SMM, MeshModel *RMM)
{
    // Convering
    cpd::Matrix Fixed = _PC2Matrix(SMM);
    cpd::Matrix Moving = _PC2Matrix(RMM);
    // Do Registering
    cpd::Rigid RigidEng;
    RigidEng.scale(true);
    cpd::RigidResult Ret = RigidEng.run(Fixed, Moving);
    // Get Result
    SRT SRTOut;
    SRTOut.scale = Ret.scale;
    SRTOut.translation.X() = Ret.translation[0];
    SRTOut.translation.Y() = Ret.translation[1];
    SRTOut.translation.Z() = Ret.translation[2];
    SRTOut.rotation.SetIdentity();
    for (int i = 0; i<3; ++i)
        for (int j = 0; j<3; ++j)
            SRTOut.rotation.ElementAt(i,j) = Ret.rotation(i, j);

    return SRTOut;
}


bool PCEst(const char *SPath, const char *RPath, const char *OutPath)
{
    GlobParam::printLogo();
    GlobParam::printParams();
    GlobParam::setThreadNum(0);
    
    const string FileName = RPath;
    const string OutDir = OutPath;
    string BaseName = FileName.substr(0, FileName.rfind('.'));
    BaseName = BaseName.substr(BaseName.rfind('/')+1);
    const string FileRecordR2S      = OutDir + "/" + BaseName + "_R2S.txt";
    const string FileRecordS2R      = OutDir + "/" + BaseName + "_S2R.txt";
    const string FilePlyRegisted    = OutDir + "/" + BaseName + "_Rged.ply";
    const string FilePlyRegistedClr = OutDir + "/" + BaseName + "_RgedClr.ply";
    const string FilePlyRegistedJet = OutDir + "/" + BaseName + "_RgedJet.ply";
    
    string FilePlyBox = SPath;
    FilePlyBox = FilePlyBox.substr(0, FilePlyBox.rfind('.'));
    FilePlyBox = FilePlyBox + "_box.txt";

    // 0. Check Validity of Inputs
    if (SPath == 0 || RPath == 0 || OutPath == 0) {
        FLOG::clog << " > [Error] Empty Input Pathes (SPath, RPath or OutPath).\n";
        return false;
    }

    // I. Load Ply
    FLOG::flog(">> Loading Ply ...\n");
    FLOG::pushIndent();   
    GlobParam::GTimer.start();
    //------------------------
    MeshModel *PlyS_ori = PCLoad(SPath);
    if (0 == PlyS_ori) {
        FLOG::clog << " > [Error] Laod (S)ource Ply (i.e., Target Ply) " << SPath <<" Failed.\n";
        return false;
    }
    MeshModel *PlyR_ori = PCLoad(RPath);
    if (0 == PlyR_ori) {
        FLOG::clog << " > [Error] Laod (R)efrence Ply (i.e., Estimating Ply)" << RPath << " Failed.\n";
        return false;
    }
    //------------------------
    FLOG::popIndent();
    FLOG::flog(">> Loading Done in %.4f Seconds.\n", GlobParam::GTimer.elapsed() / 1000.0);

    
    // II. Down Sample
    FLOG::flog(">> Down Sampling to %d NPts ...\n", GlobParam::_GSampleNum);
    FLOG::pushIndent();
    GlobParam::GTimer.start();
    //------------------------   
    MeshModel *PlyS_Samp = PCSample(PlyS_ori, GlobParam::_GSampleNum);
    MeshModel *PlyR_Samp = PCSample(PlyR_ori, GlobParam::_GSampleNum);
    //------------------------
    FLOG::popIndent();
    FLOG::flog(">> Sample Done in %.4f Seconds.\n", GlobParam::GTimer.elapsed() / 1000.0);

    // III. Coherent Point Drift Point Set Registration 
    // ~~~~~~~~~~~~~~~~~~
    // Measuring Glacier Surface Velocities With LiDAR: A Comparison of Three-Dimensional Change Detection Methods.
    // Master's thesis, University of Houston, Geosensing Systems Engineering and Sciences.
    // by Gadomski, P.J. (December 2016). 
    // ~~~~~~~~~~~~~~~~~~
    // PDF See : https://www.researchgate.net/publication/315773214_Measuring_Glacier_Surface_Velocities_With_LiDAR_A_Comparison_of_Three-Dimensional_Change_Detection_Methods
    // Document See : http://www.gadom.ski/cpd
    // Soure Code See : https://github.com/gadomski/cpd
    FLOG::flog(">> Registering with %d Direction and %d Rotations ...\n", 
        GlobParam::_GTryDirectionN, GlobParam::_GTryRotationN);
    FLOG::pushIndent();
    GlobParam::GTimer.start();
    //------------------------
    SRT SRTOut = PCRegister(PlyS_Samp, PlyR_Samp, GlobParam::_GTryDirectionN, GlobParam::_GTryRotationN);
    delete PlyS_Samp;
    delete PlyR_Samp;
    //------------------------
    FLOG::popIndent();
    FLOG::flog(">> Registration Done in %.4f Seconds.\n", GlobParam::GTimer.elapsed() / 1000.0);


    // IV. Estmate PC Accuray and Completeness
    FLOG::flog(">> Estmate PC Accuray and Completeness ...\n");
    FLOG::pushIndent();
    GlobParam::GTimer.start();
    //------------------------  
    MeshModel *PlyR_Register = new MeshModel("", "");
    Append<CMeshO, CMeshO>::MeshCopy(PlyR_Register->cm, PlyR_ori->cm);
    PlyR_Register->Enable(PlyR_ori->mask());
    PCTransform(PlyR_Register, SRTOut);

    MeshModel *PlyS_Est = PlyS_ori;
    MeshModel *PlyR_Est = PlyR_Register;
    MeshModel *Ply_toDelete = 0;
    if (PlyS_ori->cm.vn < PlyR_ori->cm.vn) {
        FLOG::flog(" > Sampling R Ply Data ...\n");
        PlyR_Est = PCSample(PlyR_Register, PlyS_ori->cm.vn);
        Ply_toDelete = PlyR_Est;
    }
    else {
        FLOG::flog(" > Sampling S Ply Data ...\n");
        PlyS_Est = PCSample(PlyS_ori, PlyR_ori->cm.vn);
        Ply_toDelete = PlyS_Est;
    }
    FLOG::flog(" > Estimate R to S ...\n");
    PCNearestDist(PlyS_Est, PlyR_Est, 0, FileRecordR2S.c_str());
    FLOG::flog(" > Estimate S to R ...\n");
    PCNearestDist(PlyR_Est, PlyS_Est, 0, FileRecordS2R.c_str());
    delete Ply_toDelete;
    //------------------------
    FLOG::popIndent();
    FLOG::flog(">> Estmation Done in %.4f Seconds.\n", GlobParam::GTimer.elapsed() / 1000.0);

    // V. Save Rigister Ply
    FLOG::flog(">> Save Result Ply ...\n");
    FLOG::pushIndent();
    GlobParam::GTimer.start();
    //------------------------  
    FLOG::clog << " > Save Registered Ply ... \n";
    PCSave(PlyR_Register, FilePlyRegisted.c_str());
    FLOG::clog << " > Save Registered Ply with Jet Color and Chosen Color ... \n";
    
    ifstream fin(FilePlyBox.c_str());
    if (fin.is_open()) {
        double x, y, z;
        fin >> x >> y >> z;
        fin.close();
        PCSaveErrorCodee(PlyS_ori, PlyR_Register, z*GlobParam::_GMaxError, FilePlyRegistedJet.c_str(), FilePlyRegistedClr.c_str());
    }
    else
        FLOG::clog << " > [Error] Laod Box File Failed : " << FilePlyBox << "\n";
    //------------------------
    FLOG::popIndent();
    FLOG::flog(">> Save Done in %.4f Seconds.\n", GlobParam::GTimer.elapsed() / 1000.0);

    delete PlyR_Register;
    delete PlyS_ori;
    delete PlyR_ori;

    return true;
}

MeshModel* PCLoad(const char *Path)
{
    FLOG::clog << "[+ Loading " << Path << "\n";

    MeshModel *newMesh = 0;
    if (0 != Path) {

        QFileInfo fi(Path);
        QString extension = fi.suffix();
        if (!(extension.toUpper() == "PLY")) {
            FLOG::clog << " > [Error] Only Format *.PLY Is Supported Now !\n";
            return newMesh;
        }
        if (!fi.exists() || !fi.isReadable()) {
            FLOG::clog << " > [Error] Invalid File Path, Please Check: " << Path << "\n";
            return newMesh;
        }

        int mask = 0;
        newMesh = new MeshModel(Path, "");
        int ret = io::Importer<CMeshO>::Open(newMesh->cm, Path, mask);
        
        if (ret == 0 || (ret < 6 && extension.toUpper() == "OBJ"))
        {
            newMesh->Enable(mask);
            UpdateBounding<CMeshO>::Box(newMesh->cm);
        }
        else {         
            FLOG::clog << " > [Error] Load Error [" << ret << "]: " << io::Importer<CMeshO>::ErrorMsg(ret) << "\n";
            if (0 != newMesh) {
                delete newMesh;
                newMesh = 0;
            }
        }
    }

    FLOG::clog << "[- Load Done.\n";

    return newMesh;
}
bool PCSave(MeshModel *MM, const char *Path)
{
    FLOG::clog << "[+ Saving " << Path << "\n";

    if (0 == MM) {
        FLOG::clog << " > [Error] Nothing Needed to Save!\n";
        return false;
    }

    int mask = MM->mask();
    int ret = vcg::tri::io::Exporter<CMeshO>::Save(MM->cm, Path, mask);
    if (ret != 0) {
        FLOG::clog << " > [Error] Save Error [" << ret << "]: " << io::Exporter<CMeshO>::ErrorMsg(ret) << "\n";
        return false;
    }
    else {
        FLOG::clog << "[- Save Done.\n";
        return true;
    }    
}
void PCTransform(MeshModel *MM, const SRT &Tr)
{
    vcg::tri::UpdatePosition<CMeshO>::Scale(MM->cm, Tr.scale);
    vcg::tri::UpdatePosition<CMeshO>::Matrix(MM->cm, Tr.rotation);
    vcg::tri::UpdatePosition<CMeshO>::Translate(MM->cm, Tr.translation);
}

MeshModel* PCSample(MeshModel *SrtMM, const int SampleNum, const char *_OutPath)
{
    FLOG::clog << "[+ Sampling with " << SrtMM->cm.vn << " NPts" << "\n";

    if (0 == SrtMM) {
        FLOG::clog << " > [Error] To Sample Nothing !\n";
        return 0;
    }
    MeshModel *DstMM = new MeshModel("", "");
    float radius = SurfaceSampling<CMeshO, MeshSampler<CMeshO>>::ComputePoissonDiskRadius(SrtMM->cm, SampleNum);
    MeshSampler<CMeshO> mps(DstMM->cm);
    SurfaceSampling<CMeshO, MeshSampler<CMeshO>>::PoissonDiskParam pp;
    SurfaceSampling<CMeshO, MeshSampler<CMeshO>>::PoissonDiskPruningByNumber(mps, SrtMM->cm, SampleNum, radius, pp, 0.005);
    if (_OutPath != 0) {
        PCSave(DstMM, _OutPath);
    }
    FLOG::clog << "[- Sample Done by " << DstMM->cm.vn << " NPts"<< "\n";
    return DstMM;
}

SRT PCRegister(MeshModel *SMM, MeshModel *RMM, const int ND, const int NR)
{
    FLOG::clog << "[+ Coherent Point Drift Point Set Registering ... \n";

    SRT SRTNice = _PCRegister(SMM, RMM);
    MeshModel *RMM_Reg = new MeshModel("","");
    Append<CMeshO, CMeshO>::MeshCopy(RMM_Reg->cm, RMM->cm);
    PCTransform(RMM_Reg, SRTNice);
    double ErrorMin = PCNearestDist(SMM, RMM_Reg);
    delete RMM_Reg;
    FLOG::flog(" [- [%7.4f] : %s.\n", ErrorMin, "Origin (0,0)");

    
    const int BuffSize = GlobParam::_GTryDirectionN*GlobParam::_GTryRotationN;
    vector<double> ErrorBuff; 
    vector<SRT> SRTBuff;   
    ErrorBuff.resize(BuffSize);
    SRTBuff.resize(BuffSize);

    const int AngD = 360 / GlobParam::_GTryDirectionN;
    const int AngR = 180 / GlobParam::_GTryRotationN;
    
#ifdef _USE_OPENMP_
#pragma omp parallel for
#endif
    for (int i = 0; i < GlobParam::_GTryDirectionN; ++i) {
        MeshModel *_RMM = new MeshModel("", "");
        Append<CMeshO, CMeshO>::MeshCopy(_RMM->cm, RMM->cm);

        vcg::Matrix44f _R = _RotMatrix(AngD*i, AngR);
        vcg::Matrix44f _RSum;
        _RSum.SetIdentity();
        for (int j = 0; j < GlobParam::_GTryRotationN; ++j) {
            vcg::tri::UpdatePosition<CMeshO>::Matrix(_RMM->cm, _R);
            _RSum = _RSum*_R;

            SRT _SRT = _PCRegister(SMM, _RMM);
            MeshModel *_RMM_Reg = new MeshModel("", "");
            Append<CMeshO, CMeshO>::MeshCopy(_RMM_Reg->cm, _RMM->cm);
            PCTransform(_RMM_Reg, _SRT);
            double _Error = PCNearestDist(SMM, _RMM_Reg);
            delete _RMM_Reg;
            FLOG::flog(" [- [%7.4f] : Rotation (%d,%d).\n", _Error, AngD*i, AngR*j);

            const int index = i*GlobParam::_GTryRotationN + j;
            _SRT.rotation = _SRT.rotation*_RSum;
            ErrorBuff[index] = _Error;
            SRTBuff[index] = _SRT;   
            
        }
        delete _RMM;
    }

    for (int i = 0; i < BuffSize; ++i) {
        if (ErrorBuff[i] < ErrorMin) {
            ErrorMin = ErrorBuff[i];
            SRTNice = SRTBuff[i];
        }
    }
    FLOG::flog(" [- [%7.4f] : Final Transform£º\n", ErrorMin);
    FLOG::flog("    ----------------------------------\n");
    FLOG::flog("    [%4.3f]  [%4.3f]  [%4.3f] [%4.3f]\n",
        SRTNice.rotation.ElementAt(0, 0), SRTNice.rotation.ElementAt(0, 1), SRTNice.rotation.ElementAt(0, 2), SRTNice.translation.X());
    FLOG::flog("    [%4.3f]  [%4.3f]  [%4.3f] [%4.3f]\n",
        SRTNice.rotation.ElementAt(1, 0), SRTNice.rotation.ElementAt(1, 1), SRTNice.rotation.ElementAt(1, 2), SRTNice.translation.Y());
    FLOG::flog("    [%4.3f]  [%4.3f]  [%4.3f] [%4.3f]\n",
        SRTNice.rotation.ElementAt(2, 0), SRTNice.rotation.ElementAt(2, 1), SRTNice.rotation.ElementAt(2, 2), SRTNice.translation.Z());
    FLOG::flog("    [%4.3f]  [%4.3f]  [%4.3f] [%4.3f]\n", 0.0, 0.0, 0.0, SRTNice.scale);

    FLOG::clog << "[- Registration Done. \n";

    return SRTNice;
}

double PCNearestDist(
    MeshModel *SMM, MeshModel *RMM,
    double *sigma, const char *OutFile)
{
    vector<float> errors;
    errors.reserve(SMM->cm.vn);
    double miu = 0.0;
    double var = 0.0;

    vcg::VertexConstDataWrapper<CMeshO> ww(SMM->cm);
    vcg::KdTree<float> KDTree(ww);

    CMeshO &mesh = RMM->cm;
    for (CMeshO::VertexIterator vi = mesh.vert.begin(); vi != mesh.vert.end(); ++vi) {
        vcg::Point3f p = vi->cP();
        unsigned int idx;
        float dist;
        KDTree.doQueryClosest(p, idx, dist);
        errors.push_back(dist);
        miu += dist;
        var += dist*dist;
    }
    miu = miu / RMM->cm.vn;
    var = var / RMM->cm.vn - miu*miu;
    if (0 != sigma)
        *sigma = var;
    if (0 != OutFile) {
        FLOG::clog << "[+ Save Recorder in " << OutFile << " ... \n";
        ofstream fout(OutFile);
        if (fout.is_open()) {
            for (int i = 0; i < errors.size(); ++i)
                fout << errors[i] << endl;
            fout.close();
            FLOG::clog << "[- Save Done.\n";
        }
        else
            FLOG::clog << "[- [Error] Open Failed !\n";
    }
    return miu;
}
void PCSaveErrorCodee(
    MeshModel *SMM, MeshModel *RMM, const double maxe, 
    const char *OutFileJet, const char *OutFileClr)
{
    FLOG::clog << "[+ Save with Error Color Coded [" << maxe << "]... \n";

    vcg::VertexConstDataWrapper<CMeshO> ww(SMM->cm);
    vcg::KdTree<float> KDTree(ww);

    CMeshO &mesh = RMM->cm;
    for (CMeshO::VertexIterator vi = mesh.vert.begin(); vi != mesh.vert.end(); ++vi) {
        vcg::Point3f p = vi->cP();
        unsigned int idx;
        float dist;
        KDTree.doQueryClosest(p, idx, dist);
        float fcode = abs(dist / maxe);
        if (fcode > 1)
            fcode = 1;
        vi->C().X() = JetColormap::Red(fcode) * 255;
        vi->C().Y() = JetColormap::Green(fcode) * 255;
        vi->C().Z() = JetColormap::Blue(fcode) * 255;
    }
    FLOG::clog << " > Save Error Color Coded Ply " << OutFileJet << "... \n";
    PCSave(RMM, OutFileJet);

    if (0 != OutFileClr) {
        vcg::Color4b c(0,0,255,0);
        for (CMeshO::VertexIterator vi = mesh.vert.begin(); vi != mesh.vert.end(); ++vi) {         
            vi->C() = c;       
        }
        FLOG::clog << " > Save Colored Ply " << OutFileClr << "... \n";
        PCSave(RMM, OutFileClr);
    }

    FLOG::clog << "[- Done.\n";
}