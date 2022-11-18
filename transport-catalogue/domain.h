#pragma once
#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <optional>

namespace transport_catalogue {

enum class TypeRoute {
	circle,
	line
};

struct RouteSettings {
	double bus_velocity;
	double bus_wait_time;
};

enum class ActionType {
	ITEM,
	WAIT,
	BUS
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

struct Item {
	Item(double loc_time, std::string_view loc_name = {}, ActionType loc_type = ActionType::ITEM, std::optional<int> loc_span_count = std::nullopt)
		: time(loc_time),
		name(loc_name),
		type(loc_type),
		span_count(loc_span_count)
	{}

	Item() = default;

	double time = 0;
	std::string_view name = {};
	ActionType type = ActionType::ITEM;
	std::optional<int> span_count = std::nullopt;
};

inline bool operator<(const Item& lhs, const Item& rhs) {
	return lhs.time < rhs.time;
}

inline bool operator>(const Item& lhs, const Item& rhs) {
	return lhs.time > rhs.time;
}

inline Item operator+(const Item& lhs, const Item& rhs) {
	return { lhs.time + rhs.time };
}

struct FoundedRoute {
	double total_time = 0;
	std::vector<const Item*> elements;
};
} //namespace transport_catalogue