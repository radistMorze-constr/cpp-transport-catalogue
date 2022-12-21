#pragma once
#include "svg.h"
#include "transport_catalogue.h"

#include <utility>
#include <vector>

namespace transport_catalogue {
namespace rendering {

struct RenderSettings {
	double width;
	double height;
	double padding;
	double line_width;
	double stop_radius;
	int bus_label_font_size;
	std::pair<double, double> bus_label_offset;
	int stop_label_font_size;
	std::pair<double, double> stop_label_offset;
	svg::Color underlayer_color;
	double underlayer_width;
	std::vector<svg::Color> color_palette;
};
class MapRenderer {
public:
	explicit MapRenderer();
	explicit MapRenderer(RenderSettings&& render_settings);
	void Render(const TransportCatalogue& tran_cat);
	void VisualiseRender(std::ostream& thread) const;

	// for serialization
	const RenderSettings& GetRenderSettings() const;

protected:
	void FillRenderPolylines(const std::unordered_map<const Stop*, svg::Point>& stop_to_coordinates, std::vector<std::pair<svg::Text, svg::Color>>& busnames_to_draw,
		const std::map<std::string_view, const Bus*>& busname_to_bus);
	svg::Text MakeRouteName(const svg::Point& point, const std::string& name);

private:
	RenderSettings render_settings_;

	svg::Document render_doc_;
};
} //namespace rendering
} //namespace transport_catalogue