#include "map_renderer.h"
namespace renderer {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    std::vector<svg::Polyline> MapRenderer::RenderRouteLines(const std::map<std::string_view, const transport::Bus*>& buses, const SphereProjector& sp) const {
        std::vector<svg::Polyline> result;
        size_t color_num = 0;
        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops_.empty()) continue;
            std::vector<const transport::Stop*> route_stops{ bus->stops_.begin(), bus->stops_.end() };
            if (bus->is_circle_ == false) route_stops.insert(route_stops.end(), std::next(bus->stops_.rbegin()), bus->stops_.rend());
            svg::Polyline line;
            for (const auto& stop : route_stops) {
                line.AddPoint(sp(stop->coordinates_));
            }
            line.SetStrokeColor(render_settings_.color_palette[color_num]);
            line.SetFillColor("none");
            line.SetStrokeWidth(render_settings_.line_width);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            if (color_num < (render_settings_.color_palette.size() - 1)) ++color_num;
            else color_num = 0;
            result.push_back(line);
        }
        return result;
    }

    std::vector<svg::Text> MapRenderer::RenderBusLabel(const std::map<std::string_view, const transport::Bus*>& buses, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        size_t color_num = 0;
        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops_.empty()) continue;
            svg::Text text;
            svg::Text underlayer;
            text.SetPosition(sp(bus->stops_[0]->coordinates_));
            text.SetOffset(render_settings_.bus_label_offset);
            text.SetFontSize(render_settings_.bus_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetFontWeight("bold");
            text.SetData(bus->name_);
            text.SetFillColor(render_settings_.color_palette[color_num]);
            if (color_num < (render_settings_.color_palette.size() - 1)) ++color_num;
            else color_num = 0;

            underlayer.SetPosition(sp(bus->stops_[0]->coordinates_));
            underlayer.SetOffset(render_settings_.bus_label_offset);
            underlayer.SetFontSize(render_settings_.bus_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetFontWeight("bold");
            underlayer.SetData(bus->name_);
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);

            if (bus->is_circle_ == false && bus->stops_[0] != bus->stops_[bus->stops_.size() - 1]) {
                svg::Text text2{ text };
                svg::Text underlayer2{ underlayer };
                text2.SetPosition(sp(bus->stops_[bus->stops_.size() - 1]->coordinates_));
                underlayer2.SetPosition(sp(bus->stops_[bus->stops_.size() - 1]->coordinates_));
                result.push_back(underlayer2);
                result.push_back(text2);
            }
        }
        return result;
    }

    std::vector<svg::Circle> MapRenderer::RenderStopsSymbols(const std::map<std::string_view, const transport::Stop*>& stops_, const SphereProjector& sp) const {
        std::vector<svg::Circle> result;
        for (const auto& [stop_name, stop] : stops_) {
            svg::Circle symbol;
            symbol.SetCenter(sp(stop->coordinates_));
            symbol.SetRadius(render_settings_.stop_radius);
            symbol.SetFillColor("white");
            result.push_back(symbol);
        }
        return result;
    }

    std::vector<svg::Text> MapRenderer::RenderStopsLabels(const std::map<std::string_view, const transport::Stop*>& stops_, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        svg::Text text;
        svg::Text underlayer;
        for (const auto& [stop_name, stop] : stops_) {
            text.SetPosition(sp(stop->coordinates_));
            text.SetOffset(render_settings_.stop_label_offset);
            text.SetFontSize(render_settings_.stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name_);
            text.SetFillColor("black");

            underlayer.SetPosition(sp(stop->coordinates_));
            underlayer.SetOffset(render_settings_.stop_label_offset);
            underlayer.SetFontSize(render_settings_.stop_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetData(stop->name_);
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);
        }
        return result;
    }

    svg::Document MapRenderer::RenderSVG(const std::map<std::string_view, const transport::Bus*>& buses) const {
        svg::Document result;
        std::vector<geo::Coordinates> route_stops_coord;
        std::map<std::string_view, const transport::Stop*> all_stops;

        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops_.empty()) continue;
            for (const auto& stop : bus->stops_) {
                route_stops_coord.push_back(stop->coordinates_);
                all_stops[stop->name_] = stop;
            }
        }
        SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
        result = RenderAll(buses, sp, all_stops);
        return result;
    }
    svg::Document MapRenderer::RenderAll(const std::map<std::string_view, const transport::Bus*>& buses, SphereProjector& sp, std::map<std::string_view, const transport::Stop*> all_stops) const
    {
        svg::Document result;

        for (const auto& line : RenderRouteLines(buses, sp)) {
            result.Add(line);
        }
        for (const auto& text : RenderBusLabel(buses, sp)) {
            result.Add(text);
        }
        for (const auto& circle : RenderStopsSymbols(all_stops, sp)) {
            result.Add(circle);
        }
        for (const auto& text : RenderStopsLabels(all_stops, sp)) {
            result.Add(text);
        }
        return result;
    }
}
