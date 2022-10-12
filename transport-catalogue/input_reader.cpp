#include "geo.h"
#include "input_reader.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <stdexcept>
#include <utility>

namespace transport_catalogue {
using namespace std::literals;

namespace detail {
void CutSpaces(std::string& line) {
    auto first_char = line.find_first_not_of(' ');
    if (first_char == std::string::npos) {
        return;
    }
    line.erase(0, first_char);
    auto last_char = line.find_last_not_of(' ');
    line.resize(last_char + 1);
}

std::unordered_map<std::string, double> HandleLengthsToStops(std::string& str) {
    std::unordered_map<std::string, double> result;
    if (str.empty()) {
        return result;
    }
    std::stringstream ss(str);
    std::string in;
    while (std::getline(ss, in, ',')) {
        std::stringstream in_ss(in);
        double length;
        std::string stopname;
        in_ss >> length;
        in_ss >> stopname;//Чтобы скушать букву "m"
        in_ss >> stopname;//Чтобы скушать разделитель-предлог "to"
        std::getline(in_ss, stopname);
        CutSpaces(stopname);
        result[std::move(stopname)] = length;
    }
    return result;
}

Stop HandleStop(std::string& str) {
    str.erase(0, 5);
    size_t colon_pos = str.find_first_of(':');
    std::string stopname(str.substr(0, colon_pos));
    str.erase(0, colon_pos + 1);
    size_t comma_pos = str.find_first_of(',');
    double lat = std::stod(std::string(str.substr(0, comma_pos)));
    str.erase(0, comma_pos + 1);
    comma_pos = std::min(str.find_first_of(','), str.size());
    double lng = std::stod(std::string(str.substr(0, comma_pos)));
    str.erase(0, comma_pos + 1);
    return { std::move(stopname), {lat, lng} };
}

Bus HandleBus(std::string&& str, TransportCatalogue& tran_cat) {
    TypeRoute type_route = TypeRoute::circle;
    char delimiter = '>';
    std::unordered_set<std::string_view> unique_stops;

    str.erase(0, 4);
    size_t colon_pos = str.find_first_of(':');
    std::string busname(str.substr(0, colon_pos));
    str.erase(0, colon_pos + 1);
    if (str.find_first_of('>') == std::string::npos) {
        type_route = TypeRoute::line;
        delimiter = '-';
    }
    std::stringstream ss_str(str);
    std::string stopname;
    std::vector<const Stop*> stops;
    while (std::getline(ss_str, stopname, delimiter)) {
        CutSpaces(stopname);
        stops.push_back(tran_cat.FindStop(stopname));
        unique_stops.insert(stops.back()->name_);
    }
    return { std::move(busname), std::move(stops), std::move(type_route), std::move(unique_stops) };
}
}

namespace reader_ifo {
void ReadInput(TransportCatalogue& tran_cat, std::istream& thread) {
    std::string in;
    getline(thread, in);
    int count = std::stoi(in);
    std::vector<std::string> bus_info_strings;
    bus_info_strings.reserve(count);
    std::unordered_map < const Stop*, std::unordered_map<std::string, double>> stop_to_lengths_to_stops;

    for (int i = 0; i < count; ++i) {
        getline(thread, in);
        auto key_word = in.substr(0, in.find_first_of(' '));
        if (key_word == "Stop"sv) {
            Stop stop = detail::HandleStop(in);
            std::string name = stop.name_;
            tran_cat.AddStop(std::move(stop));
            stop_to_lengths_to_stops[tran_cat.FindStop(name)] = detail::HandleLengthsToStops(in);
        }
        else if (key_word == "Bus"sv) {
            bus_info_strings.push_back(std::move(in));
        }
        else {
            throw std::invalid_argument("Input contains not correct command!");
        }
    }

    for (const auto& [stop, stop_length] : stop_to_lengths_to_stops) {
        tran_cat.SetLengthInStops(stop, stop_length);
    }

    for (auto& str : bus_info_strings) {
        tran_cat.AddBus(detail::HandleBus(std::move(str), tran_cat));
    }
}
}
}