#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "ILogger.hpp"
#include "FileWriter.hpp"

namespace Logger
{
    struct LogSettings
    {
        std::string logFileName;
        std::string logFilePath;
        uint16_t maxFileSizeInMB;
    };

    class FileLogger : ILogger
    {
    public:
        FileLogger();
        ~FileLogger();
        bool Initialize(LogSettings settings);

        virtual uint32_t Append(std::vector<uint8_t> data) override;
        virtual uint32_t GetPosition() const override;
        virtual void Truncate(uint32_t position) override;
        virtual void Replay(uint32_t position, std::function<void(std::vector<uint8_t>)> callback) const override;
    private:
        void WriterThread();
        void Push(const LogEntry& entry);
        LogEntry Pop();
        void ReadFile(const std::string& path, uint32_t beginOffset, std::function<void(std::vector<uint8_t>)> callback) const;

    private:
        std::deque<LogEntry> _logQueue;
        std::mutex _queueLock;
        std::condition_variable _messageEvent;
        std::thread _writerThread;
        std::atomic<bool> _isShutdown;
        FileWriter _fileWriter;
        mutable std::mutex _fileLock;
        LogSettings _settings;
        std::atomic<uint32_t> _currentEntryId;
        std::atomic<uint32_t> _currentBegPosition;
        std::mutex _currentStateLock;
        std::unordered_map<uint32_t, uint64_t> _entryIdToFileBegPosition;   // Entry ID to file offset map for the beginning of that log entry
    };
}