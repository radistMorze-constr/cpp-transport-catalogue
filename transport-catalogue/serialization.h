#pragma once
#include <transport_catalogue.pb.h>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace serialization {
	class Serializator {
	public:
		static transport_catalogue_serialize::TransportCatalogue* SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& tran_cat);
		static transport_catalogue::TransportCatalogue DeserializeTransportCatalogue(const transport_catalogue_serialize::TransportCatalogue& proto_tran_cat);

		static rendering_serialize::RenderSettings* SerializeMapRender(const transport_catalogue::rendering::RenderSettings& render_settings);
		static transport_catalogue::rendering::RenderSettings DeserializeSerializeRenderSettings(const rendering_serialize::RenderSettings proto_render_settings);
		
		static transport_router_serialize::TransportRouter* SerializeTransportRouter(const transport_router::TransportRouter& tran_router);
		static transport_router::TransportRouter DeserializeRouteSettings(const transport_router_serialize::TransportRouter& proto_tran_router,
			const std::unordered_map<std::string_view, const transport_catalogue::Stop*>& stopname_to_stop,
			const std::map<std::string_view, const transport_catalogue::Bus*>& busname_to_bus);

		static void SerializeFacade(transport_catalogue_serialize::TransportCatalogue* proto_tran_cat,
			rendering_serialize::RenderSettings* proto_render_settings,
			transport_router_serialize::TransportRouter* proto_tran_router,
			std::string filename);
		static transport_catalogue_serialize::Facade* DeserializeFacade(std::string filename);
	};
} //serialization