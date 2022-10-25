#include "map_renderer.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <iterator>

namespace transport_catalogue {
namespace rendering {
using namespace svg;

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // ���� ����� ����������� ����� �� ������, ��������� ������
        if (points_begin == points_end) {
            return;
        }

        // ������� ����� � ����������� � ������������ ��������
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs->coordinates.lng < rhs->coordinates.lng; });
        min_lon_ = (*left_it)->coordinates.lng;
        const double max_lon = (*right_it)->coordinates.lng;

        // ������� ����� � ����������� � ������������ �������
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs->coordinates.lat < rhs->coordinates.lat; });
        const double min_lat = (*bottom_it)->coordinates.lat;
        max_lat_ = (*top_it)->coordinates.lat;

        // ��������� ����������� ��������������� ����� ���������� x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // ��������� ����������� ��������������� ����� ���������� y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // ������������ ��������������� �� ������ � ������ ���������,
            // ���� ����������� �� ���
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *height_zoom;
        }
    }

    // ���������� ������ � ������� � ���������� ������ SVG-�����������
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

Polyline CreateRoute(const std::vector<const Stop*>& stops, const std::unordered_map<const Stop*, Point>& stop_to_coordinates, const TypeRoute type_route) {
	Polyline polyline;
    for (const auto& stop : stops) {
        polyline.AddPoint(stop_to_coordinates.at(stop));
    }
    if (type_route == TypeRoute::line) {
        for (auto it = std::next(stops.rbegin()); it != stops.rend(); ++it) {
            polyline.AddPoint(stop_to_coordinates.at((*it)));
        }
    }
    return polyline;
}

MapRenderer::MapRenderer() = default;

MapRenderer::MapRenderer(double width, double height, double padding, double line_width, double stop_radius, size_t bus_label_font_size,
	std::pair<double, double> bus_label_offset, size_t stop_label_font_size, std::pair<double, double> stop_label_offset,
	Color underlayer_color, double underlayer_width, std::vector<Color> color_palette) 
	: width_(width),
	height_(height),
	padding_(padding),
	line_width_(line_width),
	stop_radius_(stop_radius),
	bus_label_font_size_(bus_label_font_size),
	bus_label_offset_(bus_label_offset),
	stop_label_font_size_(stop_label_font_size),
	stop_label_offset_(stop_label_offset),
	underlayer_color_(underlayer_color),
	underlayer_width_(underlayer_width),
	color_palette_(color_palette)
{
}

Text MapRenderer::MakeRouteName(const Point& point, const std::string& name) {
    using namespace std::literals;
    const Text busname = Text().SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s)
        .SetFontSize(bus_label_font_size_)
        .SetPosition(point)
        .SetOffset({ bus_label_offset_.first, bus_label_offset_.second })
        .SetData(name);
    return busname;
}

void MapRenderer::FillRenderPolylines(const std::unordered_map<const Stop*, Point>& stop_to_coordinates, std::vector<std::pair<Text, Color>>& busnames_to_draw,
    const std::map<std::string_view, const Bus*>& busname_to_bus) {
    using namespace std::literals;
    size_t count = 0;
    for (const auto& [busname, bus_ptr] : busname_to_bus) {
        if (bus_ptr->stops.size()) {
            size_t index_color = count % color_palette_.size();
            render_doc_.Add(CreateRoute(bus_ptr->stops, stop_to_coordinates, bus_ptr->type_route).
                SetStrokeWidth(line_width_).SetStrokeLineCap(StrokeLineCap::ROUND).
                SetStrokeLineJoin(StrokeLineJoin::ROUND).SetStrokeColor(color_palette_[index_color]).
                SetFillColor({}));

            const Text busname_begin = MakeRouteName(stop_to_coordinates.at(bus_ptr->stops.front()), bus_ptr->name);
            busnames_to_draw.push_back(std::make_pair(busname_begin, color_palette_[index_color]));

            if (bus_ptr->type_route == TypeRoute::line && bus_ptr->stops.front() != bus_ptr->stops.back()) {
                const Text busname_end = MakeRouteName(stop_to_coordinates.at(bus_ptr->stops.back()), bus_ptr->name);
                busnames_to_draw.push_back(std::make_pair(busname_end, color_palette_[index_color]));
            }
            ++count;
        }
    }
}

void MapRenderer::Render(const TransportCatalogue& tran_cat) {
    using namespace std::literals;
    
    const auto stops = tran_cat.GetValidStops();
    SphereProjector projector(stops.begin(), stops.end(), width_, height_, padding_);
    std::unordered_map<const Stop*, Point> stop_to_coordinates;
    for (const auto& stop : stops) {
        stop_to_coordinates[stop] = projector(stop->coordinates);
    }
    std::vector<std::pair<Text, Color>> busnames_to_draw;
    busnames_to_draw.reserve(stops.size());

    FillRenderPolylines(stop_to_coordinates, busnames_to_draw, tran_cat.GetBusnameToBus());

    for (const auto& [busname, color] : busnames_to_draw) {
        render_doc_.Add(Text{ busname }
            .SetStrokeColor(underlayer_color_)
            .SetFillColor(underlayer_color_)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeWidth(underlayer_width_));
        render_doc_.Add(Text{ busname }.SetFillColor(color));
    }

    for (const auto& stop : stops) {
        const auto& center = stop_to_coordinates[stop];
        render_doc_.Add(Circle().SetCenter(center).SetRadius(stop_radius_).SetFillColor("white"s));
    }

    for (const auto& stop : stops) {
        const auto& center = stop_to_coordinates[stop];
        const Text stopname = Text().SetFontFamily("Verdana"s)
            .SetFontSize(stop_label_font_size_)
            .SetPosition(center)
            .SetOffset({ stop_label_offset_.first, stop_label_offset_.second })
            .SetData(stop->name);
        render_doc_.Add(Text{ stopname }
            .SetStrokeColor(underlayer_color_)
            .SetFillColor(underlayer_color_)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeWidth(underlayer_width_));
        render_doc_.Add(Text{ stopname }.SetFillColor("black"s));
    }
}

void MapRenderer::VisualiseRender(std::ostream& thread) const {
    render_doc_.Render(thread);
}
} //namespace rendering
} //namespace transport_catalogue