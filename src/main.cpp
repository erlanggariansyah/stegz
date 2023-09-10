#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <json/json.h>
#include <iostream>

void load_json(const char* file_name, Json::Value& value) {
    std::ifstream ifs(file_name);
    if (!ifs.is_open()) {
        // gagal membuka json
        throw std::exception();
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;

    std::string errs;
    if (!parseFromStream(builder, ifs, &value, &errs)) {
        // gagal parse json
        throw std::exception();
    }
}

int main() {
    // membuka file konfigurasi
    Json::Value conf;
    load_json("./app/config.jsonc", conf);

    // membuat server (dengan ssl) dan meload metadata dari file konfigurasi
    httplib::SSLServer srv(conf.get("ssl_certificate", "cert.pem").asCString(), conf.get("ssl_key", "key.pem").asCString());

    // listen server ke port 443
    srv.listen(conf.get("server_ip", "0.0.0.0").asCString(), conf.get("server_port", 443).asInt());

    return 0;
}
