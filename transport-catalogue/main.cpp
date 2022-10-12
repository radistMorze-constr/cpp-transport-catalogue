// место для вашего кода#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
//#include "tests.h"

#include <iostream>
#include <string>
#include <fstream>

using namespace transport_catalogue;

int main() {
    //TestTransportCatalogue();

    TransportCatalogue tran_cat;

    std::istream& thread_i = std::cin;
    //std::fstream thread_i("tsC_case1_input.txt"s);
    std::ostream& thread_o = std::cout;

    reader_ifo::ReadInput(tran_cat, thread_i);
    std::vector<Request> name_requests = reader_requests::ReadRequests(thread_i);
    output_request::OutputInfo(name_requests, tran_cat, thread_o);
}