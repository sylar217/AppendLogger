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
        std::cout << "Hello world" << std::endl;
    }

    return 0;
}