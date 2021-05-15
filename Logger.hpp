#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "ILogger.hpp"
#include "FileWriter.hpp"

namespace Logger
{
    struct LogEntry
    {
        LogEntry() : entryId(0), length(0) { }

        uint64_t entryId;
        uint32_t length;
        std::vector<uint8_t> data;
    };

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

        virtual uint64_t Append(std::vector<uint8_t> data) override;
        virtual uint64_t GetPosition() const override;
        virtual void Truncate(uint64_t position) override;
        virtual void Replay(uint64_t position, std::function<void(std::vector<uint8_t>)> callback) const override;

        void WriterThread();
        void Push(const LogEntry& entry);
        LogEntry Pop();

    private:
        std::deque<LogEntry> _logQueue;
        std::mutex _queueLock;
        std::condition_variable _messageEvent;
        std::thread _writerThread;
        std::atomic<bool> _isShutdown;
        FileWriter _fileWriter;
        LogSettings _settings;
    };
}