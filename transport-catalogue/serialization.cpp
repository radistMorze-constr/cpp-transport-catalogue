
#include <fstream>
#include <iostream>
#include <memory>

#include "serialization.h"

using namespace transport_catalogue;

namespace serialization {
transport_catalogue_serialize::TransportCatalogue* SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& tran_cat) {
	auto proto_tran_cat = new transport_catalogue_serialize::TransportCatalogue;
	const auto& tran_cat_stops = tran_cat.GetDequeStops();
	for (const auto& stop : tran_cat_stops) {
		auto proto_stop = proto_tran_cat->mutable_stops()->Add();
		auto proto_coordinates = new transport_catalogue_serialize::Coordinates{};
		proto_coordinates->set_lat(stop.coordinates.lat);
		proto_coordinates->set_lng(stop.coordinates.lng);
		proto_stop->set_name(stop.name);
		proto_stop->set_allocated_coordinates(proto_coordinates);
	}
	const auto& tran_cat_busses = tran_cat.GetDequeBusses();
	for (const auto& bus : tran_cat_busses) {
		auto proto_bus = proto_tran_cat->mutable_busses()->Add();
		proto_bus->set_name(bus.name);
		bus.type_route == TypeRoute::circle ? proto_bus->set_type_route(transport_catalogue_serialize::TYPEROUTE_CIRCLE) :
			proto_bus->set_type_route(transport_catalogue_serialize::TYPEROUTE_LINE);
		for (const auto& stop : bus.stops) {
			auto proto_stop = proto_bus->mutable_stops()->Add();
			*proto_stop = stop->name;
		}
		for (const auto& stop : bus.unique_stops) {
			auto proto_stop = proto_bus->mutable_unique_stops()->Add();
			*proto_stop = stop;
		}
	}
	const auto& tran_cat_distance_stops = tran_cat.GetDistanceToStops();
	for (const auto& [stops, distance] : tran_cat_distance_stops) {
		auto proto_distance_stops = proto_tran_cat->mutable_distance_to_stops()->Add();
		*proto_distance_stops->mutable_stop_from() = stops.first->name;
		*proto_distance_stops->mutable_stop_to() = stops.second->name;
		proto_distance_stops->set_distance(distance);
	}
	return proto_tran_cat;
}

void SetProtoColor(const svg::Color& color, svg_serialize::Color* proto_color) {
	if (color.index() == 1) {
		proto_color->set_color_type(svg_serialize::COLORTYPE_STRING);
		proto_color->set_color_string(std::get<1>(color));
	}
	else if (color.index() == 2) {
		proto_color->set_color_type(svg_serialize::COLORTYPE_RGB);
		proto_color->mutable_color_rgb()->set_red(std::get<2>(color).red);
		proto_color->mutable_color_rgb()->set_green(std::get<2>(color).green);
		proto_color->mutable_color_rgb()->set_blue(std::get<2>(color).blue);
	}
	else if (color.index() == 3) {
		proto_color->set_color_type(svg_serialize::COLORTYPE_RGBA);
		proto_color->mutable_color_rgba()->set_red(std::get<3>(color).red);
		proto_color->mutable_color_rgba()->set_green(std::get<3>(color).green);
		proto_color->mutable_color_rgba()->set_blue(std::get<3>(color).blue);
		proto_color->mutable_color_rgba()->set_opacity(std::get<3>(color).opacity);
	}
	else {
		proto_color->set_color_type(svg_serialize::COLORTYPE_MONOSTATE);
	}
}

rendering_serialize::RenderSettings* SerializeMapRender(const transport_catalogue::rendering::RenderSettings& render_settings) {
	auto proto_render_settings = new rendering_serialize::RenderSettings;

	proto_render_settings->set_width(render_settings.width);
	proto_render_settings->set_height(render_settings.height);
	proto_render_settings->set_padding(render_settings.padding);
	proto_render_settings->set_line_width(render_settings.line_width);
	proto_render_settings->set_stop_radius(render_settings.stop_radius);

	proto_render_settings->set_bus_label_font_size(render_settings.bus_label_font_size);
	auto proto_bus_offset = new rendering_serialize::LableOffset{};
	proto_bus_offset->set_first(render_settings.bus_label_offset.first);
	proto_bus_offset->set_second(render_settings.bus_label_offset.second);
	proto_render_settings->set_allocated_bus_label_offset(proto_bus_offset);

	proto_render_settings->set_stop_label_font_size(render_settings.stop_label_font_size);
	auto proto_stop_offset = new rendering_serialize::LableOffset{};
	proto_stop_offset->set_first(render_settings.stop_label_offset.first);
	proto_stop_offset->set_second(render_settings.stop_label_offset.second);
	proto_render_settings->set_allocated_stop_label_offset(proto_stop_offset);

	auto proto_underlayer_color = proto_render_settings->mutable_underlayer_color();
	SetProtoColor(render_settings.underlayer_color, proto_underlayer_color);

	proto_render_settings->set_underlayer_width(render_settings.underlayer_width);

	for (const auto& color : render_settings.color_palette) {
		auto proto_color = proto_render_settings->mutable_color_palette()->Add();
		SetProtoColor(color, proto_color);
	}
	return proto_render_settings;
}

