#include "transport_catalogue.h"

#include <utility>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace transport_catalogue {
using namespace std::literals;

struct LengthToStop {
	LengthToStop() = default;
	LengthToStop(double real_length, double geo_length) :
		real_length_(real_length),
		geo_length_(geo_length)
	{}
	double real_length_;
	double geo_length_;
};

TransportCatalogue::TransportCatalogue() = default;

void TransportCatalogue::AddBus(Bus&& bus) {
	busses_.push_back(std::move(bus));
	auto* ptr_bus = &busses_.back();
	busname_to_bus_[ptr_bus->name] = ptr_bus;
	for (const auto& stop : ptr_bus->unique_stops) {
		stopname_to_busses_[stopname_to_stop_[stop]].insert(ptr_bus->name);
	}
}

void TransportCatalogue::AddStop(Stop&& stop) {
	stops.push_back(std::move(stop));
	stopname_to_stop_[stops.back().name] = &stops.back();
}

const Stop* TransportCatalogue::FindStop(std::string_view stopname) const {
	if (stopname_to_stop_.count(stopname)) {
		return stopname_to_stop_.at(stopname);
	}
	else {
		throw "The stopname is not found!";
	}
}

const Bus* TransportCatalogue::FindBus(std::string_view busname) const {
	if (busname_to_bus_.count(busname)) {
		return busname_to_bus_.at(busname);
	}
	return nullptr;
}

std::optional<BusInfo> TransportCatalogue::GetInfromBus(std::string_view busname) const {
	auto* bus = FindBus(busname);
	if (bus == nullptr) {
		return {};
	}
	size_t stops = (bus->type_route == TypeRoute::circle) ? bus->stops.size() : (2 * bus->stops.size() - 1);
	size_t unique_stops = bus->unique_stops.size();
	std::vector<LengthToStop> lengths;
	lengths.resize(stops);
	auto summarise_circle = [this](const Stop* left, const Stop* right) {
		auto real_length = GetLengthInStops(left, right);
		auto geo_length = geo::ComputeDistance(left->coordinates, right->coordinates);
		return LengthToStop(real_length, geo_length);
	};
	auto summarise_line = [this](const Stop* left, const Stop* right) {
		auto real_length = GetLengthInStops(left, right);
		auto real_length_reverse = GetLengthInStops(right, left);
		auto geo_length = geo::ComputeDistance(left->coordinates, right->coordinates);
		return LengthToStop(real_length + real_length_reverse, 2 * geo_length);
	};
	if (bus->type_route == TypeRoute::circle) {
		std::transform(bus->stops.begin(), std::prev(bus->stops.end()), std::next(bus->stops.begin()), lengths.begin(), summarise_circle);
	}
	else {
		std::transform(bus->stops.begin(), std::prev(bus->stops.end()), std::next(bus->stops.begin()), lengths.begin(), summarise_line);
	}
	LengthToStop length = std::reduce(lengths.begin(), lengths.end(), LengthToStop(0, 0), [](const LengthToStop& left, const LengthToStop& right) {
		return LengthToStop(left.real_length_ + right.real_length_, left.geo_length_ + right.geo_length_);
		});
	auto curvature = length.real_length_ / length.geo_length_;
	return { {stops, unique_stops, length.real_length_, curvature} };
}

std::set<std::string_view> TransportCatalogue::GetListBusses(std::string_view stopname) const {
	if (!stopname_to_stop_.count(stopname)) {
		throw "not found"s;
	}
	else if (!stopname_to_busses_.count(FindStop(stopname))) {
		return {};
	}
	return stopname_to_busses_.at(FindStop(stopname));
}

double TransportCatalogue::GetLengthInStops(const Stop* left, const Stop* right) const {
	auto key = std::make_pair(left, right);
	auto reverse_key = std::make_pair(right, left);
	if (distance_stops_.count(key)) {
		return distance_stops_.at(key);
	}
	else if (distance_stops_.count(reverse_key)) {
		return distance_stops_.at(reverse_key);
	}
	else {
		throw "Distance between these stops is not declared!"s;
	}
}

void TransportCatalogue::SetLengthInStops(const Stop* from, const Stop* to, double length) {
	auto key = std::make_pair(from, to);
	distance_stops_[key] = length;
}

const std::map<std::string_view, const Bus*>& TransportCatalogue::GetBusnameToBus() const {
	return busname_to_bus_;
}

const std::vector<const Stop*> TransportCatalogue::GetValidStops() const {
	std::vector<const Stop*> result;
	for (const auto& [stop, buses] : stopname_to_busses_) {
		if (buses.size()) {
			result.push_back(stop);
		}
	}
	std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
		return lhs->name < rhs->name;
		});
	return result;
}

const std::unordered_map<std::string_view, const Stop*>& TransportCatalogue::GetStopnameToStop() const {
	return stopname_to_stop_;
}

const std::deque<Stop>& TransportCatalogue::GetDequeStops() const {
	return stops;
}
const std::deque<Bus>& TransportCatalogue::GetDequeBusses() const {
	return busses_;
}
const std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::HashTransportCatalogue>& TransportCatalogue::GetDistanceToStops() const {
	return distance_stops_;
}
} //namespace transport_catalogue