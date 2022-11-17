#include "transport_router.h"

namespace transport_router {
using namespace graph;
using namespace transport_catalogue;
using namespace json;
using namespace std::literals;

template <typename Iterator>
void FullfillGraph(Iterator begin, Iterator end, DirectedWeightedGraph<Item>& graph, const RouteSettings& route_settings, 
	const TransportCatalogue& tran_cat, std::string_view busname) {
	const auto& valid_stopname_to_vertex = tran_cat.GetValidStopsVertex();
	for (auto it = begin; it < std::prev(end); ++it) {
		double time = 0;
		double span = 0;
		for (auto it_next = std::next(it); it_next < end; ++it_next) {
			auto vertex1 = valid_stopname_to_vertex.at((*it)->name) + 1;
			auto vertex2 = valid_stopname_to_vertex.at((*it_next)->name);
			time += tran_cat.GetLengthInStops(*std::prev(it_next), *it_next) / route_settings.bus_velocity * 6 / 100;
			++span;
			graph.AddEdge({ vertex1, vertex2, Item(time, busname, ActionType::BUS, span) });
		}
	}
}

TransportRouter::TransportRouter(const TransportCatalogue& tran_cat, RouteSettings&& route_settings) : route_settings_(std::move(route_settings)) {
	const auto& valid_stopname_to_vertex = tran_cat.GetValidStopsVertex();
	graph_ = std::make_unique<graph::DirectedWeightedGraph<Item>>(DirectedWeightedGraph<Item>(2 * valid_stopname_to_vertex.size()));
	for (const auto& [stopname, vertex] : valid_stopname_to_vertex) {
		graph_->AddEdge({ vertex, vertex + 1, Item(route_settings_.bus_wait_time, stopname, ActionType::WAIT) });
	}
	for (const auto& [busname, bus] : tran_cat.GetBusnameToBus()) {
		FullfillGraph(bus->stops.begin(), bus->stops.end(), *graph_, route_settings_, tran_cat, busname);
		if (bus->type_route == TypeRoute::line) {
			FullfillGraph(bus->stops.rbegin(), bus->stops.rend(), *graph_, route_settings_, tran_cat, busname);
		}
	}
	router_ptr_ = std::make_unique<graph::Router<Item>>(Router<Item>(*graph_));
}

std::optional<Router<Item>::RouteInfo> TransportRouter::FindRoute(const json::Dict& request_as_map, const std::map<std::string_view, VertexId>& valid_stopname_to_vertex) const {
	auto& stop_from = request_as_map.at("from"s).AsString();
	auto& stop_to = request_as_map.at("to"s).AsString();
	return router_ptr_->BuildRoute(valid_stopname_to_vertex.at(stop_from), valid_stopname_to_vertex.at(stop_to));
}

const graph::Edge<Item>& TransportRouter::GetGraphEdge(graph::EdgeId edge_id) const {
	return graph_->GetEdge(edge_id);
}

const transport_catalogue::RouteSettings& TransportRouter::GetRouteSettings() const {
	return route_settings_;
}
} //namespace transport_router