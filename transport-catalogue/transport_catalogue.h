#pragma once
#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>

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
	Bus(std::string&& name, std::vector<const Stop*>&& stops, TypeRoute&& type_route, std::unordered_set<std::string_view>&& unique_stops);

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
	bool not_found;
};

namespace detail {
class HashTransportCatalogue {
public:
	size_t operator()(std::pair<const Stop*, const Stop*> stops) const {
		return stops.first->coordinates.lat * std::pow(37, 1) + stops.first->coordinates.lng * std::pow(37, 2) +
			stops.second->coordinates.lat * std::pow(37, 3) + stops.second->coordinates.lng * std::pow(37, 4);
	}
};
}

class TransportCatalogue {
public:
	TransportCatalogue();

	void AddBus(Bus&& bus);
	void AddStop(Stop&& stop);
	const Stop* FindStop(std::string_view stopname) const;
	const Bus* FindBus(std::string_view busname) const;
	BusInfo GetInfromBus(std::string_view busname) const;
	std::set<std::string_view> GetListBusses(std::string_view stopname) const;
	double GetLengthInStops(const Stop* left, const Stop* right) const;
	void SetLengthInStops(const Stop* from, std::unordered_map<std::string, double> stop_length);
private:
	std::deque<Stop> stops;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::deque<Bus> busses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::HashTransportCatalogue> distance_stops_;
	std::unordered_map<const Stop*, std::set<std::string_view>> stopname_to_busses_;
};
}