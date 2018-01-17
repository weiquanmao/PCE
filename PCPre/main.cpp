#include <stdio.h>
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <io.h>

#include "MeshDoc.h"
#include "PCA.h"

const std::string InputDir = "E:/Data/PCEst";

bool PCADimensions(CMeshO &mesh, const char *savePath);
void PCReColor(
    CMeshO &mesh, 
    const unsigned int R, 
    const unsigned int G, 
    const unsigned int B);

int main()
{
    const std::string GreenDir = InputDir + "/Green";
    const std::string RedDir   = InputDir + "/Red";
    const std::string BlueDir  = InputDir + "/Blue";
    CreateDirectoryA(GreenDir.c_str(), NULL);
    CreateDirectoryA(RedDir.c_str(), NULL);
    CreateDirectoryA(BlueDir.c_str(), NULL);

    const std::string szPath = InputDir + "/*.ply";
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    if ((hFile = _findfirst(szPath.c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib & _A_ARCH))
            {
                std::cout << "Processing " << fileinfo.name << std::endl;
                const std::string FileName = fileinfo.name;
                const std::string BaseName = FileName.substr(0, FileName.rfind('.'));

                const std::string FilePath  = InputDir + "/" + FileName;
                const std::string BoxFile   = InputDir + "/" + BaseName + "_box.txt";
                const std::string RedFile   = RedDir   + "/" + FileName;
                const std::string BlueFile  = BlueDir  + "/" + FileName;
                const std::string GreenFile = GreenDir + "/" + FileName;
                
                MeshDocument MeshDoc;
                if (!MeshDoc.loadMesh(FilePath.c_str())) {
                    std::cout << "[Error] : Failed to Open Ply File " << FilePath << std::endl;
                    continue;
                }
                CMeshO &mesh = MeshDoc.mesh->cm;

                PCADimensions(mesh, BoxFile.c_str());
                std::cout << BoxFile << std::endl;

                PCReColor(mesh, 255, 0, 0);
                MeshDoc.saveMesh(RedFile.c_str());
                std::cout << RedFile << std::endl;

                PCReColor(mesh, 0, 255, 0);
                MeshDoc.saveMesh(GreenFile.c_str());
                std::cout << GreenFile << std::endl;

                PCReColor(mesh, 0, 0, 255);
                MeshDoc.saveMesh(BlueFile.c_str());
                std::cout << BlueFile << std::endl;

                std::cout << "----------------------" << std::endl;           
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
    system("pause");
    return 0;
}

//===============================================

double MeshSizeAlongN(CMeshO &mesh, const vcg::Point3f n)
{
    double minDis = DBL_MAX;
    double maxDis = -DBL_MAX;
    vcg::Point3f normalN = n;
    normalN.Normalize();
    for (CMeshO::VertexIterator vi = mesh.vert.begin(); vi != mesh.vert.end(); ++vi)
    {
        if ((*vi).IsD())
            continue;
        double proLoc = (*vi).cP()*normalN;
        if (proLoc > maxDis) maxDis = proLoc;
        if (proLoc < minDis) minDis = proLoc;
    }
    return abs(maxDis - minDis);
}
bool PCADimensions(CMeshO &mesh, const char *savePath)
{
    std::vector<vcg::Point3f> PDirections;
    vcg::Point3f PSize;
    //----[[
    // 1. Call PCA
    const int N = mesh.vn;
    int row = 3, col = N;
    double *data = new double[3 * N];
    int Count = 0;
    for (CMeshO::VertexIterator vi = mesh.vert.begin(); vi != mesh.vert.end(); ++vi)
    {
        if ((*vi).IsD())
            continue;
        data[Count] = (*vi).cP().X();
        data[N + Count] = (*vi).cP().Y();
        data[2 * N + Count] = (*vi).cP().Z();
        ++Count;
    }
    assert(Count == N);
    double PC[9], V[3];//V[3] useless
    int ret = PCA(data, row, col, PC, V);
    delete[] data;
    if (ret == -1)
        return false;

    // 2. Normalize And Assign
    vcg::Point3f NX = vcg::Point3f(PC[0], PC[3], PC[6]);
    vcg::Point3f NY = vcg::Point3f(PC[1], PC[4], PC[7]);
    vcg::Point3f NZ = vcg::Point3f(PC[2], PC[5], PC[8]);
    NX.Normalize();
    NY.Normalize();
    NZ.Normalize();

    PSize.X() = MeshSizeAlongN(mesh, NX);
    PSize.Y() = MeshSizeAlongN(mesh, NY);
    PSize.Z() = MeshSizeAlongN(mesh, NZ);

    //

    std::ofstream fout(savePath);

    fout << PSize.X() << " " << PSize.Y() << " " << PSize.Z() << std::endl;
    fout << NX.X() << " " << NX.Y() << " " << NX.Z() << std::endl;
    fout << NY.X() << " " << NY.Y() << " " << NY.Z() << std::endl;
    fout << NZ.X() << " " << NZ.Y() << " " << NZ.Z() << std::endl;

    fout.close();

    return true;
}

void PCReColor(
    CMeshO &mesh,
    const unsigned int R,
    const unsigned int G,
    const unsigned int B)
{
    for (CMeshO::VertexIterator vi = mesh.vert.begin(); vi != mesh.vert.end(); ++vi)
    {
        if ((*vi).IsD())
            continue;
        vi->C().X() = R;
        vi->C().Y() = G;
        vi->C().Z() = B;
    }
 
}