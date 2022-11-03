#include "transport_catalogue.h"
#include "json_reader.h"
//#include "tests.h"

#include <iostream>
#include <string>
#include <fstream>

using namespace transport_catalogue;
using namespace handle_iformation;
using namespace geo;

int main() {
    //TestTransportCatalogue();

    std::istream& stream_input = std::cin;
    //std::fstream stream_input("s10_final_opentest_3.json");
    std::ostream& stream_output = std::cout;

    Facade facade(stream_input);
    facade.AsnwerRequests(stream_output);
    //facade.RenderRoute(stream_output);
}