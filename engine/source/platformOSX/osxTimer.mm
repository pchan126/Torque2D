#include "platform/platformTimer.h"


PlatformTimer *PlatformTimer::create()
{
   return new DefaultPlatformTimer();
}