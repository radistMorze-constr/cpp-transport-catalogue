#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

namespace transport_catalogue {
namespace handle_iformation {

class Facade {
public:
	explicit Facade(std::istream& thread);
	void AsnwerRequests(std::ostream& thread);
	void RenderRoute(std::ostream& thread);
protected:
	void InitialiseBaseRequests();
	void InitialiseRenderSettings();
	json::Node HandleBusRequest(const json::Dict& request_as_map) const;
	json::Node HandleStopRequest(const json::Dict& request_as_map) const;
	json::Node HandleMapRequest(const json::Dict& request_as_map);
private:
	TransportCatalogue tran_cat_;
	rendering::MapRenderer map_render_;
	json::Document document_;
};

} //namespace handle_iformation
} //namespace transport_catalogue