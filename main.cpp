#include <iostream>
#include <vector>
#include "Logger.hpp"

int main()
{
    Logger::LogSettings settings;
    settings.maxFileSizeInMB = 100;

    std::unique_ptr<Logger::FileLogger> logger(std::make_unique<Logger::FileLogger>());
    if (logger && logger->Initialize(settings))
    {
        std::string data = "Hello world";
        std::vector<uint8_t> vec(data.begin(), data.end());
        uint32_t pos = logger->Append(vec);
        logger->Replay(pos, [](std::vector<uint8_t> content)
        {
            std::string str(content.begin(), content.end());
            std::cout << str << std::endl;
        });
    }

    return 0;
}