void SetProtoItem(graph_serialize::Item* proto_item, const transport_catalogue::Item& item) {
	proto_item->set_time(item.time);
	proto_item->set_name(std::string(item.name));
	if (item.type == transport_catalogue::ActionType::BUS) {
		proto_item->set_type(graph_serialize::ACTION_TYPE_BUS);
	}
	else if (item.type == transport_catalogue::ActionType::WAIT) {
		proto_item->set_type(graph_serialize::ACTION_TYPE_WAIT);
	}
	else {
		proto_item->set_type(graph_serialize::ACTION_TYPE_ITEM);
	}
	if (item.span_count.has_value()) {
		proto_item->set_span_count(*item.span_count);
	}
}

transport_router_serialize::TransportRouter* SerializeTransportRouter(const transport_router::TransportRouter& tran_router) {
	auto proto_tran_router = new transport_router_serialize::TransportRouter;
	{ //RouteSettings
		auto proto_route_settings = new transport_router_serialize::RouteSettings;
		const auto& route_settings = tran_router.GetRouteSettings();
		proto_route_settings->set_bus_velocity(route_settings.bus_velocity);
		proto_route_settings->set_bus_wait_time(route_settings.bus_wait_time);

		proto_tran_router->set_allocated_route_settings(proto_route_settings);
	} //RouteSettings
	
	{ //Graph
		auto proto_graph = new graph_serialize::Graph;
		const auto& graph = tran_router.GetGraph();
		const auto& edges = graph.GetEdges();
		const auto& incidence_lists = graph.GetIncidenceLists();
		for (const auto& edge : edges) {
			auto proto_edge = proto_graph->mutable_edges()->Add();
			proto_edge->set_to(edge.to);
			proto_edge->set_from(edge.from);
			auto proto_item = proto_edge->mutable_weight();
			SetProtoItem(proto_item, edge.weight);
		}
		for (const auto& incidence_list : incidence_lists) {
			auto proto_incidence_list = proto_graph->mutable_incidence_lists()->Add();
			for (const auto& edge_id : incidence_list) {
				proto_incidence_list->add_edge_id(edge_id);
			}
		}
		proto_tran_router->set_allocated_graph(proto_graph);
	} //Graph
	
	{ //Router
		auto proto_router = new transport_router_serialize::Router;
		const auto& router = tran_router.GetRouter();
		const auto& routes_internal_data = router.GetRoutesInternalData();
		for (const auto& route_internal : routes_internal_data) {
			auto proto_route_internal = proto_router->mutable_routesinternaldata()->Add();
			for (const auto& data : route_internal) {
				auto proto_data = proto_route_internal->mutable_values()->Add();
				proto_data->Clear();
				if (!data.has_value()) {
					continue;
				}
				auto proto_item = new graph_serialize::Item;
				SetProtoItem(proto_item, data->weight);
				proto_data->set_allocated_weight(proto_item);
				if (data->prev_edge.has_value()) {
					auto proto_edge_id = new transport_router_serialize::EdgeId;
					proto_edge_id->set_value(*data->prev_edge);
					proto_data->set_allocated_prev_edge(proto_edge_id);
				}
			}
		}
		proto_tran_router->set_allocated_router(proto_router);
	} //Router

	{ //StopnamesToVertex
		const auto& stopnames_to_vertex = tran_router.GetStopnameToVertex();
		for (const auto& [stopname, vertex] : stopnames_to_vertex) {
			auto proto_stopname_to_vertex = proto_tran_router->add_stopnames_to_vertex();;
			proto_stopname_to_vertex->set_stopname(stopname);
			proto_stopname_to_vertex->set_vertex(vertex);
		}
	} //StopnamesToVertex
	return proto_tran_router;
}

void SerializeFacade(transport_catalogue_serialize::TransportCatalogue* proto_tran_cat,
	rendering_serialize::RenderSettings* proto_render_settings,
	transport_router_serialize::TransportRouter* proto_tran_router,
	std::string filename) {
	std::ofstream out_file(filename, std::ios::binary);
	transport_catalogue_serialize::Facade proto_facade;
	proto_facade.set_allocated_tran_cat(proto_tran_cat);
	proto_facade.set_allocated_render_settings(proto_render_settings);
	proto_facade.set_allocated_tran_router(proto_tran_router);
	proto_facade.SerializeToOstream(&out_file);
}

transport_catalogue::TransportCatalogue DeserializeTransportCatalogue(const transport_catalogue_serialize::TransportCatalogue& proto_tran_cat) {
	transport_catalogue::TransportCatalogue tran_cat{};
	for (int i = 0; i < proto_tran_cat.stops_size(); ++i) {
		const auto& proto_stop = proto_tran_cat.stops(i);
		transport_catalogue::Stop stop = { proto_stop.name(), {proto_stop.coordinates().lat(), proto_stop.coordinates().lng()} };
		tran_cat.AddStop(std::move(stop));
	}
	for (int i = 0; i < proto_tran_cat.distance_to_stops_size(); ++i) {
		const auto& proto_distance_stops = proto_tran_cat.distance_to_stops(i);
		auto stop_from = tran_cat.FindStop(proto_distance_stops.stop_from());
		auto stop_to = tran_cat.FindStop(proto_distance_stops.stop_to());
		tran_cat.SetLengthInStops(stop_from, stop_to, proto_distance_stops.distance());
	}
	for (int i = 0; i < proto_tran_cat.busses_size(); ++i) {
		const auto& proto_bus = proto_tran_cat.busses(i);
		TypeRoute type_route;
		proto_bus.type_route() == transport_catalogue_serialize::TYPEROUTE_CIRCLE ? type_route = TypeRoute::circle : type_route = TypeRoute::line;
		std::vector<const transport_catalogue::Stop*> stops;
		std::unordered_set<std::string_view> unique_stops;
		for (int j = 0; j < proto_bus.stops_size(); ++j) {
			stops.push_back(tran_cat.FindStop(proto_bus.stops(j)));
			unique_stops.insert(stops.back()->name);
		}
		tran_cat.AddBus({ std::string(proto_bus.name()), std::move(stops), std::move(type_route), std::move(unique_stops) });
	}
	return tran_cat;
}

svg::Color MakeColor(const svg_serialize::Color& proto_color) {
	svg::Color color;
	if (proto_color.color_type() == svg_serialize::COLORTYPE_STRING) {
		color = proto_color.color_string();
	}
	else if (proto_color.color_type() == svg_serialize::COLORTYPE_RGB) {
		svg::Rgb rgb(proto_color.color_rgb().red(), proto_color.color_rgb().green(), proto_color.color_rgb().blue());
		color = rgb;
	}
	else if (proto_color.color_type() == svg_serialize::COLORTYPE_RGBA) {
		svg::Rgba rgba(proto_color.color_rgba().red(), proto_color.color_rgba().green(), proto_color.color_rgba().blue(), proto_color.color_rgba().opacity());
		color = rgba;
	}
	return color;
}

transport_catalogue::rendering::RenderSettings DeserializeSerializeRenderSettings(const rendering_serialize::RenderSettings proto_render_settings) {
	transport_catalogue::rendering::RenderSettings render_settings{};
	render_settings.width = proto_render_settings.width();
	render_settings.height = proto_render_settings.height();
	render_settings.padding = proto_render_settings.padding();
	render_settings.line_width = proto_render_settings.line_width();
	render_settings.stop_radius = proto_render_settings.stop_radius();

	render_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();
	render_settings.bus_label_offset = std::make_pair(proto_render_settings.bus_label_offset().first(),
		proto_render_settings.bus_label_offset().second());

	render_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();
	render_settings.stop_label_offset = std::make_pair(proto_render_settings.stop_label_offset().first(),
		proto_render_settings.stop_label_offset().second());

	render_settings.underlayer_color = MakeColor(proto_render_settings.underlayer_color());
	render_settings.underlayer_width = proto_render_settings.underlayer_width();
	std::vector<svg::Color> color_palette;
	for (int i = 0; i < proto_render_settings.color_palette_size(); ++i) {
		color_palette.push_back(MakeColor(proto_render_settings.color_palette(i)));
	}
	render_settings.color_palette = std::move(color_palette);

	return render_settings;
}

transport_catalogue::Item MakeItem(const graph_serialize::Item& proto_item,
	const std::unordered_map<std::string_view, const Stop*>& stopname_to_stop,
	const std::map<std::string_view, const Bus*>& busname_to_bus) {
	transport_catalogue::Item item;
	item.time = proto_item.time();
	if (proto_item.type() == graph_serialize::ACTION_TYPE_BUS) {
		item.type = transport_catalogue::ActionType::BUS;
		item.name = busname_to_bus.at(proto_item.name())->name;
	}
	else if (proto_item.type() == graph_serialize::ACTION_TYPE_WAIT) {
		item.type = transport_catalogue::ActionType::WAIT;
		item.name = stopname_to_stop.at(proto_item.name())->name;
	}
	else {
		item.type = transport_catalogue::ActionType::ITEM;
	}
	item.span_count = proto_item.span_count();
	return item;
}

transport_router::TransportRouter DeserializeRouteSettings(const transport_router_serialize::TransportRouter& proto_tran_router,
	const std::unordered_map<std::string_view, const Stop*>& stopname_to_stop,
	const std::map<std::string_view, const Bus*>& busname_to_bus) {
	// RouteSettings
	auto& proto_route_settings = proto_tran_router.route_settings();
	transport_catalogue::RouteSettings route_settings{proto_route_settings.bus_velocity(), proto_route_settings.bus_wait_time()};
	// RouteSettings
	
	// Graph
	auto& proto_graph = proto_tran_router.graph();
	std::vector<graph::Edge<transport_catalogue::Item>> edges;
	for (int i = 0; i < proto_graph.edges_size(); ++i) {
		auto& proto_edge = proto_graph.edges(i);
		graph::Edge<transport_catalogue::Item> edge{proto_edge.from(), proto_edge.to(), MakeItem(proto_edge.weight(), stopname_to_stop, busname_to_bus)};
		edges.push_back(std::move(edge));
	}
	std::vector<std::vector<graph::EdgeId>> incidence_lists;
	for (int i = 0; i < proto_graph.incidence_lists_size(); ++i) {
		std::vector<graph::EdgeId> vector_edge_id;
		for (int j = 0; j < proto_graph.incidence_lists(i).edge_id_size(); ++j) {
			vector_edge_id.push_back(proto_graph.incidence_lists(i).edge_id(j));
		}
		incidence_lists.push_back(std::move(vector_edge_id));
	}
	auto graph = std::make_unique<graph::DirectedWeightedGraph<transport_catalogue::Item>>(std::move(edges), std::move(incidence_lists));
	// Graph
	
	// StopnamesToVertex
	std::map<std::string, transport_router::VertexId> valid_stopname_to_vertex;
	for (int i = 0; i < proto_tran_router.stopnames_to_vertex_size(); ++i) {
		valid_stopname_to_vertex[proto_tran_router.stopnames_to_vertex(i).stopname()] = proto_tran_router.stopnames_to_vertex(i).vertex();
	}
	// StopnamesToVertex

	// Router
	auto& proto_router = proto_tran_router.router();
	std::vector<std::vector<std::optional<graph::Router<transport_catalogue::Item>::RouteInternalData>>> routes_internal_data;
	for (int i = 0; i < proto_router.routesinternaldata_size(); ++i) {
		std::vector<std::optional<graph::Router<transport_catalogue::Item>::RouteInternalData>> vector_route_data;
		auto& proto_router_value = proto_router.routesinternaldata(i);
		for (int j = 0; j < proto_router_value.values_size(); ++j) {
			if (!proto_router_value.values(j).has_weight()) {
				vector_route_data.push_back(std::nullopt);
				continue;
			}
			std::optional<graph::EdgeId> prev_edge;
			if (proto_router_value.values(j).has_prev_edge()) {
				prev_edge = proto_router_value.values(j).prev_edge().value();
			}
			graph::Router<transport_catalogue::Item>::RouteInternalData route_data{ MakeItem(proto_router_value.values(j).weight(), stopname_to_stop, busname_to_bus), prev_edge };
			vector_route_data.push_back(std::move(route_data));
		}
		routes_internal_data.push_back(std::move(vector_route_data));
	}
	auto router = std::make_unique<graph::Router<transport_catalogue::Item>>(*graph, std::move(routes_internal_data));
	transport_router::TransportRouter transport_router{ std::move(route_settings), std::move(graph), std::move(router), std::move(valid_stopname_to_vertex) };
	return transport_router;
}

transport_catalogue_serialize::Facade* DeserializeFacade(std::string filename) {
	std::ifstream in_file(filename, std::ios::binary);
	auto proto_facade = new transport_catalogue_serialize::Facade;
	if (!proto_facade->ParseFromIstream(&in_file)) {
		return proto_facade;
	}
	return proto_facade;
}
} //serialization