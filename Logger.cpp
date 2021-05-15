#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include "Logger.hpp"

namespace Logger
{
    FileLogger::FileLogger(): _messageEvent(), _writerThread(), _isShutdown(false) { }

    bool FileLogger::Initialize(LogSettings settings)
    {
        _settings = settings;
        if (settings.logFilePath.empty())
            _settings.logFilePath = "/tmp/logstore/";
        
        if (settings.logFileName.empty())
        {
            std::time_t now = std::time(nullptr);
            struct tm nowTm = {0};
            gmtime_r(&now, &nowTm);
            std::string logFileName;
            char fileName[260] = {0};
            size_t len = strftime(fileName, (sizeof(fileName) / sizeof(*fileName)), "log_ymd%Y-%m-%d_hms%H-%M-%S", &nowTm);
            snprintf(fileName + len, (sizeof(fileName) / sizeof(*fileName)) - len, "_pid%u.txt", getpid());

            _settings.logFileName = fileName;
        }

        int status = mkdir(_settings.logFilePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (status == 0 || errno == EEXIST)
        {
            if (!_fileWriter.Open(_settings.logFilePath +_settings.logFileName))
                return false;

            return true;
        }

        return false;
    }

    FileLogger::~FileLogger()
    {
        _isShutdown.store(true);
        if (_writerThread.joinable())
            _writerThread.join();
        
        _fileWriter.Close();
        _logQueue.clear();
    }

    void FileLogger::Push(const LogEntry& entry)
    {
        {
            std::lock_guard<std::mutex> lg(_queueLock);
            if (!_isShutdown.load())
            {
                _logQueue.emplace_back(std::move(entry));
            }
        }

        _messageEvent.notify_all();
    }

    LogEntry FileLogger::Pop()
    {
        std::unique_lock<std::mutex> ul(_queueLock);

        if (_logQueue.empty())
            _messageEvent.wait(ul);

        if (_logQueue.empty())
            return LogEntry();

        LogEntry entry = std::move(_logQueue.front());
        _logQueue.pop_front();

        return std::move(entry);
    }

    void FileLogger::WriterThread()
    {
        while (true)
        {
            if (_isShutdown.load())
                return;

            LogEntry entry = Pop();
            if (entry.length > 0)
            {
                // Serialize the LogEntry and write to file
            }
        }
    }

    uint64_t FileLogger::Append(std::vector<uint8_t> data)
    {
        return 0;
    }

    uint64_t FileLogger::GetPosition() const
    {
        return 0;
    }

    void FileLogger::FileLogger::Truncate(uint64_t position)
    {

    }

    void FileLogger::Replay(uint64_t position, std::function<void(std::vector<uint8_t>)> callback) const
    {

    }
}