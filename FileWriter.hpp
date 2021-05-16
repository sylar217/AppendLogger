#include <fstream>
#include <vector>
#include <cstring>
#include "LogEntry.hpp"

namespace Logger
{
    class FileWriter
    {
    public:
        FileWriter() { }
        ~FileWriter() { Close(); }

        bool Open(const std::string& path)
        {
            if (path.empty())
                return false;

            _file.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::app);

            if (!_file)
                return false;
            
            return true;
        }

        void Close()
        {
            if (!IsOpen())
                return;
            
            Flush();
            _file.close();
        }

        bool IsOpen() const { return _file.is_open(); }

        bool Write(const void* data, size_t size)
        {
            if (!data || !size)
                return false;

            return !!_file.write((const char*)data, size);
        }

        bool Write(std::vector<uint8_t>& data)
        {
            if (data.empty())
                return false;

            return Write((const char*)data.data(), data.size());
        }

        void Write(const LogEntry& entry)
        {
            _file << entry;
        }

        void Flush() { _file.flush(); }

    private:
        std::ofstream _file;
    };
}