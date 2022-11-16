#include "json_reader.h"
#include "json_builder.h"

#include <iostream>
#include <sstream>
#include <utility>
#include <sstream>

namespace transport_catalogue {
namespace handle_iformation {
using namespace std::literals;
using namespace svg;
using namespace json;
using namespace graph;
using namespace transport_router;

Color ParseColor(const Node& color_node) {
	Color color;
	if (color_node.IsArray()) {
		const auto& color_node_as_array = color_node.AsArray();
		if (color_node_as_array.size() == 3) {
			color = Rgb(color_node_as_array.at(0).AsInt(),
				color_node_as_array.at(1).AsInt(),
				color_node_as_array.at(2).AsInt());
		}
		else if (color_node_as_array.size() == 4) {
			color = Rgba(color_node_as_array.at(0).AsInt(),
				color_node_as_array.at(1).AsInt(),
				color_node_as_array.at(2).AsInt(),
				color_node_as_array.at(3).AsDouble());
		}
		else {
			throw "Something is wrong with render settings initialisation!"s;
		}
	}
	else {
		color = color_node.AsString();
	}
	return color;
}

void Facade::InitialiseBaseRequests() {
	const auto& base_requests = document_.GetRoot().AsDict().at("base_requests"s).AsArray();
	std::vector<const Node*> bus_requests;
	bus_requests.reserve(base_requests.size());
	std::unordered_map < const Stop*, Dict> stop_to_lengths_to_stops;
	for (const auto& request : base_requests) {
		const auto& request_as_map = request.AsDict();
		if (request_as_map.at("type"s).AsString() == "Stop"s) {
			const auto& name = request_as_map.at("name"s).AsString();
			const auto lat = request_as_map.at("latitude"s).AsDouble();
			const auto lng = request_as_map.at("longitude"s).AsDouble();
			tran_cat_.AddStop({ std::move(name), {lat, lng} });
			stop_to_lengths_to_stops[tran_cat_.FindStop(name)] = request_as_map.at("road_distances"s).AsDict();
		}
		else if (request_as_map.at("type"s).AsString() == "Bus"s) {
			bus_requests.push_back(&request);
		}
		else {
			throw std::invalid_argument("Input contains not correct command!"s);
		}
	}

	for (const auto& [stop, stop_length] : stop_to_lengths_to_stops) {
		tran_cat_.SetLengthInStops(stop, stop_length);
	}

	for (const auto& request : bus_requests) {
		const auto& request_as_map = request->AsDict();
		auto& busname = request_as_map.at("name"s).AsString();
		std::vector<const Stop*> stops;
		std::unordered_set<std::string_view> unique_stops;
		for (const auto& stopname : request_as_map.at("stops"s).AsArray()) {
			stops.push_back(tran_cat_.FindStop(stopname.AsString()));
			unique_stops.insert(stops.back()->name);
		}
		auto type_route = request_as_map.at("is_roundtrip"s).AsBool() ? TypeRoute::circle : TypeRoute::line;
		tran_cat_.AddBus({ std::move(busname), std::move(stops), std::move(type_route), std::move(unique_stops) });
	}

	tran_cat_.BuildValidStopsVertex();
}

void Facade::InitialiseRenderSettings() {
	const auto& render_settings = document_.GetRoot().AsDict().at("render_settings"s).AsDict();
	const auto& bus_offset = render_settings.at("bus_label_offset"s).AsArray();
	const auto bus_label_offset = std::make_pair(bus_offset[0].AsDouble(), bus_offset[1].AsDouble());

	const auto& stop_offset = render_settings.at("stop_label_offset"s).AsArray();
	const auto stop_label_offset = std::make_pair(stop_offset[0].AsDouble(), stop_offset[1].AsDouble());

	Color underlayer_color = ParseColor(render_settings.at("underlayer_color"s));
	std::vector<Color> color_palette;
	color_palette.reserve(render_settings.at("color_palette"s).AsArray().size());
	for (const auto& color : render_settings.at("color_palette"s).AsArray()) {
		color_palette.push_back(ParseColor(color));
	}

	rendering::RenderSettings settings = { render_settings.at("width"s).AsDouble(), render_settings.at("height"s).AsDouble(),
		render_settings.at("padding"s).AsDouble(), render_settings.at("line_width"s).AsDouble(),
		render_settings.at("stop_radius"s).AsDouble(), render_settings.at("bus_label_font_size"s).AsInt(),
		bus_label_offset, render_settings.at("stop_label_font_size"s).AsInt(),
		stop_label_offset, underlayer_color,
		render_settings.at("underlayer_width"s).AsDouble(), color_palette };
	map_render_ = rendering::MapRenderer(std::move(settings));
}

void Facade::InitializeTransportRouter() {
	const auto& routing_settings_doc = document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
	auto bus_velocity = routing_settings_doc.at("bus_velocity"s).AsDouble();
	auto bus_wait_time = routing_settings_doc.at("bus_wait_time"s).AsDouble();
	RouteSettings route_settings = { bus_velocity, bus_wait_time };
	transport_router_ = transport_router::TransportRouter(tran_cat_, std::move(route_settings));
}

Facade::Facade(std::istream& thread)  
	: document_(Load(thread)) 
{
	InitialiseBaseRequests();
	InitialiseRenderSettings();
	InitializeTransportRouter();
}

json::Node Facade::HandleBusRequest(const json::Dict& request_as_map) const {
	auto bus_info = tran_cat_.GetInfromBus(request_as_map.at("name"s).AsString());
	Node node;
	if (!bus_info) {
		return Builder{}.StartDict().Key("request_id"s).Value(request_as_map.at("id"s).AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build();
	}
	else {
		return Builder{}.StartDict().Key("request_id"s).Value(request_as_map.at("id"s).AsInt())
			.Key("curvature"s).Value(bus_info->curvature)
			.Key("route_length"s).Value(bus_info->length)
			.Key("stop_count"s).Value(static_cast<int>(bus_info->amount_stops))
			.Key("unique_stop_count"s).Value(static_cast<int>(bus_info->amount_unique_stops))
			.EndDict().Build();
	}
}

json::Node Facade::HandleStopRequest(const json::Dict& request_as_map) const {
	Node node;
	try {
		auto busses = tran_cat_.GetListBusses(request_as_map.at("name"s).AsString());
		Array names;
		names.reserve(busses.size());
		for (const auto& sv : busses) {
			names.push_back(std::string(sv));
		}
		return Builder{}.StartDict().Key("request_id"s).Value(request_as_map.at("id"s).AsInt()).Key("buses"s).Value(std::move(names)).EndDict().Build();
	}
	catch (std::string error) {
		return Builder{}.StartDict().Key("request_id"s).Value(request_as_map.at("id"s).AsInt()).Key("error_message"s).Value(error).EndDict().Build();
	}
}

json::Node Facade::HandleMapRequest(const json::Dict& request_as_map) {
	std::ostringstream render;
	RenderRoute(render);
	return Builder{}.StartDict().Key("request_id"s).Value(request_as_map.at("id"s).AsInt()).Key("map"s).Value(render.str()).EndDict().Build();
}

void Facade::AsnwerRequests(std::ostream& thread) {
	const auto& stat_requests = document_.GetRoot().AsDict().at("stat_requests"s).AsArray();
	Builder builder = Builder{};
	auto result = builder.StartArray();
	for (const auto& request : stat_requests) {
		const auto& request_as_map = request.AsDict();
		Node node;
		if (request_as_map.at("type"s).AsString() == "Bus"s) {
			node = HandleBusRequest(request_as_map);
		}
		else if (request_as_map.at("type"s).AsString() == "Stop"s) {
			node = HandleStopRequest(request_as_map);
		}
		else if (request_as_map.at("type"s).AsString() == "Map"s) {
			node = HandleMapRequest(request_as_map);
		}
		else if (request_as_map.at("type"s).AsString() == "Route"s) {
			const auto& valid_stopname_to_vertex = tran_cat_.GetValidStopsVertex();
			node = transport_router_.FindRoute(request_as_map, valid_stopname_to_vertex);
		}
		else {
			throw std::invalid_argument("Input contains not correct request!"s);
		}
		result.Value(std::move(node.GetValue()));
	}
	json::Print(json::Document(result.EndArray().Build()), thread);
}

void Facade::RenderRoute(std::ostream& thread) {
	map_render_.Render(tran_cat_);
	map_render_.VisualiseRender(thread);
}
} //namespace handle_iformation
} //namespace transport_catalogue