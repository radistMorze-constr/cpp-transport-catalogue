#include "transport_catalogue.h"

#include <utility>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace transport_catalogue {
struct LengthToStop {
	LengthToStop() = default;
	LengthToStop(double real_length, double geo_length) :
		real_length_(real_length),
		geo_length_(geo_length)
	{}
	double real_length_;
	double geo_length_;
};

Bus::Bus(std::string name, std::vector<const Stop*> stops, TypeRoute type_route, std::unordered_set<std::string_view> unique_stops) :
	name_(name),
	stops_(stops),
	type_route_(type_route),
	unique_stops_(unique_stops)
{
}

TransportCatalogue::TransportCatalogue() = default;

void TransportCatalogue::AddBus(Bus&& bus) {
	busses_.push_back(std::move(bus));
	auto* ptr_bus = &busses_.back();
	busname_to_bus_[ptr_bus->name_] = ptr_bus;
	for (const auto& stop : ptr_bus->unique_stops_) {
		stopname_to_busses_[stopname_to_stop_[stop]].insert(ptr_bus->name_);
	}
}

void TransportCatalogue::AddStop(Stop&& stop) {
	stops_.push_back(std::move(stop));
	stopname_to_stop_[stops_.back().name_] = &stops_.back();
}

const Stop* TransportCatalogue::FindStop(std::string_view stopname) {
	if (stopname_to_stop_.count(stopname)) {
		return stopname_to_stop_[stopname];
	}
	else {
		throw "The stopname is not found!";
	}
}

const Bus* TransportCatalogue::FindBus(std::string_view busname) {
	if (busname_to_bus_.count(busname)) {
		return busname_to_bus_[busname];
	}
	return nullptr;
}

BusInfo TransportCatalogue::GetInfromBus(std::string_view busname) {
	auto* bus = FindBus(busname);
	if (bus == nullptr) {
		return { 0, 0, 0, 0, true };
	}
	size_t stops = (bus->type_route_ == TypeRoute::circle) ? bus->stops_.size() : (2 * bus->stops_.size() - 1);
	size_t unique_stops = bus->unique_stops_.size();
	std::vector<LengthToStop> lengths;
	lengths.resize(stops);
	auto summarise_circle = [this](const Stop* left, const Stop* right) {
		auto real_length = GetLengthInStops(left, right);
		auto geo_length = geo::ComputeDistance(left->coordinates_, right->coordinates_);
		return LengthToStop(real_length, geo_length);
	};
	auto summarise_line = [this](const Stop* left, const Stop* right) {
		auto real_length = GetLengthInStops(left, right);
		auto real_length_reverse = GetLengthInStops(right, left);
		auto geo_length = geo::ComputeDistance(left->coordinates_, right->coordinates_);
		return LengthToStop(real_length + real_length_reverse, 2 * geo_length);
	};
	if (bus->type_route_ == TypeRoute::circle) {
		std::transform(bus->stops_.begin(), std::prev(bus->stops_.end()), std::next(bus->stops_.begin()), lengths.begin(), summarise_circle);
	}
	else {
		std::transform(bus->stops_.begin(), std::prev(bus->stops_.end()), std::next(bus->stops_.begin()), lengths.begin(), summarise_line);
	}
	LengthToStop length = std::reduce(lengths.begin(), lengths.end(), LengthToStop(0, 0), [](const LengthToStop& left, const LengthToStop& right) {
		return LengthToStop(left.real_length_ + right.real_length_, left.geo_length_ + right.geo_length_);
		});
	auto curvature = length.real_length_ / length.geo_length_;
	return { stops, unique_stops, length.real_length_, curvature, false };
}

std::set<std::string_view> TransportCatalogue::GetListBusses(std::string_view stopname) {
	if (!stopname_to_stop_.count(stopname)) {
		throw "not found"s;
	}
	else if (!stopname_to_busses_[FindStop(stopname)].size()) {
		throw "no buses"s;
	}
	return stopname_to_busses_[FindStop(stopname)];
}

double TransportCatalogue::GetLengthInStops(const Stop* left, const Stop* right) {
	auto key = std::make_pair(left, right);
	auto reverse_key = std::make_pair(right, left);
	if (distance_stops_.count(key)) {
		return distance_stops_[key];
	}
	else if (distance_stops_.count(reverse_key)) {
		return distance_stops_[reverse_key];
	}
	else {
		throw "Distance between these stops is not declared!"s;
	}
}

void TransportCatalogue::SetLengthInStops(const Stop* from, std::unordered_map<std::string, double> stop_length) {
	for (const auto& [stop, length] : stop_length) {
		auto key = std::make_pair(from, FindStop(stop));
		distance_stops_[key] = length;
	}
}
}