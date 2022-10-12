#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <sstream>

namespace transport_catalogue {
namespace reader_ifo {
void ReadInput(TransportCatalogue& tran_cat, std::istream& thread);
}
}