#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport_router {
using transport_catalogue::ActionType;
using transport_catalogue::Item;

using VertexId = size_t;

class TransportRouter {
public:
	TransportRouter() = default;

	TransportRouter(const transport_catalogue::TransportCatalogue& tran_cat, transport_catalogue::RouteSettings&& route_settings);
	std::optional<transport_catalogue::FoundedRoute> FindRoute(std::string_view stop_from, std::string_view stop_to) const;
protected:
	void BuildValidStopsVertex(const std::unordered_map<std::string_view, const transport_catalogue::Stop*>& stopname_to_stop);
	template <typename Iterator>
	void FullfillGraph(Iterator begin, Iterator end, const transport_catalogue::TransportCatalogue& tran_cat, std::string_view busname);
private:
	transport_catalogue::RouteSettings route_settings_;
	std::unique_ptr<graph::DirectedWeightedGraph<Item>> graph_;
	std::unique_ptr<graph::Router<Item>> router_ptr_;
	std::map<std::string_view, VertexId> valid_stopname_to_vertex_;
};

template <typename Iterator>
void TransportRouter::FullfillGraph(Iterator begin, Iterator end, const transport_catalogue::TransportCatalogue& tran_cat, std::string_view busname) {
	for (auto it = begin; it < std::prev(end); ++it) {
		double time = 0;
		double span = 0;
		for (auto it_next = std::next(it); it_next < end; ++it_next) {
			auto vertex1 = valid_stopname_to_vertex_.at((*it)->name) + 1;
			auto vertex2 = valid_stopname_to_vertex_.at((*it_next)->name);
			time += tran_cat.GetLengthInStops(*std::prev(it_next), *it_next) / route_settings_.bus_velocity * 6 / 100;
			++span;
			graph_->AddEdge({ vertex1, vertex2, Item(time, busname, ActionType::BUS, span) });
		}
	}
}
} //namespace transport_router