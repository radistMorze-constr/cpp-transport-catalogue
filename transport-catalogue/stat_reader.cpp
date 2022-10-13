#include "geo.h"
#include "stat_reader.h"

namespace transport_catalogue {
using namespace std::literals;
namespace stat_reader {
void OutputStopInfo(Request stop, TransportCatalogue& tran_cat, std::ostream& thread) {
    thread << "Stop "s << stop.request << ": "s;
    try {
        auto busses = tran_cat.GetListBusses(stop.request);
        thread << "buses";
        for (const auto& bus : busses) {
            thread << " "s << bus;
        }
    }
    catch (std::string error) {
        thread << error;
    }
    thread << std::endl;
}

void OutputBusInfo(Request bus, TransportCatalogue& tran_cat, std::ostream& thread) {
    auto bus_info = tran_cat.GetInfromBus(bus.request);
    if (bus_info.not_found) {
        thread << "Bus "s << bus.request << ": not found"s << std::endl;
        return;
    }
    thread << std::setprecision(6);
    thread << "Bus "s << bus.request << ": "s << bus_info.amount_stops << " stops on route, "s << bus_info.amount_unique_stops;
    thread << " unique stops, "s << bus_info.length << " route length, "s << bus_info.curvature << " curvature"s << std::endl;
}

std::vector<Request> ReadRequests(std::istream& thread) {
    TypeRequest type;
    std::string in;
    getline(thread, in);
    int count = std::stoi(in);
    std::vector<Request> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        getline(thread, in);
        std::string_view in_sv(in);
        auto space_pos = in_sv.find_first_of(' ');
        auto key_word = in_sv.substr(0, space_pos);
        if (key_word == "Stop"sv) {
            type = TypeRequest::stop;
        }
        else if (key_word == "Bus"sv) {
            type = TypeRequest::bus;
        }
        else {
            throw std::invalid_argument("Input contains not correct command!");
        }
        in.erase(0, space_pos + 1);
        result.push_back({ type, std::move(in) });
    }
    return result;
}

void OutputInfo(std::vector<Request> name_requests, TransportCatalogue& tran_cat, std::ostream& thread) {
    for (const auto& request : name_requests) {
        if (request.type == TypeRequest::bus) {
            OutputBusInfo(request, tran_cat, thread);
        }
        else {
            OutputStopInfo(request, tran_cat, thread);
        }
    }
}
}
}