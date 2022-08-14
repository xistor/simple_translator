
add_executable(server ./server.cpp ./Translator.cpp)


include_directories(${OPENSSL_INCLUDE_DIRS})
link_directories(${OPENSSL_LIBRARIES})


target_link_libraries(server OpenSSL::Crypto)
target_link_libraries(server ${CURL_LIBRARY})
target_link_libraries(server pthread)
target_link_libraries(server nlohmann_json::nlohmann_json)

install(TARGETS server DESTINATION translator)


