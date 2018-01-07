#include <io.h>
#include <windows.h>

#include "PCEst.h"
#include "flog.h"

int main(int argc, char** argv) {
    const std::string PlyDirFixed  = "E:/Data/PCEst";
    const std::string PlyDirMoving = "../Data";
    const std::string DirResult    = "../Data/Result";

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