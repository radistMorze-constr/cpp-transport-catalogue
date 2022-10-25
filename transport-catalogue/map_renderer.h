#pragma once
#include "svg.h"
#include "transport_catalogue.h"

#include <utility>
#include <vector>

namespace transport_catalogue {
namespace rendering {
using namespace svg;

class MapRenderer {
public:
	explicit MapRenderer();
	explicit MapRenderer(double width, double height, double padding, double line_width, double stop_radius, size_t bus_label_font_size,
		std::pair<double, double> bus_label_offset, size_t stop_label_font_size, std::pair<double, double> stop_label_offset,
		Color underlayer_color, double underlayer_width, std::vector<Color> color_palette);
	void Render(const TransportCatalogue& tran_cat);
	void VisualiseRender(std::ostream& thread) const;

protected:
	void FillRenderPolylines(const std::unordered_map<const Stop*, Point>& stop_to_coordinates, std::vector<std::pair<Text, Color>>& busnames_to_draw,
		const std::map<std::string_view, const Bus*>& busname_to_bus);
	Text MakeRouteName(const Point& point, const std::string& name);

private:
	double width_;
	double height_;
	double padding_;
	double line_width_;
	double stop_radius_;
	size_t bus_label_font_size_;
	std::pair<double, double> bus_label_offset_;
	size_t stop_label_font_size_;
	std::pair<double, double> stop_label_offset_;
	Color underlayer_color_;
	double underlayer_width_;
	std::vector<Color> color_palette_;

	svg::Document render_doc_;
};
} //namespace rendering
} //namespace transport_catalogue