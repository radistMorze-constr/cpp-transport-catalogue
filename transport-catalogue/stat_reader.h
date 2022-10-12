#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

namespace transport_catalogue {
enum TypeRequest {
	bus,
	stop
};

struct Request {
	TypeRequest type_;
	std::string request_;
};

namespace reader_requests {
std::vector<Request> ReadRequests(std::istream& thread);
}
namespace output_request {
void OutputInfo(std::vector<Request> name_requests, TransportCatalogue& tran_cat, std::ostream& thread);
}
}