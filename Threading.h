#ifndef RCP_THREADING_H
#define RCP_THREADING_H

#include <mutex>

namespace rcp
{

class Threading
{
public:
    static std::recursive_mutex mutex;
};

} // namespace rcp

#endif // RCP_THREADING_H
