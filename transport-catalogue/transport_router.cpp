#include "transport_router.h"

#include <algorithm>
#include <iterator>

namespace transport_router {
using namespace graph;
using namespace transport_catalogue;
//using namespace json;
using namespace std::literals;

void TransportRouter::BuildValidStopsVertex(const std::unordered_map<std::string_view, const Stop*>& stopname_to_stop) {
	VertexId index = 0;
	for (const auto& [stopname, stop] : stopname_to_stop) {
		valid_stopname_to_vertex_[stopname] = index;
		index += 2;
	}
}

TransportRouter::TransportRouter(const TransportCatalogue& tran_cat, RouteSettings&& route_settings) : route_settings_(std::move(route_settings)) {
	BuildValidStopsVertex(tran_cat.GetStopnameToStop());
	graph_ = std::make_unique<graph::DirectedWeightedGraph<Item>>(DirectedWeightedGraph<Item>(2 * valid_stopname_to_vertex_.size()));
	for (const auto& [stopname, vertex] : valid_stopname_to_vertex_) {
		graph_->AddEdge({ vertex, vertex + 1, Item(route_settings_.bus_wait_time, stopname, ActionType::WAIT) });
	}
	for (const auto& [busname, bus] : tran_cat.GetBusnameToBus()) {
		FullfillGraph(bus->stops.begin(), bus->stops.end(), tran_cat, busname);
		if (bus->type_route == TypeRoute::line) {
			FullfillGraph(bus->stops.rbegin(), bus->stops.rend(), tran_cat, busname);
		}
	}
	router_ptr_ = std::make_unique<graph::Router<Item>>(Router<Item>(*graph_));
}

std::optional<FoundedRoute> TransportRouter::FindRoute(std::string_view stop_from, std::string_view stop_to) const {
	const auto& route_info = router_ptr_->BuildRoute(valid_stopname_to_vertex_.at(stop_from), valid_stopname_to_vertex_.at(stop_to));
	if (!route_info) {
		return {};
	}
	else {
		std::vector<const Item*> elements;
		std::transform(route_info->edges.begin(), route_info->edges.end(), std::back_inserter(elements), [&](const EdgeId& edge_id) {
			return &graph_->GetEdge(edge_id).weight;
			});
		FoundedRoute founded_route = { route_info->weight.time, elements };
		return founded_route;
	}
}
} //namespace transport_router