#pragma once
#include "geo.h"
#include "domain.h"
#include "json.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <optional>

namespace transport_catalogue {
using VertexId = size_t;

namespace detail {
class HashTransportCatalogue {
public:
	size_t operator()(std::pair<const Stop*, const Stop*> stops) const {
		return stops.first->coordinates.lat * 37 + stops.first->coordinates.lng * 37*37 +
			stops.second->coordinates.lat * 37*37*37 + stops.second->coordinates.lng * 37*37*37*37;
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
	std::optional<BusInfo> GetInfromBus(std::string_view busname) const;
	std::set<std::string_view> GetListBusses(std::string_view stopname) const;
	double GetLengthInStops(const Stop* left, const Stop* right) const;
	void SetLengthInStops(const Stop* from, json::Dict stop_length);

	const std::map<std::string_view, const Bus*>& GetBusnameToBus() const;
	const std::vector<const Stop*> GetValidStops() const;
	void BuildValidStopsVertex();
	const std::map<std::string_view, VertexId>& GetValidStopsVertex() const;
private:
	std::deque<Stop> stops;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::deque<Bus> busses_;
	std::map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::HashTransportCatalogue> distance_stops_;
	std::unordered_map<const Stop*, std::set<std::string_view>> stopname_to_busses_;
	std::map<std::string_view, VertexId> valid_stopname_to_vertex_;
};
} //namespace transport_catalogue