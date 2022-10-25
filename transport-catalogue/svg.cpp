#include "svg.h"

namespace svg {
 using namespace std::literals;
std::ostream& operator<<(std::ostream& out, const StrokeLineCap line_cap) {
    switch (line_cap) {
    case StrokeLineCap::BUTT: return out << "butt"sv;
    case StrokeLineCap::ROUND: return out << "round"sv;
    case StrokeLineCap::SQUARE: return out << "square"sv;
    default: return out << (int)line_cap;
    }
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join) {
    switch (line_join) {
    case StrokeLineJoin::ARCS: return out << "arcs"sv;
    case StrokeLineJoin::BEVEL: return out << "bevel"sv;
    case StrokeLineJoin::MITER: return out << "miter"sv;
    case StrokeLineJoin::MITER_CLIP: return out << "miter-clip"sv;
    case StrokeLineJoin::ROUND: return out << "round"sv;
    default: return out << (int)line_join;
    }
}

Rgb::Rgb() = default;

Rgb::Rgb(uint8_t red_in, uint8_t green_in, uint8_t blue_in)
    :red(red_in),
    green(green_in),
    blue(blue_in)
{
}

Rgba::Rgba() = default;

Rgba::Rgba(uint8_t red_in, uint8_t green_in, uint8_t blue_in, double opacity_in)
    :red(red_in),
    green(green_in),
    blue(blue_in),
    opacity(opacity_in)
{
}

void ColorPrinter::operator()(std::monostate) const { out << "none"sv; }

void ColorPrinter::operator()(std::string color) const { out << color; }

void ColorPrinter::operator()(Rgb color) const {
    out << "rgb("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ")"sv;
}

void ColorPrinter::operator()(Rgba color) const {
    out << "rgba("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ","sv << color.opacity << ")"sv;
}

const std::unordered_map<char, std::string> DICTIONARY{ {'"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"},
    {'>', "&gt;"}, {'&', "&amp;"} };

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

std::string Polyline::GetPointsLine() const {
    std::stringstream out;
    bool is_first = true;
    for (const auto& point : points_) {
        if (!is_first) {
            out << " "s;
        }
        is_first = false;
        out << point.x << ","s << point.y;
    }
    return out.str();
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv << GetPointsLine() << "\" ";
    RenderAttrs(context.out);
    out << "/>";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out <<" x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y;
    out << "\" font-size=\"" << size_;
    if (!font_family_.empty()) {
        out << "\" font-family=\"" << font_family_;
    }
    if (!font_weight_.empty()) {
        out << "\" font-weight=\"" << font_weight_;
    }
    out << "\">"sv;
    for (const char i : data_) {
        if (DICTIONARY.count(i)) {
            out << DICTIONARY.at(i);
        }
        else {
            out << i;
        }
    }
    out  << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg