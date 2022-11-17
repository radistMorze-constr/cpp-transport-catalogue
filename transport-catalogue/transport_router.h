#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include "json_builder.h"
#include <memory>

namespace transport_router {
using transport_catalogue::ActionType;
using transport_catalogue::Item;

class TransportRouter {
public:
	TransportRouter() = default;

	TransportRouter(const transport_catalogue::TransportCatalogue& tran_cat, transport_catalogue::RouteSettings&& route_settings);
	std::optional<graph::Router<Item>::RouteInfo> FindRoute(const json::Dict& request_as_map, 
		const std::map<std::string_view, transport_catalogue::VertexId>& valid_stopname_to_vertex) const;
	const graph::Edge<Item>& GetGraphEdge(graph::EdgeId edge_id) const;
	const transport_catalogue::RouteSettings& GetRouteSettings() const;
private:
	transport_catalogue::RouteSettings route_settings_;
	std::unique_ptr<graph::DirectedWeightedGraph<Item>> graph_;
	std::unique_ptr<graph::Router<Item>> router_ptr_;
};
} //namespace transport_router