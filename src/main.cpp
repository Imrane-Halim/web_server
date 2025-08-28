#include "Logger.hpp"

int main()
{
    Logger console;

    try
    {
        Logger file("file");
        file.info("this is a test log message");
        file.error("this is a test log message");
        file.warning("this is a test log message");
        file.debug("this is a test log message");
        file.success("this is a test log message");
    }
    catch(const std::exception& e)
    {
        console.error(e.what());
    }

    console.info("this is a test log message");
    console.error("this is a test log message");
    console.warning("this is a test log message");
    console.debug("this is a test log message");
    console.success("this is a test log message");

    console.custom("something strange", "helo world", 0x0);
    std::cout << console.getCustomLine("tag", "hdflaklfda", 0xffff00) << std::endl;

    return 0;
}