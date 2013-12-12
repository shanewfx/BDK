#ifndef _LOG_H
#define _LOG_H

#include <string>

#define DEBUG_LOG
#define ENABLE_DEBUGVIEWER
#define ENABLE_CONSOLE

#define LOG_FILE_DIR  "C:\\DebugLog"
#define LOG_FILE_NAME "BDK.txt"
#define LOG_FILE_PATH "C:\\DebugLog\\BDK.txt"

typedef enum tagLogLevel {
    _LOG_TRACE,
    _LOG_INFO,
    _LOG_WARNING,
    _LOG_ERROR,
    _LOG_FATAL,
    _LOG_NONE,
} LogLevel;

#ifdef DEBUG_LOG
	#define LogData              Logger::Instance().Log
    #define LogTrace(fmt, ...)   Logger::Instance().Log(_LOG_TRACE, fmt, __VA_ARGS__)
    #define LogInfo(fmt, ...)    Logger::Instance().Log(_LOG_INFO, fmt, __VA_ARGS__)
    #define LogWarning(fmt, ...) Logger::Instance().Log(_LOG_WARNING, fmt, __VA_ARGS__)
    #define LogError(fmt, ...)   Logger::Instance().Log(_LOG_ERROR, fmt, __VA_ARGS__)
    #define LogFatal(fmt, ...)   Logger::Instance().Log(_LOG_FATAL, fmt, __VA_ARGS__)
	#define SetFileDirectory     Logger::SetLogFileDirectory
    #define SetFileName          Logger::SetLogFileName
	#define SetLevel             Logger::SetLogLevel
    #define DisableLog           Logger::DisableDebugLog
    #define EnableLogToFile      Logger::EnableLogFile
    #define EnableLogToDebugView Logger::EnableDebugView
    #define EnableLogToConsole   Logger::EnableConsole
#else
	#define LogData
    #define LogTrace(fmt, ...)
    #define LogInfo(fmt, ...)
    #define LogWarning(fmt, ...)
    #define LogError(fmt, ...)
    #define LogFatal(fmt, ...)
	#define SetFileDirectory
    #define SetFileName
	#define SetLevel
    #define DisableLog
    #define EnableLogToFile
    #define EnableLogToDebugView
    #define EnableLogToConsole
#endif


class Logger
{
public:
	static Logger& Instance();
	
    static bool SetLogFileDirectory(const std::string& strFileDir, bool bDeleteOldDir = false);
    static void SetLogFileName(const std::string& strFileName);
	static void SetLogLevel(const LogLevel Level);

    static void DisableDebugLog();
    static void EnableLogFile(bool bEnableFileLog);
    static void EnableDebugView(bool bEnableDebugView);
    static void EnableConsole(bool bEnableConsole);
	
	void Log(const LogLevel Level, const char* Format, ...);
	
private:
	Logger();
	Logger(Logger const&);
	Logger& operator=(Logger const&);
	~Logger();

    static bool Initialise();
    static void Dispose();
    static void MakeLogFilePath();
    static bool CreateLogDirectory(const std::string& strFileDir);
    static bool DeleteLogDirectory(const std::string& strFileDir);
    static bool DeleteDirectoryFile(char* lpszPath);

    static bool         m_bEnableFileLog;
    static bool         m_bEnableDebugView;
    static bool         m_bEnableConsole;

	static FILE*        m_hLogFile;
	static LogLevel	    m_LogLevel;

    static bool         m_bCreateDir;

    static std::string	m_strFileDir;
    static std::string	m_strFileName;
    static std::string	m_strFilePath;
};

#endif//_LOG_H