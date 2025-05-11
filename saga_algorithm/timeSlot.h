/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   timeSlot.h
 * Author: renzo
 *
 * Created on 11 de mayo de 2025, 14:47
 */

#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <string>

using namespace std;

class TimeSlot {
private:
    int startMinute; // Minutes from midnight (e.g. 480 = 08:00)
    int endMinute;

public:
    TimeSlot();
    TimeSlot(const string& startTimeStr, const string& endTimeStr);
    TimeSlot(int startMinute, int endMinute);

    int getStart() const;
    int getEnd() const;

    string getStartAsString() const;
    string getEndAsString() const;
};

#endif /* TIMESLOT_H */

