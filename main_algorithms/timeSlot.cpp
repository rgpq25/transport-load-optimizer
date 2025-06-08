#include "timeSlot.h"
#include <sstream>
#include <iomanip>

TimeSlot::TimeSlot() {
    this->startMinute = 0;
    this->endMinute = 0;
}

TimeSlot::TimeSlot(const string& startTimeStr, const string& endTimeStr) {
    int startHour = stoi(startTimeStr.substr(0, 2));
    int startMin = stoi(startTimeStr.substr(3, 2));
    int endHour = stoi(endTimeStr.substr(0, 2));
    int endMin = stoi(endTimeStr.substr(3, 2));

    this->startMinute = startHour * 60 + startMin;
    this->endMinute = endHour * 60 + endMin;
}

TimeSlot::TimeSlot(int start, int end) {
    this->startMinute = start;
    this->endMinute = end;
}


int TimeSlot::getStart() const { return startMinute; }
int TimeSlot::getEnd() const { return endMinute; }


string TimeSlot::getStartAsString() const {
    int hours = startMinute / 60;
    int minutes = startMinute % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << hours
        << ":" << setw(2) << setfill('0') << minutes;
    return oss.str();
}

string TimeSlot::getEndAsString() const {
    int hours = endMinute / 60;
    int minutes = endMinute % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << hours
        << ":" << setw(2) << setfill('0') << minutes;
    return oss.str();
}