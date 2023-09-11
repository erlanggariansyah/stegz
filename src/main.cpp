#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

using NJson = nlohmann::json;

int main() {
    sql::Driver* driver;
    sql::Connection* con;
    sql::PreparedStatement* prep_stmt;

    try {
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "root", "");
        con->setSchema("stegz");
    }
    catch (sql::SQLException sql_e) {
        std::cout << "Gagal koneksi ke database: " << sql_e.what() << std::endl;
        system("pause");
        exit(1);
    }

    // membuat instance server
    httplib::Server srv;

    // route mapping
    srv.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        NJson project_information;
        std::string project_stacks[5];

        project_stacks[0] = "C++";
        project_stacks[1] = "cpp-httplib";
        project_stacks[2] = "MySQL";
        project_stacks[3] = "React";
        project_stacks[4] = "JavaScript";

        project_information["name"] = "stegZ";
        project_information["description"] = "stegZ Project is a simple-minimalistic steganography tool for IBM Hybrid Cloud & AI Project Capstone";
        project_information["creator"] = "Erlangga Riansyah";
        project_information["stacks"] = project_stacks;
        project_information["created_at"] = "12/09/2023";

        res.set_content(project_information.dump(), "application/json");
    });

    srv.Post("/register", [](const httplib::Request& req, httplib::Response& res) {
        std::string request_body_string(req.body.c_str());

        res.set_content(NJson::parse(request_body_string), "application/json");
    });

    // listen server ke port 8080
    srv.listen("127.0.0.1", 8080);

    return 0;
}
