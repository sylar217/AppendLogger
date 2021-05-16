#include <vector>
#include <stdint.h>
#include <fstream>
#include <iostream>

namespace Logger
{
    class LogEntry
    {
    public:
        LogEntry() : _entryId(0), _length(0) { }
        LogEntry(uint32_t entryId, uint32_t length, const std::vector<uint8_t>& data) :
            _entryId(entryId), _length(length), _data(data)
        { }

        uint32_t GetEntryId() const { return _entryId; }
        uint32_t GetDataLength() const { return _length; }
        std::vector<uint8_t> GetData() const { return _data; }

        std::ostream& Serialize(std::ostream& out) const
        {
            out << _entryId;
            out << ','; //separator
            out << _length; // size of data
            out << ','; //separator
            out << _data.data();

            return out;
        }

        std::istream& Deserialize(std::istream& in)
        {
            if (in)
            {
                char comma;
                in >> _entryId;
                in >> comma;
                in >> _length;
                in >> comma;
                if (_length)
                {
                    std::vector<uint8_t> tmp(_length);
                    in.read(reinterpret_cast<char*>(tmp.data()), _length);
                    _data = std::move(tmp);
                }
            }

            return in;
        }

    private:
        uint32_t _entryId;
        uint32_t _length;
        std::vector<uint8_t> _data;
    };

    inline std::ostream& operator<<(std::ostream& out, const LogEntry& obj)
    {
        obj.Serialize(out);
        return out;
    }

    inline std::istream& operator>>(std::istream& in, LogEntry& obj)
    {
        obj.Deserialize(in);
        return in;
    }
}