#ifndef _LOGGER
#define _LOGGER

#include <map>
#include <string>
#include <fstream>

#if defined(_WIN32) || defined(WIN32)
#define __FUNCTIONNAME__ __FUNCTION__
#else
#define __FUNCTIONNAME__ __PRETTY_FUNCTION__
#endif

typedef enum LogLevel
{
    LOG_INFO=0,
    LOG_ERROR=1,
    LOG_WARNING=2,
    LOG_CRITICAL=3,
    LOG_PANIC=4
}LogLevel;

class Logger
{
public:
	Logger();
	~Logger();

    void    StartLogging();
    void    StopLogging();
    void    Write(std::string logEntry, LogLevel llevel, const char* func, const char* file, int line);
    void    SetLogFileSize(int flsz);
    void    SetLogDirectory(const std::string &dirpath);
    void    SetModuleName(const std::string& mname);
    static Logger*  GetInstance();
private:
	std::string logFilename;
	std::string  logDirectory;
	std::string  moduleName;
    size_t     logFileSize;
    std::ofstream   logFile;
    std::map<LogLevel, std::string> logLevelMap;
};

#define writeLog(str,level) Logger::GetInstance()->Write(str,level,__FUNCTIONNAME__,__FILE__,__LINE__);
#define writeLogNormal(str) Logger::GetInstance()->Write(str,LOG_INFO,__FUNCTIONNAME__,__FILE__,__LINE__);

#endif

