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
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs->coordinates.lng < rhs->coordinates.lng; });
        min_lon_ = (*left_it)->coordinates.lng;
        const double max_lon = (*right_it)->coordinates.lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs->coordinates.lat < rhs->coordinates.lat; });
        const double min_lat = (*bottom_it)->coordinates.lat;
        max_lat_ = (*top_it)->coordinates.lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding
        };
    }

private:
    double padding;
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

/*MapRenderer::MapRenderer(double width, double height, double padding, double line_width, double stop_radius, size_t bus_label_font_size,
	std::pair<double, double> bus_label_offset, size_t stop_label_font_size, std::pair<double, double> stop_label_offset,
	Color underlayer_color, double underlayer_width, std::vector<Color> color_palette) 
	: width(width),
	height(height),
	padding(padding),
	line_width(line_width),
	stop_radius(stop_radius),
	bus_label_font_size(bus_label_font_size),
	bus_label_offset(bus_label_offset),
	stop_label_font_size(stop_label_font_size),
	stop_label_offset(stop_label_offset),
	underlayer_color(underlayer_color),
	underlayer_width(underlayer_width),
	color_palette(color_palette)
{
}*/

MapRenderer::MapRenderer(RenderSettings&& render_settings)
	: render_settings_(std::move(render_settings))
{
}

Text MapRenderer::MakeRouteName(const Point& point, const std::string& name) {
    using namespace std::literals;
    const Text busname = Text().SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s)
        .SetFontSize(render_settings_.bus_label_font_size)
        .SetPosition(point)
        .SetOffset({ render_settings_.bus_label_offset.first, render_settings_.bus_label_offset.second })
        .SetData(name);
    return busname;
}

void MapRenderer::FillRenderPolylines(const std::unordered_map<const Stop*, Point>& stop_to_coordinates, std::vector<std::pair<Text, Color>>& busnames_to_draw,
    const std::map<std::string_view, const Bus*>& busname_to_bus) {
    using namespace std::literals;
    size_t count = 0;
    for (const auto& [busname, bus_ptr] : busname_to_bus) {
        if (bus_ptr->stops.size()) {
            size_t index_color = count % render_settings_.color_palette.size();
            render_doc_.Add(CreateRoute(bus_ptr->stops, stop_to_coordinates, bus_ptr->type_route).
                SetStrokeWidth(render_settings_.line_width).SetStrokeLineCap(StrokeLineCap::ROUND).
                SetStrokeLineJoin(StrokeLineJoin::ROUND).SetStrokeColor(render_settings_.color_palette[index_color]).
                SetFillColor({}));

            const Text busname_begin = MakeRouteName(stop_to_coordinates.at(bus_ptr->stops.front()), bus_ptr->name);
            busnames_to_draw.push_back(std::make_pair(busname_begin, render_settings_.color_palette[index_color]));

            if (bus_ptr->type_route == TypeRoute::line && bus_ptr->stops.front() != bus_ptr->stops.back()) {
                const Text busname_end = MakeRouteName(stop_to_coordinates.at(bus_ptr->stops.back()), bus_ptr->name);
                busnames_to_draw.push_back(std::make_pair(busname_end, render_settings_.color_palette[index_color]));
            }
            ++count;
        }
    }
}

void MapRenderer::Render(const TransportCatalogue& tran_cat) {
    using namespace std::literals;
    
    const auto stops = tran_cat.GetValidStops();
    SphereProjector projector(stops.begin(), stops.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    std::unordered_map<const Stop*, Point> stop_to_coordinates;
    for (const auto& stop : stops) {
        stop_to_coordinates[stop] = projector(stop->coordinates);
    }
    std::vector<std::pair<Text, Color>> busnames_to_draw;
    busnames_to_draw.reserve(stops.size());

    FillRenderPolylines(stop_to_coordinates, busnames_to_draw, tran_cat.GetBusnameToBus());

    for (const auto& [busname, color] : busnames_to_draw) {
        render_doc_.Add(Text{ busname }
            .SetStrokeColor(render_settings_.underlayer_color)
            .SetFillColor(render_settings_.underlayer_color)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeWidth(render_settings_.underlayer_width));
        render_doc_.Add(Text{ busname }.SetFillColor(color));
    }

    for (const auto& stop : stops) {
        const auto& center = stop_to_coordinates[stop];
        render_doc_.Add(Circle().SetCenter(center).SetRadius(render_settings_.stop_radius).SetFillColor("white"s));
    }

    for (const auto& stop : stops) {
        const auto& center = stop_to_coordinates[stop];
        const Text stopname = Text().SetFontFamily("Verdana"s)
            .SetFontSize(render_settings_.stop_label_font_size)
            .SetPosition(center)
            .SetOffset({ render_settings_.stop_label_offset.first, render_settings_.stop_label_offset.second })
            .SetData(stop->name);
        render_doc_.Add(Text{ stopname }
            .SetStrokeColor(render_settings_.underlayer_color)
            .SetFillColor(render_settings_.underlayer_color)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeWidth(render_settings_.underlayer_width));
        render_doc_.Add(Text{ stopname }.SetFillColor("black"s));
    }
}

void MapRenderer::VisualiseRender(std::ostream& thread) const {
    render_doc_.Render(thread);
}
} //namespace rendering
} //namespace transport_catalogue