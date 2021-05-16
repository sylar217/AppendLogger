#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include "Logger.hpp"

namespace Logger
{
    FileLogger::FileLogger() :
         _messageEvent(), _writerThread(),
         _isShutdown(false), _currentEntryId(0),
         _currentBegPosition(0)
    { }

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

            // Initialize entry ID and file begin position
            _currentEntryId.store(1);
            _currentBegPosition.store(0);

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
            if (entry.GetDataLength() > 0)
            {
                std::lock_guard<std::mutex> lg(_fileLock);
                _fileWriter.Write(entry);
            }
        }
    }

    void FileLogger::ReadFile(const std::string& path, uint32_t beginOffSet, std::function<void(std::vector<uint8_t>)> callback) const
    {
        std::ifstream file;
        int lengthToRead;
        {
            std::lock_guard<std::mutex> lg(_fileLock);
            file = std::ifstream(path.c_str(), std::ios::in | std::ios::binary);

            if (!file)
                return;

            file.seekg(0, file.end);
            lengthToRead = file.tellg();
            file.seekg(beginOffSet, file.beg);
            lengthToRead -= beginOffSet;
        }

        while (lengthToRead > 0)
        {
            LogEntry entry;
            file >> entry;

            lengthToRead -= 4 + 8 + 2 + entry.GetDataLength();
            callback(std::move(entry.GetData()));
        }
    }

    uint32_t FileLogger::Append(std::vector<uint8_t> data)
    {
        if (data.empty())
            return 0;

        std::lock_guard<std::mutex> lg(_currentStateLock);

        uint32_t prevEntry = std::atomic_fetch_add(&_currentEntryId, 1);

        uint32_t fileOffset = 4 + 8 + 2 + (uint32_t)data.size(); // entryId + length + two comma separator + data size
        uint32_t prevBegPosition = std::atomic_fetch_add(&_currentBegPosition, fileOffset);
        _entryIdToFileBegPosition.insert(std::make_pair(prevEntry, prevBegPosition));

        LogEntry entry = {prevEntry, (uint32_t)data.size(), data};
        Push(entry);

        return entry.GetEntryId();
    }

    uint32_t FileLogger::GetPosition() const
    {
        return _currentEntryId.load() - 1;
    }

    void FileLogger::FileLogger::Truncate(uint32_t position)
    {

    }

    void FileLogger::Replay(uint32_t position, std::function<void(std::vector<uint8_t>)> callback) const
    {
        auto itr = _entryIdToFileBegPosition.find(position);
        if (itr == _entryIdToFileBegPosition.end())
            return;

        uint32_t begPosition = itr->second;

        ReadFile(_settings.logFilePath + _settings.logFileName, begPosition, callback);
    }
}