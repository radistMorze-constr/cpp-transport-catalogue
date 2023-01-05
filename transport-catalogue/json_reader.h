#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <iostream>

namespace transport_catalogue {
namespace handle_iformation {
class DataDocument {
public:
	static DataDocument MakeDataDocument(std::istream& thread);
	const json::Document& GetDocument() const;
	std::string GetSerializationFile() const;
private:
	json::Document document_;
	explicit DataDocument(std::istream& thread);
};

class MakeBase {
public:
	explicit MakeBase(const json::Document& document);

	TransportCatalogue MakeTransportCatalogue() const;
	rendering::MapRenderer MakeMapRenderer() const;
	transport_router::TransportRouter MakeTransportRouter(const TransportCatalogue& tran_cat) const;
private:
	const json::Document& document_;
};

class ProcessRequests {
public:
	explicit ProcessRequests(const json::Document& document
	, TransportCatalogue* tran_cat
	, rendering::MapRenderer* map_render
	, transport_router::TransportRouter* transport_router);

	void AsnwerRequests(std::ostream& thread);
private:
	void RenderRoute(std::ostream& thread);
	json::Node HandleBusRequest(const json::Dict& request_as_map) const;
	json::Node HandleStopRequest(const json::Dict& request_as_map) const;
	json::Node HandleMapRequest(const json::Dict& request_as_map);
	json::Node HandleRouteRequest(const json::Dict& request_as_map) const;
private:
	const json::Document& document_;

	TransportCatalogue* p_tran_cat_;
	rendering::MapRenderer* p_map_render_;
	transport_router::TransportRouter* p_transport_router_;
};
} //namespace handle_iformation
} //namespace transport_catalogue