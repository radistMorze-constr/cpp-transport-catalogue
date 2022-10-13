#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
//#include "tests.h"

#include <iostream>
#include <string>
#include <fstream>

using namespace transport_catalogue;
using namespace stat_reader;
using namespace reader_ifo;
using namespace geo;

int main() {
    //TestTransportCatalogue();

    TransportCatalogue tran_cat;

    std::istream& stream_input = std::cin;
    //std::fstream stream_input("tsC_case1_input.txt"s);
    std::ostream& stream_output = std::cout;

    ReadInput(tran_cat, stream_input);
    std::vector<Request> name_requests = ReadRequests(stream_input);
    OutputInfo(name_requests, tran_cat, stream_output);
}