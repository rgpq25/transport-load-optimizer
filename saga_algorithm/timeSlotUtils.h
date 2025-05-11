#ifndef TIMESLOTUTILS_H
#define TIMESLOTUTILS_H

#include <vector>
#include "timeSlot.h"
#include "route.h"

namespace TimeSlotUtils {
    vector<TimeSlot> generateForWindow(
        int startMin, 
        int endMin,
        const Route& route,
        int unloadingTimePerClient,
        int slotIntervalMinutes
    );
}

#endif /* TIMESLOTUTILS_H */

