#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <sha256.h>;
#include <base64.h>

using NJson = nlohmann::json;

void migrate_db() {
    sql::Driver* driver;
    sql::Connection* con;
    sql::PreparedStatement* prep_stmt;

    try {
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "root", "");
        prep_stmt = con->prepareStatement("CREATE DATABASE IF NOT EXISTS stegz");
        prep_stmt->execute();

        con->setSchema("stegz");
        prep_stmt = con->prepareStatement("CREATE TABLE IF NOT EXISTS users(uuid VARCHAR(16) DEFAULT (uuid()) NOT NULL PRIMARY KEY, username VARCHAR(255) NOT NULL, email VARCHAR(255) NOT NULL, first_name VARCHAR(255) NOT NULL, last_name VARCHAR(255), password TEXT, is_verified BOOLEAN DEFAULT FALSE, role VARCHAR(20) NOT NULL, created_at TIMESTAMP)");
        prep_stmt->execute();

        prep_stmt = con->prepareStatement("CREATE TABLE IF NOT EXISTS access_tokens(uuid VARCHAR(16) DEFAULT (uuid()) NOT NULL PRIMARY KEY, user_uuid VARCHAR(16) NOT NULL, token TEXT NOT NULL, revoked_at TIMESTAMP NULL, created_at TIMESTAMP DEFAULT NOW())");
        prep_stmt->execute();
    }
    catch (sql::SQLException sql_e) {
        con->close();

        std::cout << "Gagal migrasi database: " << sql_e.what() << std::endl;
        system("pause");
        exit(1);
    }

    con->close();

    delete prep_stmt;
    delete con;
}

