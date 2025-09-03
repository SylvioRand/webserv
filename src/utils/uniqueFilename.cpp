
#include "../../include/utils/utils.hpp"

std::string unique_filename(const std::string &original)
{
    std::time_t t = std::time(NULL);
    int r = std::rand() % 100000;

    char buffer[128];
    std::sprintf(buffer, "%s_%ld_%d", original.c_str(), static_cast<long>(t), r);

    return std::string(buffer);
}
