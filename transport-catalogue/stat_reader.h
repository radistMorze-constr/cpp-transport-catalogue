#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

namespace transport_catalogue {
namespace stat_reader {
enum class TypeRequest {
	bus,
	stop
};

struct Request {
	TypeRequest type;
	std::string request;
};

std::vector<Request> ReadRequests(std::istream& thread);
void OutputInfo(std::vector<Request> name_requests, TransportCatalogue& tran_cat, std::ostream& thread);
}
}