int main() {
    migrate_db();
    httplib::Server srv;

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
        project_information["repository"] = "https://github.com/erlanggariansyah/stegz";
        project_information["created_at"] = "12/09/2023";

        res.set_content(project_information.dump(), "application/json");
    });


    srv.Post("/api/v1/login", [](const httplib::Request& req, httplib::Response& res) {
        NJson request_body = NJson::parse(req.body);
        NJson response;
        
        std::vector<std::string> error_fields;

        if (!request_body.contains("email") || !request_body["email"].is_string() || request_body["email"].get<std::string>().find('@') == std::string::npos) {
            error_fields.push_back("email");
        }

        if (request_body.contains("password") && request_body["password"].is_string()) {
            if (request_body["password"].get<std::string>().size() < 8) {
                error_fields.push_back("password");
            }
        }
        else {
            error_fields.push_back("password");
        }

        if (error_fields.size() > 0) {
            response["code"] = 400;
            response["message"] = "Bad Request";
            response["data"] = error_fields;

            res.status = 400;
            res.set_content(response.dump(), "application/json");
        }
        else {
            try {
                SHA256 sha256;

                sql::Driver* driver{ get_driver_instance() };
                sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
                sql::PreparedStatement* prep_stmt;
                sql::ResultSet* result_query;

                con->setSchema("stegz");

                std::string email(request_body.at("email"));
                std::string password(request_body.at("password"));

                prep_stmt = con->prepareStatement("SELECT * FROM users WHERE email = ? LIMIT 1");
                prep_stmt->setString(1, email);

                result_query = prep_stmt->executeQuery();
                if (result_query->next()) {
                    std::string user_password = result_query->getString("password");

                    if (user_password == sha256(password)) {
                        NJson user_information;
                        user_information["uuid"] = result_query->getString("uuid");
                        user_information["email"] = result_query->getString("email");
                        user_information["username"] = result_query->getString("username");
                        user_information["first_name"] = result_query->getString("first_name");
                        user_information["last_name"] = result_query->getString("last_name");
                        user_information["role"] = result_query->getString("role");
                        user_information["created_at"] = result_query->getString("created_at");

                        std::string token{ base64_encode(reinterpret_cast<const unsigned char*>(user_information.dump().c_str()), user_information.dump().length()) };

                        prep_stmt = con->prepareStatement("INSERT INTO access_tokens(user_uuid, token) VALUES(?, ?)");
                        prep_stmt->setString(1, user_information["uuid"].get<std::string>());
                        prep_stmt->setString(2, token);
                        prep_stmt->execute();

                        con->close();

                        delete prep_stmt;
                        delete con;
                        delete result_query;

                        response["code"] = 200;
                        response["message"] = "OK";
                        response["data"]["user"] = user_information;
                        response["data"]["token"] = token;

                        res.status = 200;
                        res.set_content(response.dump(), "application/json");
                    }
                    else {
                        error_fields.push_back("email");
                        error_fields.push_back("password");

                        con->close();

                        delete prep_stmt;
                        delete con;
                        delete result_query;

                        response["code"] = 401;
                        response["message"] = "Unauthorized";
                        response["data"] = error_fields;

                        res.status = 401;
                        res.set_content(response.dump(), "application/json");
                    }
                }
                else {
                    error_fields.push_back("email");
                    error_fields.push_back("password");

                    con->close();

                    delete prep_stmt;
                    delete con;
                    delete result_query;

                    response["code"] = 401;
                    response["message"] = "Unauthorized";
                    response["data"] = error_fields;

                    res.status = 401;
                    res.set_content(response.dump(), "application/json");
                }
            }
            catch (sql::SQLException sql_e) {
                response["code"] = 500;
                response["message"] = "Internal Server Error";
                response["data"] = "Error connection to database.";

                res.status = 500;
                res.set_content(response.dump(), "application/json");
            }
        }
    });

    srv.Post("/api/v1/register", [](const httplib::Request& req, httplib::Response& res) {
        NJson request_body = NJson::parse(req.body);
        NJson response;

        std::vector<std::string> error_fields;

        if (!request_body.contains("email") || !request_body["email"].is_string() || request_body["email"].get<std::string>().find('@') == std::string::npos) {
            error_fields.push_back("email");
        }

        if (!request_body.contains("first_name")) {
            error_fields.push_back("first_name");
        }

        if (!request_body.contains("last_name")) {
            error_fields.push_back("last_name");
        }

        if (request_body.contains("password") && request_body["password"].is_string()) {
            if (request_body["password"].get<std::string>().size() < 8) {
                error_fields.push_back("password");
            }
        }
        else {
            error_fields.push_back("password");
        }

        if (error_fields.size() > 0) {
            response["code"] = 400;
            response["message"] = "Bad Request";
            response["data"] = error_fields;

            res.status = 400;
            res.set_content(response.dump(), "application/json");
        }
        else {
            try {
                SHA256 sha256;

                sql::Driver* driver{ get_driver_instance() };
                sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
                sql::PreparedStatement* prep_stmt;
                sql::ResultSet* result_query;

                con->setSchema("stegz");

                std::string email(request_body.at("email"));
                std::string username = "stegz_" + request_body["email"].get<std::string>().substr(0, email.find("@"));
                std::string first_name(request_body.at("first_name"));
                std::string last_name(request_body.at("last_name"));
                std::string password(request_body.at("password"));

                prep_stmt = con->prepareStatement("SELECT * FROM users WHERE email = ? LIMIT 1");
                prep_stmt->setString(1, email);

                result_query = prep_stmt->executeQuery();
                if (result_query->next()) {
                    error_fields.push_back("email");

                    response["code"] = 400;
                    response["message"] = "Bad Request";
                    response["data"] = error_fields;

                    res.status = 422;
                    res.set_content(response.dump(), "application/json");
                }
                else {
                    prep_stmt = con->prepareStatement("INSERT INTO users(username, email, first_name, last_name, password, role) VALUES(?, ?, ?, ?, ?, 'USER')");

                    prep_stmt->setString(1, username);
                    prep_stmt->setString(2, email);
                    prep_stmt->setString(3, first_name);
                    prep_stmt->setString(4, last_name);
                    prep_stmt->setString(5, sha256(password));
                    prep_stmt->execute();

                    con->close();

                    delete prep_stmt;
                    delete con;
                    delete result_query;

                    response["code"] = 200;
                    response["message"] = "OK";
                    response["data"] = NULL;

                    res.set_content(response.dump(), "application/json");
                }
            }
            catch (sql::SQLException sql_e) {
                response["code"] = 500;
                response["message"] = "Internal Server Error";
                response["data"] = "Error connection to database.";

                res.status = 500;
                res.set_content(response.dump(), "application/json");
            }
        }
    });

    srv.listen("127.0.0.1", 8080);

    return 0;
}
