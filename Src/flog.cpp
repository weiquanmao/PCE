#include "flog.h"
#include <stdarg.h>
#include <stdio.h>
#if defined(USE_FLOG_NAMESPACE)
namespace FLOG {
#endif

    FLogCout clog;
    FILE *_log_file_ = 0;
    char _log_file_path_[1024];
    bool _b_log_file_opened_ = false;
    char _log_mode_[4] = "w";
    const char _log_file_path_default_[] = "../flog.txt";


    //======================
    bool _auto_indent_ = true;
    int _tabn_ = 2;
    int _cur_stage_ = 0;
    int _max_stage_ = 100;
    void autuIndent(const bool bOn) { _auto_indent_ = bOn; }
    bool autuIndent() { return _auto_indent_; }
    int setIndent(const int tabn)
    {
        int old = _tabn_;
        if (tabn >= 0) {
            _tabn_ = tabn;
        }
        return old;
    }
    int setMaxStage(const int maxn)
    {
        int old = _max_stage_;
        if (maxn >= 0) {
            _max_stage_ = maxn;
        }
        return old;
    }
    int maxStage() { return _max_stage_ ;}
    int indentStage() { return _cur_stage_; }
    int pushIndent() {
        if (_cur_stage_<_max_stage_)
            _cur_stage_++;
        return _cur_stage_;
    }
    int popIndent()
    {
        if (_cur_stage_ > 0)
            _cur_stage_--;
        return _cur_stage_;
    }
    void addIndent()
    {
        if (_auto_indent_)
        {
            for (int i = 0; i < _tabn_*_cur_stage_;++i) {
                printf(" ");
                fprintf(_log_file_, " ");
            }
        }
    }
    //======================

   
    
    bool _closeLog(bool cleanPath)
    {
        bool bRet = true;
        if (_b_log_file_opened_) {
            if (fclose(_log_file_) == 0) {
                _log_file_ = 0;
                clog._m_log_file_ = std::ofstream(_log_file_);
                _b_log_file_opened_ = false;
                if (cleanPath)
                     sprintf(_log_file_path_, "");
            }
            else {
                printf("[FLOG ERROR]: Failed to close log file %s.", _log_file_path_);
                bRet = false;
            }
        }
        return bRet;
    }
    bool _openLog(const char *logFile)
    {
        bool bRet = true;
        if (logFile == 0)
            bRet = _closeLog(true);
        else {
            if (_b_log_file_opened_ && !_closeLog(false))
                return false;

            FILE *log_file_temp = fopen(logFile, _log_mode_);
            if (log_file_temp != 0) {               
                _log_file_ = log_file_temp;
                clog._m_log_file_ = std::ofstream(_log_file_);
                sprintf(_log_file_path_, logFile);
                _b_log_file_opened_ = true;
                bRet = true;
            }
            else {
                printf("[FLOG ERROR]: Failed to open log file %s.", _log_file_path_);
                bRet = false;
            }
        }

        return bRet;
    }
    
    bool isLogFileOpened() { return _b_log_file_opened_; }
    bool setDefaultLogFile() { return _openLog(_log_file_path_default_); }
    bool setLogFile(const char *logFile) { return _openLog(logFile); }
    const char* getLogFile() { return _log_file_path_; }

    void setAppendMode() { sprintf(_log_mode_, "a"); }
    void setWriteMode() { sprintf(_log_mode_, "w"); }

    int flog(const char *format, ...)
    {        
        if (_auto_indent_)
            addIndent();
        va_list ap1;
        va_start(ap1, format);
        int n1 = vprintf(format, ap1);
        va_end(ap1);

        if (!_b_log_file_opened_)
            _openLog(_log_file_path_default_);

        if (_b_log_file_opened_) {
            va_list ap2;
            va_start(ap2, format);
            vfprintf(_log_file_, format, ap2);
            va_end(ap2);
            fflush(_log_file_);
        }
        return n1;
    }

#if defined(USE_FLOG_NAMESPACE)
}
#endif