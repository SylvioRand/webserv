
#include "../../include/utils/utils.hpp"
#include <cstddef>

std::string unique_filename(const std::string &original)
{
    std::time_t t = std::time(NULL);
    int r = std::rand() % 100000;
    std::string filename;
    size_t  pos = original.rfind(".");
    std::string ext;
    char buffer[128];

    if (pos != std::string::npos && original.at(original.size() - 1) != '.')
    {
      filename = original.substr(0, original.size() - pos + 1);
      ext = original.substr(pos);
      std::sprintf(buffer, "%s_%ld_%d%s", filename.c_str(), static_cast<long>(t), r, ext.c_str());
    }
    else
    {
      filename = original;
      std::sprintf(buffer, "%s_%ld_%d", filename.c_str(), static_cast<long>(t), r);
    }
    return (std::string(buffer));
}
