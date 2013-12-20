#include "log.h"
#include <stdarg.h>
#include <windows.h>

namespace BDK {

FILE*       Logger::m_hLogFile    = NULL;
LogLevel    Logger::m_LogLevel    = _LOG_TRACE;

std::string Logger::m_strFileDir  = LOG_FILE_DIR;
std::string Logger::m_strFileName = LOG_FILE_NAME;
std::string Logger::m_strFilePath = "";
bool        Logger::m_bCreateDir  = true;

bool        Logger::m_bEnableFileLog   = true;
bool        Logger::m_bEnableDebugView = true;
bool        Logger::m_bEnableConsole   = true;

int         Logger::m_logFileCount     = 0;


char* LogLevelStr[] = {
    "TRACE",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL",
    "NONE"
};

Logger& Logger::Instance() 
{
    static Logger LoggerInstance;
    return LoggerInstance;
}

void Logger::SetLogFileName(const std::string& strFileName)
{
    m_strFileName = strFileName;
    MakeLogFilePath();
}

bool Logger::SetLogFileDirectory(const std::string& strFileDir, bool bDeleteOldDir)
{
    std::string strDirTemp = strFileDir;
    if (strDirTemp.substr(strDirTemp.length() - 1, 1) == "\\") {
        strDirTemp = strDirTemp.substr(0, strDirTemp.length() - 1);
    }
 

    if (!CreateLogDirectory(strDirTemp)) {
        return false;
    }

    if (bDeleteOldDir) {
        Dispose();
        DeleteLogDirectory(m_strFileDir);
    }
    
    m_strFileDir = strDirTemp;
    MakeLogFilePath();
    return true;
}

void Logger::SetLogLevel(const LogLevel Level)
{
	m_LogLevel = Level;
}

Logger::Logger()
{
	Initialise();
}

Logger::~Logger()
{
	Dispose();
}

bool Logger::Initialise()
{
	if (m_hLogFile != NULL) {
        return true;	
	}

    if (m_bCreateDir) {
        if (!SetLogFileDirectory(m_strFileDir)) {
            return false;
        }
        m_bCreateDir = false;
    }

    m_hLogFile = fopen(m_strFilePath.c_str(), "w+");
    if (m_hLogFile == NULL) {
        return false;
    }
    return true;
}

void Logger::Dispose()
{
	if (NULL != m_hLogFile) {
		fflush(m_hLogFile);
		fclose(m_hLogFile);
		m_hLogFile = NULL;
	}
}

void Logger::Log(const LogLevel Level, const char* Format, ...)
{
    if (!m_bEnableFileLog && !m_bEnableDebugView && !m_bEnableConsole) {
        return;
    }

    if (m_LogLevel > Level) {
        return;
    }

    if (NULL == m_hLogFile && !Initialise()) {
        return;
    }

	char szBuffer[1024];

	va_list args;
    va_start(args, Format);
	vsprintf_s(szBuffer, Format, args);
	va_end(args);

#ifdef ENABLE_DEBUGVIEWER
    if (m_bEnableDebugView) {
        OutputDebugStringA(szBuffer);
    }
#endif

#ifdef ENABLE_CONSOLE
    if (m_bEnableConsole) {
        printf(szBuffer);
    }
#endif


    if (!m_bEnableFileLog) {
        return;
    }

	SYSTEMTIME st;		
	GetLocalTime(&st);
	if (0 > fprintf(m_hLogFile, "[%04u-%02u-%02u %02u:%02u:%02u:%03u] [%s] %s", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, 
		LogLevelStr[Level], szBuffer)) {
		Dispose();
	}
	else {
		fflush(m_hLogFile);

        if (ftell(m_hLogFile) >= MAXLOGSIZE) {
            m_logFileCount++;
            MakeLogFilePath();
        }
	}
}

void Logger::MakeLogFilePath()
{
    Dispose();
    m_strFilePath  = m_strFileDir + "\\";
    m_strFilePath += m_strFileName;

    SYSTEMTIME st;		
    GetLocalTime(&st);
    char buf[64] = {0};
    _snprintf_s(buf, sizeof(buf), "_%4d%02d%02d%02d%02d%02d_%d" LOG_FILE_EXT,
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, m_logFileCount);
    m_strFilePath += buf;
}

bool Logger::CreateLogDirectory(const std::string& strFileDir)
{
    char* pChar = NULL;
    char strPathTemp[MAX_PATH] = { 0 };
    memcpy(strPathTemp, strFileDir.c_str(), strFileDir.size());

    pChar = strchr(strPathTemp, ':');
    if (pChar == NULL) {
        return false;
    }
    pChar = strchr(strPathTemp, '\\');
    if (pChar == NULL) {
        return false;
    }

    //C:\Dir1\Dir2\Dir3
    do {
        pChar = strchr(++pChar, '\\');
        if (pChar == NULL) {
            CreateDirectoryA(strPathTemp, NULL);
        }
        else {
            *pChar = '\0';
            CreateDirectoryA(strPathTemp, NULL);
            memcpy(strPathTemp, strFileDir.c_str(), strFileDir.size());
        }
    } while (pChar != NULL);

    return true;
}

bool Logger::DeleteLogDirectory(const std::string& strFileDir)
{
    char* pChar = NULL;
    char strPathTemp[MAX_PATH] = { 0 };
    memcpy(strPathTemp, strFileDir.c_str(), strFileDir.size());

    pChar = strchr(strPathTemp, ':');
    if (pChar == NULL) {
        return false;
    }
    pChar = strchr(strPathTemp, '\\');
    if (pChar == NULL) {
        return false;
    }

    pChar = strchr(++pChar, '\\');
    if (pChar != NULL) {
        *pChar = '\0';
    }
    
    return DeleteDirectoryFile(strPathTemp);;
}

bool Logger::DeleteDirectoryFile(char* lpszPath)
{
    SHFILEOPSTRUCTA FileOp = { 0 };
    FileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
    FileOp.pFrom  = lpszPath;
    FileOp.pTo    = NULL;
    FileOp.wFunc  = FO_DELETE;
    return SHFileOperationA(&FileOp) == 0;
}

void Logger::DisableDebugLog()
{
    m_bEnableFileLog   = false;
    m_bEnableDebugView = false;
    m_bEnableConsole   = false;
}

void Logger::EnableLogFile(bool bEnableFileLog)
{
    m_bEnableFileLog   = bEnableFileLog;
}

void Logger::EnableDebugView(bool bEnableDebugView)
{
    m_bEnableDebugView = bEnableDebugView;
}

void Logger::EnableConsole(bool bEnableConsole)
{
    m_bEnableConsole = bEnableConsole;
}

}//namespace BDK