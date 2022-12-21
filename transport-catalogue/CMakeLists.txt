cmake_minimum_required(VERSION 3.10)

project(Transport_Catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto transport_router.proto)

set(TRANSPORT_CATALOGUE_FILES main.cpp geo.h geo.cpp domain.h domain.cpp transport_catalogue.h transport_catalogue.cpp)
set(ROUTER transport_router.h transport_router.cpp router.h ranges.h graph.h)
set(JSON_REALISATION json.cpp json.h json_builder.cpp json_builder.h json_reader.cpp json_reader.h)
set(GRAPHICS svg.h svg.cpp map_renderer.h map_renderer.cpp)
set(SERIALIZATION serialization.h serialization.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES} ${ROUTER} ${JSON_REALISATION} ${GRAPHICS} ${SERIALIZATION})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)