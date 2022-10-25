#include "domain.h"

namespace transport_catalogue {
Bus::Bus(const std::string&& name, std::vector<const Stop*>&& stops, TypeRoute&& type_route, std::unordered_set<std::string_view>&& unique_stops) :
	name(std::move(name)),
	stops(std::move(stops)),
	type_route(std::move(type_route)),
	unique_stops(std::move(unique_stops))
{
}
} //namespace transport_catalogue