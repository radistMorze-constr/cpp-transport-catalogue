#include "json_reader.h"
#include "serialization.h"
//#include "tests.h"

#include <iostream>
#include <string>
#include <fstream>

using namespace transport_catalogue;
using namespace handle_iformation;
using namespace geo;
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    //TestTransportCatalogue();
    
    std::istream& stream_input = std::cin;
    std::ostream& stream_output = std::cout;
    /*
    std::fstream stream_input_make_base("s14_3_opentest_2_make_base.json");
    std::fstream stream_input_process_requests("s14_3_opentest_2_process_requests.json");

    std::fstream clear_output("opentest_myanswer.json", std::ios::out);
    std::fstream stream_output("opentest_myanswer.json");
    */
    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        auto data_doc = DataDocument::MakeDataDocument(stream_input);
        MakeBase facade(data_doc.GetDocument());
        TransportCatalogue tran_cat = facade.MakeTransportCatalogue();
        rendering::MapRenderer map_render = facade.MakeMapRenderer();
        transport_router::TransportRouter trant_router = facade.MakeTransportRouter(tran_cat);
        serialization::SerializeFacade(tran_cat, map_render, trant_router, data_doc.GetSerializationFile());
    }
    else if (mode == "process_requests"sv) {
        auto data_doc = DataDocument::MakeDataDocument(stream_input);
        std::unique_ptr<transport_catalogue_serialize::Facade> proto_facade(serialization::DeserializeFacade(data_doc.GetSerializationFile()));
        auto tran_cat = serialization::DeserializeTransportCatalogue(proto_facade->tran_cat());
        auto map_render = rendering::MapRenderer{ serialization::DeserializeSerializeRenderSettings(proto_facade->render_settings()) };
        auto transport_router = serialization::DeserializeRouteSettings(proto_facade->tran_router(),
            tran_cat.GetStopnameToStop(), tran_cat.GetBusnameToBus());
        ProcessRequests facade(data_doc.GetDocument()
            , &tran_cat
            , &map_render
            , &transport_router);
        //facade.SetTransportCatalogue(&tran_cat).SetMapRenderer(&map_render).SetTransportRouter(&transport_router);
        facade.AsnwerRequests(stream_output);
    }
    else {
        PrintUsage();
        return 1;
    }
}