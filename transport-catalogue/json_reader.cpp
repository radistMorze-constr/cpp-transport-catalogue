#include "json_reader.h"

#include <iostream>
#include <sstream>
#include <utility>
#include <sstream>

namespace transport_catalogue {
namespace handle_iformation {
using namespace std::literals;
using namespace svg;

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
	const auto& base_requests = document_.GetRoot().AsMap().at("base_requests"s).AsArray();
	std::vector<const Node*> bus_requests;
	bus_requests.reserve(base_requests.size());
	std::unordered_map < const Stop*, Dict> stop_to_lengths_to_stops;
	for (const auto& request : base_requests) {
		const auto& request_as_map = request.AsMap();
		if (request_as_map.at("type"s).AsString() == "Stop"s) {
			const auto& name = request_as_map.at("name"s).AsString();
			const auto lat = request_as_map.at("latitude"s).AsDouble();
			const auto lng = request_as_map.at("longitude"s).AsDouble();
			tran_cat_.AddStop({ std::move(name), {lat, lng} });
			stop_to_lengths_to_stops[tran_cat_.FindStop(name)] = request_as_map.at("road_distances"s).AsMap();
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
		const auto& request_as_map = request->AsMap();
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
}

void Facade::InitialiseRenderSettings() {
	const auto& render_settings = document_.GetRoot().AsMap().at("render_settings"s).AsMap();
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

	map_render_ = rendering::MapRenderer(render_settings.at("width"s).AsDouble(), render_settings.at("height"s).AsDouble(),
		render_settings.at("padding"s).AsDouble(), render_settings.at("line_width"s).AsDouble(),
		render_settings.at("stop_radius"s).AsDouble(), render_settings.at("bus_label_font_size"s).AsInt(),
		bus_label_offset, render_settings.at("stop_label_font_size"s).AsInt(),
		stop_label_offset, underlayer_color,
		render_settings.at("underlayer_width"s).AsDouble(), color_palette);
}

Facade::Facade(std::istream& thread) 
	: document_(Load(thread)) 
{
	InitialiseBaseRequests();
	InitialiseRenderSettings();
}

void Facade::AsnwerRequests(std::ostream& thread) {
	const auto& stat_requests = document_.GetRoot().AsMap().at("stat_requests"s).AsArray();
	Array result;
	result.reserve(stat_requests.size());
	for (const auto& request : stat_requests) {
		const auto& request_as_map = request.AsMap();
		if (request_as_map.at("type"s).AsString() == "Bus"s) {
			auto bus_info = tran_cat_.GetInfromBus(request_as_map.at("name"s).AsString());
			if (!bus_info) {
				Node node({ { "request_id"s, request_as_map.at("id"s).AsInt() }, { "error_message"s, "not found"s } });
				result.push_back(std::move(node));
			}
			else {
				Node node({ { "request_id"s, request_as_map.at("id"s).AsInt() },
					{ "curvature"s, bus_info->curvature },
					{ "route_length"s, bus_info->length },
					{ "stop_count"s, static_cast<int>(bus_info->amount_stops) },
					{ "unique_stop_count"s, static_cast<int>(bus_info->amount_unique_stops) } });
				result.push_back(std::move(node));
			}
		}
		else if (request_as_map.at("type"s).AsString() == "Stop"s) {
			try {
				auto busses = tran_cat_.GetListBusses(request_as_map.at("name"s).AsString());
				Array names;
				names.reserve(busses.size());
				for (const auto& sv : busses) {
					names.push_back(std::string(sv));
				}
				Node node({ { "request_id"s, request_as_map.at("id"s).AsInt() }, { "buses"s, std::move(names) } });
				result.push_back(std::move(node));
			}
			catch (std::string error) {
				Node node({ { "request_id"s, request_as_map.at("id"s).AsInt() }, { "error_message"s, error } });
				result.push_back(std::move(node));
			}
		}
		else if (request_as_map.at("type"s).AsString() == "Map"s) {
			std::ostringstream render;
			RenderRoute(render);
			Node node({ { "request_id"s, request_as_map.at("id"s).AsInt() }, { "map"s, render.str()}});
			result.push_back(std::move(node));
		}
		else {
			throw std::invalid_argument("Input contains not correct request!"s);
		}
	}
	Print(json::Document(result), thread);
}

void Facade::RenderRoute(std::ostream& thread) {
	map_render_.Render(tran_cat_);
	map_render_.VisualiseRender(thread);
}
} //namespace handle_iformation
} //namespace transport_catalogue