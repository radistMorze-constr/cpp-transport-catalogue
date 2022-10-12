#include "geo.h"
#include "stat_reader.h"

namespace transport_catalogue {
using namespace std::literals;

namespace detail {
void OutputStopInfo(Request stop, TransportCatalogue& tran_cat, std::ostream& thread) {
    thread << "Stop "s << stop.request_ << ": "s;
    try {
        auto busses = tran_cat.GetListBusses(stop.request_);
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
    auto bus_info = tran_cat.GetInfromBus(bus.request_);
    if (bus_info.not_found_) {
        thread << "Bus "s << bus.request_ << ": not found"s << std::endl;
        return;
    }
    thread << std::setprecision(6);
    thread << "Bus "s << bus.request_ << ": "s << bus_info.amount_stops_ << " stops on route, "s << bus_info.amount_unique_stops_;
    thread << " unique stops, "s << bus_info.length_ << " route length, "s << bus_info.curvature_ << " curvature"s << std::endl;
}
}

namespace reader_requests {
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
}

namespace output_request {
void OutputInfo(std::vector<Request> name_requests, TransportCatalogue& tran_cat, std::ostream& thread) {
    for (const auto& request : name_requests) {
        if (request.type_ == TypeRequest::bus) {
            detail::OutputBusInfo(request, tran_cat, thread);
        }
        else {
            detail::OutputStopInfo(request, tran_cat, thread);
        }
    }
}
}
}