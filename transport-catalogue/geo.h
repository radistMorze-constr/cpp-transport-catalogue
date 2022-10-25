#pragma once

#include <iostream>

namespace geo {
struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Coordinates& coordinates) {
    using namespace std::literals;
    return os << coordinates.lat << ", "s << coordinates.lng;
}

double ComputeDistance(Coordinates from, Coordinates to);
}