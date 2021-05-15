#include <vector>
#include <stdint.h>
#include <functional>

namespace Logger
{
    class ILogger
    {
    public:
        virtual ~ILogger() {}
        virtual uint64_t Append(std::vector<uint8_t> data) = 0;
        virtual uint64_t GetPosition() const = 0;
        virtual void Truncate(uint64_t position) = 0;
        virtual void Replay(uint64_t position, std::function<void(std::vector<uint8_t>)> callback) const = 0;
    };
}