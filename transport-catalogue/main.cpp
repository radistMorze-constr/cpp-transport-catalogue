#include "transport_catalogue.h"
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
        Facade facade(stream_input);
        facade.Serialize();
    }
    else if (mode == "process_requests"sv) {
        Facade facade(stream_input);
        facade.Deserialize();
        facade.AsnwerRequests(stream_output);
    }
    else {
        PrintUsage();
        return 1;
    }
}