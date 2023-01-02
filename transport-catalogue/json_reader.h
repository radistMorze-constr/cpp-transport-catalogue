#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <iostream>

namespace transport_catalogue {
namespace handle_iformation {

class Facade {
public:
	explicit Facade(std::istream& thread);

	void AsnwerRequests(std::ostream& thread);
	TransportCatalogue MakeTransportCatalogue() const;
	rendering::MapRenderer MakeMapRenderer() const;
	transport_router::TransportRouter MakeTransportRouter(const TransportCatalogue& tran_cat) const;
	std::string GetSerializationFile() const;

	Facade& SetTransportCatalogue(TransportCatalogue* tran_cat);
	Facade& SetMapRenderer(rendering::MapRenderer* map_render);
	Facade& SetTransportRouter(transport_router::TransportRouter* transport_router);
private:
	void InitializeSerializationSettings();
	void RenderRoute(std::ostream& thread);
	json::Node HandleBusRequest(const json::Dict& request_as_map) const;
	json::Node HandleStopRequest(const json::Dict& request_as_map) const;
	json::Node HandleMapRequest(const json::Dict& request_as_map);
	json::Node HandleRouteRequest(const json::Dict& request_as_map) const;
private:
	json::Document document_;
	std::string serialization_file_;

	TransportCatalogue* p_tran_cat_;
	rendering::MapRenderer* p_map_render_;
	transport_router::TransportRouter* p_transport_router_;
};
} //namespace handle_iformation
} //namespace transport_catalogue