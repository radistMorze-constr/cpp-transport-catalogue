#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <iostream>

namespace transport_catalogue {
namespace handle_iformation {

class Facade {
public:
	explicit Facade(std::istream& thread);

	void AsnwerRequests(std::ostream& thread);
	void RenderRoute(std::ostream& thread);
	void Serialize();	
	void Deserialize();
protected: // Ќадо сделать private, наследовани€ же нет, не отличаетс€ от private
	void InitialiseBaseRequests();
	void InitialiseRenderSettings();
	void InitializeTransportRouter();
	void InitializeSerializationSettings();
	json::Node HandleBusRequest(const json::Dict& request_as_map) const;
	json::Node HandleStopRequest(const json::Dict& request_as_map) const;
	json::Node HandleMapRequest(const json::Dict& request_as_map);
	json::Node HandleRouteRequest(const json::Dict& request_as_map) const;
private:
	TransportCatalogue tran_cat_;
	rendering::MapRenderer map_render_;
	json::Document document_;
	transport_router::TransportRouter transport_router_;
	std::string serialization_file_;
};

} //namespace handle_iformation
} //namespace transport_catalogue