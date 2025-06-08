#include "timeSlotUtils.h"

namespace TimeSlotUtils {
    vector<TimeSlot> generateForWindow(
        int startMin, 
        int endMin,
        const Route& route,
        int unloadingTimePerClient,
        int slotIntervalMinutes
    ) {
        vector<TimeSlot> slots;

        // Calculate total duration: travel + unloading at each customer
        int travelTime = 0;
        int clientsVisited = 0;

        for (const RoutePoint& point : route.getRoutePoints()) {
            travelTime += point.getMinutesToClient();
            if (point.getClient()->getId() != 1) { // ID 1 is the dispatch center
                clientsVisited++;
            }
        }

        int unloadingTimeTotal = unloadingTimePerClient * clientsVisited;
        int totalDuration = travelTime + unloadingTimeTotal;

        // Slide through the window using the interval
        for (int t = startMin; t + totalDuration <= endMin; t += slotIntervalMinutes) {
            slots.emplace_back(TimeSlot(t, t + totalDuration));
        }

        return slots;
    }
}