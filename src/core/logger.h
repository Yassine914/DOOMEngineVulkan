#pragma once
// -------------- MINI-LOGGER-CPP ------------
// made by yassin shehab (Y)

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>

#include <chrono>
using std::chrono::system_clock;

#define LOCK_MUTEX(x) std::lock_guard<std::mutex> lock(x)
#define INITLOG()                                                                                                      \
    Logger *Logger::instance = nullptr;                                                                                \
    Logger &Log = *Logger::GetInstance();

// macros for initializing logger with
// a different outstream.
// define at the end instead of LOGINIT_COUT()
#define LOGINIT_COUT()    Logger Log(std::cout)
#define LOGINIT_CERR()    Logger Log(std::cerr)
#define LOGINIT_CLOG()    Logger Log(std::clog)
#define LOGINIT_CUSTOM(x) Logger Log(x)

// NOTE: remove this line to remove colors
#define TEXT_COLORS

#ifdef TEXT_COLORS
    #define TEXT_RED    "\x1b[31m"
    #define TEXT_GREEN  "\x1b[32m"
    #define TEXT_YELLOW "\x1b[33m"
    #define TEXT_BLUE   "\x1b[34m"
    #define TEXT_PURPLE "\x1b[35m"
    #define TEXT_CYAN   "\x1b[36m"
    #define TEXT_WHITE  "\x1b[0m"
#else
    #define TEXT_RED    "\x1b[0m"
    #define TEXT_GREEN  "\x1b[0m"
    #define TEXT_YELLOW "\x1b[0m"
    #define TEXT_BLUE   "\x1b[0m"
    #define TEXT_PURPLE "\x1b[0m"
    #define TEXT_CYAN   "\x1b[0m"
    #define TEXT_WHITE  "\x1b[0m"
#endif

#define LOG_FATAL 1
#define LOG_ERROR 2
#define LOG_WARN  3
#define LOG_DEBUG 4
#define LOG_TRACE 5
#define LOG_INFO  6

#define LOG_CONSOLE 0
#define LOG_FILE    1

// default file for file output if one isn't specified
#define LOG_DEFAULT_FILE "logger.txt"
#define FILE_INFO        __FILE__, __LINE__

// macro for logging with file info.
#define LOG(x, y) Log(x, FILE_INFO) << y

enum class LogLevel
{
    NONE = 0,
    FATAL = 1,
    ERROR = 2,
    WARN = 3,
    DEBUG = 4,
    TRACE = 5,
    INFO = 6,
};

enum class OutputType
{
    NONE = 0,
    CONSOLE = 1,
    FILE = 2,
    FILEONLY = 3
};

// --------------------- LOGGER CLASS --------------------
class Logger
{
    public:
    static LogLevel logLevel;
    static OutputType outType;

    private:
    LogLevel currentLevel;

    std::ofstream fileStream;
    std::string filePath;

    std::ostream &outStream;

    public:
    static Logger *instance;
    std::mutex mut;

    // clang-format off
    Logger(std::ostream &os)
        :currentLevel{logLevel},
        outStream{os}
    {
    }

    public:
    Logger(const Logger &log) = delete;

    static Logger *GetInstance()
    {
        if(instance == nullptr)
            instance = new Logger(std::cout);

        return instance; 
    }

    // setters
    void EnableFileOutput(std::string filepath = nullptr)
    {
        if(filepath.size() == 0)
            filePath = LOG_DEFAULT_FILE;
        else
            filePath = filepath;

        Logger::outType = OutputType::FILE;

        if(fileStream.is_open())
            fileStream.close();

        fileStream.open(filePath, std::ios::out);
    }

    void EnableOnlyFileOutput(std::string filepath = nullptr)
    {
        if(filepath.size() == 0)
            filePath = LOG_DEFAULT_FILE;
        else
            filePath = filepath;

        Logger::outType = OutputType::FILEONLY;

        if(fileStream.is_open())
            fileStream.close();

        fileStream.open(filePath, std::ios::out);
    }

    void DisableFileOutput()
    {
        Logger::outType = OutputType::CONSOLE;

        if(fileStream.is_open())
            fileStream.close();
    }

    template<typename T>
    static void SetLogLevel(T ll)
    {
        Logger::logLevel = (LogLevel) ll;
    }

    // operator() takes log level and (optional) file data
    template<typename T>
    Logger &operator()(T ll)
    {
        currentLevel = (LogLevel) ll;
        LOCK_MUTEX(mut);
        if((LogLevel)ll > Logger::logLevel)
            return *this;
        
        if(Logger::outType != OutputType::FILEONLY)
        {
            outStream << "[";
            outStream << OutputColoredHeader((LogLevel) ll);

            outStream << " ";
            outStream << CurrentTime();
            
            outStream << ResetColor();
            outStream << "]\t";
        }

        if(fileStream.is_open())
        {
            fileStream << "[";
            fileStream << OutputHeader((LogLevel) ll);
            
            outStream << " ";
            outStream << CurrentTime();
            
            fileStream << "]\t";
        }
        
        return *this;
    }

