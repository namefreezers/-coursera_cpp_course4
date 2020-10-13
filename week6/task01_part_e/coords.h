#pragma once


class Coords {
public:
    Coords(double latitude_degrees, double longitude_degrees);

    double operator-(const Coords &other) const;

private:
    constexpr static const double PI = 3.1415926535;

    double latitude_degrees = 0;
    double longitude_degrees = 0;
    double latitude = 0;
    double longitude = 0;
};