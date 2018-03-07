#include <io.h>
#include <windows.h>

#include "PCEst.h"
#include "flog.h"

#if 0
int main(int argc, char** argv) {
    const std::string PlyDirFixed  = "E:/Data/PCEst";
    const std::string PlyDirMoving = "E:/Data/SpaceModels";
    const std::string DirResult    = "E:/Results/PCE/8Tars";

    const std::string LogFile = DirResult + "/flog.txt";


    CreateDirectoryA(DirResult.c_str(), NULL);
    FLOG::setLogFile(LogFile.c_str());
    

    const std::string szPath = PlyDirMoving + "/*.ply";
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    if ((hFile = _findfirst(szPath.c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib & _A_ARCH))
            {
                const std::string FileName  = fileinfo.name;
                const std::string PlyFixed  = PlyDirFixed  + "/" + FileName;
                const std::string PlyMoving = PlyDirMoving + "/" + FileName;

                if ((_access(PlyFixed.c_str(), 0)) != -1) {
                    FLOG::clog << "> " << PlyMoving << "\n";
                    FLOG::clog << "> " << PlyFixed << "\n";
                    PCEst(PlyFixed.c_str(), PlyMoving.c_str(), DirResult.c_str());
                }
                else
                    FLOG::clog << "[Warning] : No Reference File " << PlyFixed << "\n";
               
                FLOG::clog << "\n\n----------------------\n\n";
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
    system("pause");
    return 0;
}
#endif

int main(int argc, char** argv) {
    const std::string PlyDirFixed = "E:/Data/PCEst";
    const std::string DirResult = "E:/Results/PCE/Comp";
    const std::string vecSN[7] = { { "0" },{ "3" },{ "5" },{ "10" },{ "20" },{ "40" },{ "60" } };
    const std::string vecObj[2] = { { "Dsp" },{ "GPS" } };
    const std::string _vecObj[2] = { { "dsp" },{ "gps" }};
    const std::string pathColmap = "E:/Results/ColMap";
    const std::string pathVSFM = "E:/Results/VSFM";


    const std::string LogFile = DirResult + "/flog.txt";

    
    CreateDirectoryA(DirResult.c_str(), NULL);
    FLOG::setLogFile(LogFile.c_str());

    for (int i = 0; i < 7; ++i) {
        int j = 1;
       
        const std::string _path = pathVSFM + "/" + vecSN[i] + "/" + vecObj[j];
        const std::string PlyFixed = PlyDirFixed + "/" + _vecObj[j] + ".ply";
        const std::string PlyMoving = _path + "/" + vecObj[j] + ".ply";
        const std::string DirResult_One = DirResult + "/VSFM_"+ vecObj[j] + "_" + vecSN[i];
        CreateDirectoryA(DirResult_One.c_str(), NULL);

        SetConsoleTitleA(PlyMoving.c_str());

        if ((_access(PlyFixed.c_str(), 0)) != -1) {
            FLOG::clog << "> " << PlyMoving << "\n";
            FLOG::clog << "> " << PlyFixed << "\n";
            PCEst(PlyFixed.c_str(), PlyMoving.c_str(), DirResult_One.c_str());
        }
        else
            FLOG::clog << "[Warning] : No Reference File " << PlyFixed << "\n";

        FLOG::clog << "\n\n----------------------\n\n";
    }
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 2; ++j) {

            const std::string _path = pathColmap + "/" + vecSN[i] + "/" + vecObj[j];
            const std::string PlyFixed = PlyDirFixed + "/" + _vecObj[j] + ".ply";
            const std::string PlyMoving = _path + "/" + vecObj[j] + ".ply";
            const std::string DirResult_One = DirResult + "/COLMAP_" + vecObj[j] + "_" + vecSN[i];
            CreateDirectoryA(DirResult_One.c_str(), NULL);

            SetConsoleTitleA(PlyMoving.c_str());

            if ((_access(PlyFixed.c_str(), 0)) != -1) {
                FLOG::clog << "> " << PlyMoving << "\n";
                FLOG::clog << "> " << PlyFixed << "\n";
                PCEst(PlyFixed.c_str(), PlyMoving.c_str(), DirResult_One.c_str());
            }
            else
                FLOG::clog << "[Warning] : No Reference File " << PlyFixed << "\n";

            FLOG::clog << "\n\n----------------------\n\n";
        }
    }

    system("pause");
    return 0;
}