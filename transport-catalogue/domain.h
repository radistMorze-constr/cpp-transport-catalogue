#pragma once
#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

namespace transport_catalogue {
enum class TypeRoute {
	circle,
	line
};

struct Stop {
	std::string name;
	geo::Coordinates coordinates;
};

struct Bus {
	Bus(const std::string&& name, std::vector<const Stop*>&& stops, TypeRoute&& type_route, std::unordered_set<std::string_view>&& unique_stops);

	std::string name;
	std::vector<const Stop*> stops;
	TypeRoute type_route;
	std::unordered_set<std::string_view> unique_stops;
};

struct BusInfo {
	size_t amount_stops;
	size_t amount_unique_stops;
	double length;
	double curvature;
};
} //namespace transport_catalogue