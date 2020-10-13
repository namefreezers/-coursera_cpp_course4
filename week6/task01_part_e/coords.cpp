#include <cmath>

#include "coords.h"


double Coords::operator-(const Coords &other) const {
    return acos(
            sin(this->latitude) * sin(other.latitude) +
            cos(this->latitude) * cos(other.latitude) *
            cos(fabs(this->longitude - other.longitude))
    ) * 6371000;
}


Coords::Coords(double latitude_degrees, double longitude_degrees) : latitude_degrees(latitude_degrees), longitude_degrees(longitude_degrees),
                                                                    latitude(latitude_degrees / 180 * PI), longitude(longitude_degrees / 180 * PI) {}