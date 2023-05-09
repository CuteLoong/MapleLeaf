#include "Log.hpp"

namespace MapleLeaf {
std::mutex    Log::WriteMutex;
std::ofstream Log::FileStream;


}   // namespace MapleLeaf