    template<typename T, typename F, typename N>
    Logger &operator()(T ll, F fileName, N lineNumber)
    {
        currentLevel = (LogLevel) ll;

        LOCK_MUTEX(mut);
        if((LogLevel) ll > Logger::logLevel)
            return *this;

        if(Logger::outType != OutputType::FILEONLY)
        {
            // [LEVEL TIME FILE]\t
            outStream << "[";
            outStream << OutputColoredHeader((LogLevel) ll);
            outStream << " ";
            
            outStream << CurrentTime();
            outStream << " ";

            outStream << OutputFileInfo(fileName, lineNumber);

            outStream << ResetColor();
            outStream << "]\t";
        }
       
        if(fileStream.is_open())
        {
            fileStream << "[" << OutputHeader((LogLevel) ll);
            
            fileStream << " ";

            outStream << CurrentTime();
            outStream << " ";

            fileStream << OutputFileInfo(fileName, lineNumber);
            fileStream << "]\t";
        }

        return *this;
    }

    // outputs actual message with << operator
    template<typename T>
    Logger &operator<<(const T &txt)
    {
        LOCK_MUTEX(mut);
        if(currentLevel > Logger::logLevel)
            return *this;
        
        outStream << txt;

        if(fileStream.is_open())
            fileStream << txt;
        
        return *this;
    }

    private:
    std::string OutputColoredHeader(LogLevel ll)
    {
        std::string output;
        switch(ll)
        {
        case LogLevel::NONE:
            break;
        case LogLevel::FATAL:
            output = "";
            output.append(TEXT_RED);
            output.append("FATAL");
            return output;
            break;
        case LogLevel::ERROR:
            output = TEXT_RED;
            output.append("ERROR");
            return output;
            break;
        case LogLevel::WARN:
            output = TEXT_YELLOW;
            output.append("WARN");
            return output;
            break;
        case LogLevel::DEBUG:
            output = TEXT_BLUE;
            output.append("DEBUG");
            return output;
            break;
        case LogLevel::TRACE:
            output = TEXT_CYAN;
            output.append("TRACE");
            return output;
            break;
        case LogLevel::INFO:
            output = TEXT_PURPLE;
            output.append("INFO");
            return output;
            break;
        }
        return output;
    }

    std::string OutputHeader(LogLevel ll) 
    {
        std::string output;

        if(!fileStream.is_open())
            return "";
        
        switch(ll)
        {
        case LogLevel::NONE:
            break;
        case LogLevel::FATAL:
            return "FATAL";
            break;
        case LogLevel::ERROR:
            return "ERROR";
            break;
        case LogLevel::WARN:
            return "WARN";
            break;
        case LogLevel::DEBUG:
            return "DEBUG";
            break;
        case LogLevel::TRACE:
            return "TRACE";
            break;
        case LogLevel::INFO:
            return "INFO";
            break;
        }

        return "";
    }

    std::string CurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%X");
        return ss.str();
    }

    template<typename F, typename L>
    std::string OutputFileInfo(F file, L line)
    {
        std::stringstream output;
        output << file << ":" << line;
        
        return output.str();
    }

    std::string ResetColor() { return TEXT_WHITE; }
    // clang-format on
};

// macros for initialzing and running the logger with minimal code.
#define LFATAL(x)                                                                                                      \
    {                                                                                                                  \
        LOGINIT_COUT();                                                                                                \
        Log(LOG_FATAL, FILE_INFO) << x;                                                                                \
    }

#define LERROR(x)                                                                                                      \
    {                                                                                                                  \
        LOGINIT_COUT();                                                                                                \
        Log(LOG_ERROR, FILE_INFO) << x;                                                                                \
    }

#define LWARN(x, y)                                                                                                    \
    {                                                                                                                  \
        LOGINIT_COUT();                                                                                                \
        if(x)                                                                                                          \
        {                                                                                                              \
            Log(LOG_WARN, FILE_INFO) << y;                                                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            Log(LOG_WARN) << y;                                                                                        \
        }                                                                                                              \
    }

#define LDEBUG(x, y)                                                                                                   \
    {                                                                                                                  \
        LOGINIT_COUT();                                                                                                \
        if(x)                                                                                                          \
        {                                                                                                              \
            Log(LOG_DEBUG, FILE_INFO) << y;                                                                            \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            Log(LOG_DEBUG) << y;                                                                                       \
        }                                                                                                              \
    }

#define LTRACE(x, y)                                                                                                   \
    {                                                                                                                  \
        LOGINIT_COUT();                                                                                                \
        if(x)                                                                                                          \
        {                                                                                                              \
            Log(LOG_TRACE, FILE_INFO) << y;                                                                            \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            Log(LOG_TRACE) << y;                                                                                       \
        }                                                                                                              \
    }

#define LINFO(x, y)                                                                                                    \
    {                                                                                                                  \
        LOGINIT_COUT();                                                                                                \
        if(x)                                                                                                          \
        {                                                                                                              \
            Log(LOG_INFO, FILE_INFO) << y;                                                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            Log(LOG_INFO) << y;                                                                                        \
        }                                                                                                              \
    }