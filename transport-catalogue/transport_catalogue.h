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
using namespace std::literals;

enum TypeRoute {
	circle,
	line
};

struct Stop {
	std::string name_;
	geo::Coordinates coordinates_;
};

struct Bus {
	Bus(std::string name, std::vector<const Stop*> stops, TypeRoute type_route, std::unordered_set<std::string_view> unique_stops);

	std::string name_;
	std::vector<const Stop*> stops_;
	TypeRoute type_route_;
	std::unordered_set<std::string_view> unique_stops_;
};

struct BusInfo {
	size_t amount_stops_;
	size_t amount_unique_stops_;
	double length_;
	double curvature_;
	bool not_found_;
};

class HashTransportCatalogue {
public:
	size_t operator()(std::pair<const Stop*, const Stop*> stops) const {
		return stops.first->coordinates_.lat * std::pow(37, 1) + stops.first->coordinates_.lng * std::pow(37, 2) +
			stops.second->coordinates_.lat * std::pow(37, 3) + stops.second->coordinates_.lng * std::pow(37, 4);
	}
};

class TransportCatalogue {
public:
	TransportCatalogue();

	void AddBus(Bus&& bus);
	void AddStop(Stop&& stop);
	const Stop* FindStop(std::string_view stopname);
	const Bus* FindBus(std::string_view busname);
	BusInfo GetInfromBus(std::string_view busname);
	std::set<std::string_view> GetListBusses(std::string_view stopname);
	double GetLengthInStops(const Stop* left, const Stop* right);
	void SetLengthInStops(const Stop* from, std::unordered_map<std::string, double> stop_length);
private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::deque<Bus> busses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, double, HashTransportCatalogue> distance_stops_;
	std::unordered_map<const Stop*, std::set<std::string_view>> stopname_to_busses_;
};
}