#ifdef USE_ANTICHEAT
#error "This file should not be included when the real anticheat module is present"
#endif

#include "Anticheat.hpp"

AnticheatLibInterface* GetAnticheatLib()
{
    static NullAnticheatLib l;
    return &l;
}