#include <iostream>
#include <algorithm>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <sha256.h>
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

        prep_stmt = con->prepareStatement("CREATE TABLE IF NOT EXISTS access_tokens(uuid VARCHAR(16) DEFAULT (uuid()) NOT NULL PRIMARY KEY, user_uuid VARCHAR(16) NOT NULL, token TEXT NOT NULL, source VARCHAR(10) NOT NULL, revoked_at TIMESTAMP NULL, comment TEXT, created_at TIMESTAMP DEFAULT NOW())");
        prep_stmt->execute();

        prep_stmt = con->prepareStatement("CREATE TABLE IF NOT EXISTS plans(uuid VARCHAR(16) DEFAULT (uuid()) NOT NULL PRIMARY KEY, tier INT(2) NOT NULL, plan_name VARCHAR(36) NOT NULL, description TEXT, price VARCHAR(100) NOT NULL, availability VARCHAR(100) NOT NULL DEFAULT 'Not Available', created_at TIMESTAMP DEFAULT NOW())");
        prep_stmt->execute();

        prep_stmt = con->prepareStatement("CREATE TABLE IF NOT EXISTS plan_usages(uuid VARCHAR(16) DEFAULT (uuid()) NOT NULL PRIMARY KEY, user_uuid VARCHAR(16) NOT NULL, tier INT(2) NOT NULL, plan_name VARCHAR(36) NOT NULL, price BIGINT NOT NULL, canceled_at TIMESTAMP NULL, created_at TIMESTAMP DEFAULT NOW())");
        prep_stmt->execute();

        prep_stmt = con->prepareStatement("CREATE TABLE IF NOT EXISTS billing_usages(uuid VARCHAR(16) DEFAULT (uuid()) NOT NULL PRIMARY KEY, user_uuid VARCHAR(16) NOT NULL, action VARCHAR(100) NOT NULL, price BIGINT NOT NULL, created_at TIMESTAMP DEFAULT NOW())");
        prep_stmt->execute();

        // prep_stmt = con->prepareStatement("INSERT INTO plans(tier, plan_name, description, price, availability) VALUES (1, 'Free', '10 maximum free encoding count, 10 maximum free decoding count, no-SSO', 0, 'Not Available'), (2, 'SeaLion', '100 maximum free encoding count, 100 maximum free decoding count, SSO', 300000, 'Not Available'), (3, 'Nordwind', '1000 maximum free encoding count, 1000 maximum free decoding count, SSO', 800000, 'Not Available'), (4, 'Uranus', '10000 maximum free encoding count, 10000 maximum free decoding count, SSO', 1800000, 'Not Available'), (5, 'Vistula-Oder', 'Unlimited encoding count, unlimited decoding count, SSO', 9200000, 'Available - Auto Apply')");
        // prep_stmt->execute();
    }
    catch (sql::SQLException sql_e) {
        std::cout << "Gagal migrasi database: " << sql_e.what() << std::endl;
        system("pause");
        exit(1);
    }

    con->close();

    delete prep_stmt;
    delete con;
}

std::string generate_random_string(size_t length) {
    auto randchar = []() -> char
        {
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };

    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);

    return str;
}

int main() {
    migrate_db();
    httplib::Server srv;

    srv.Options("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    });

    srv.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");

        NJson project_information;
        std::string project_stacks[6];

        project_stacks[0] = "C++";
        project_stacks[1] = "cpp-httplib";
        project_stacks[2] = "MySQL";
        project_stacks[3] = "React";
        project_stacks[4] = "JavaScript";
        project_stacks[5] = "IBM Security Verify";

        project_information["name"] = "stegZ";
        project_information["description"] = "stegZ Project is a simple-minimalistic web-application steganography tool for IBM Hybrid Cloud & AI Project Capstone";
        project_information["creator"] = "Erlangga Riansyah";
        project_information["stacks"] = project_stacks;
        project_information["repository"] = "https://github.com/erlanggariansyah/stegz";
        project_information["created_at"] = "12/09/2023";

        res.status = 200;
        res.set_content(project_information.dump(), "application/json");
    });

    srv.Options("/api/v1/plans", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    });

    srv.Get("/api/v1/plans", [](const httplib::Request& req, httplib::Response& res) {
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");

        sql::Driver* driver{ get_driver_instance() };
        sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
        sql::PreparedStatement* prep_stmt;
        sql::ResultSet* result_query;

        con->setSchema("stegz");

        prep_stmt = con->prepareStatement("SELECT * FROM plans ORDER BY created_at ASC");
        result_query = prep_stmt->executeQuery();

        std::vector<NJson> plans;
        while (result_query->next()) {
            NJson plan;
            plan["uuid"] = result_query->getString("uuid");
            plan["tier"] = result_query->getInt("tier");
            plan["plan_name"] = result_query->getString("plan_name");
            plan["description"] = result_query->getString("description");
            plan["price"] = result_query->getInt("price");
            plan["availability"] = result_query->getString("availability");
            plan["created_at"] = result_query->getString("created_at");

            plans.push_back(plan);
        }

        con->close();

        delete prep_stmt;
        delete con;
        delete result_query;

        response["code"] = 200;
        response["message"] = "OK";
        response["data"] = plans;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    });

    srv.Options("/api/v1/sessions/comment", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    });

    srv.Post("/api/v1/sessions/comment", [](const httplib::Request& req, httplib::Response& res) {
        NJson request_body = NJson::parse(req.body);
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");

        std::vector<std::string> error_fields;

        if (!request_body.contains("access_token_id")) {
            error_fields.push_back("access_token_id");
        }

        if (!request_body.contains("comment")) {
            error_fields.push_back("comment");
        }

        if (error_fields.size() > 0) {
            response["code"] = 400;
            response["message"] = "Bad Request";
            response["data"] = error_fields;

            res.status = 400;
            res.set_content(response.dump(), "application/json");
        }
        else {
            if (req.has_header("Authorization")) {
                std::string bearer_token(req.get_header_value("Authorization").substr(7));

                sql::Driver* driver{ get_driver_instance() };
                sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
                sql::PreparedStatement* prep_stmt;
                sql::ResultSet* result_query;

                con->setSchema("stegz");

                prep_stmt = con->prepareStatement("SELECT * FROM access_tokens WHERE token = ? AND revoked_at IS NULL LIMIT 1");
                prep_stmt->setString(1, bearer_token);

                result_query = prep_stmt->executeQuery();
                if (result_query->next()) {
                    std::string user_uuid(result_query->getString("user_uuid"));
                    std::string access_token_id(request_body.at("access_token_id"));

                    prep_stmt = con->prepareStatement("SELECT * FROM access_tokens WHERE uuid = ? LIMIT 1");
                    prep_stmt->setString(1, access_token_id);
                    result_query = prep_stmt->executeQuery();

                    if (result_query->next()) {
                        if (result_query->getString("user_uuid") != user_uuid) {
                            con->close();

                            delete prep_stmt;
                            delete con;
                            delete result_query;

                            response["code"] = 401;
                            response["message"] = "Unauthorized";
                            response["data"] = "You don't have access to perform this action.";

                            res.status = 401;
                            res.set_content(response.dump(), "application/json");
                        }
                        else {
                            std::string comment(request_body.at("comment"));

                            prep_stmt = con->prepareStatement("UPDATE access_tokens SET comment = ? WHERE uuid = ?");
                            prep_stmt->setString(1, comment);
                            prep_stmt->setString(2, access_token_id);
                            prep_stmt->execute();

                            con->close();

                            delete prep_stmt;
                            delete con;
                            delete result_query;

                            response["code"] = 200;
                            response["message"] = "OK";
                            response["data"] = "Success adding comment.";

                            res.status = 200;
                            res.set_content(response.dump(), "application/json");
                        }
                    }
                    else {
                        con->close();

                        delete prep_stmt;
                        delete con;
                        delete result_query;

                        response["code"] = 404;
                        response["message"] = "Not Found";
                        response["data"] = "The token with given id is not found.";

                        res.status = 404;
                        res.set_content(response.dump(), "application/json");
                    }
                }
                else {
                    con->close();

                    delete prep_stmt;
                    delete con;
                    delete result_query;

                    response["code"] = 400;
                    response["message"] = "Bad Request";
                    response["data"] = "The authorization token is not valid.";

                    res.status = 400;
                    res.set_content(response.dump(), "application/json");
                }
            }
            else {
                response["code"] = 401;
                response["message"] = "Unauthorized";
                response["data"] = "You don't have access to perform this action.";

                res.status = 401;
                res.set_content(response.dump(), "application/json");
            }
        }
    });

    srv.Options("/api/v1/billings/usage", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    });

    srv.Post("/api/v1/billings/usage", [](const httplib::Request& req, httplib::Response& res) {
        NJson request_body = NJson::parse(req.body);
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");

        if (req.has_header("Authorization")) {
            std::string bearer_token(req.get_header_value("Authorization").substr(7));

            sql::Driver* driver{ get_driver_instance() };
            sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
            sql::PreparedStatement* prep_stmt;
            sql::ResultSet* result_query;

            con->setSchema("stegz");

            prep_stmt = con->prepareStatement("SELECT * FROM access_tokens WHERE token = ? AND revoked_at IS NULL LIMIT 1");
            prep_stmt->setString(1, bearer_token);

            result_query = prep_stmt->executeQuery();
            if (result_query->next()) {
                std::string action(request_body.at("action"));
                std::string price(request_body.at("price"));

                prep_stmt = con->prepareStatement("INSERT INTO billing_usages(user_uuid, action, price) VALUES (?, ?, ?)");
                prep_stmt->setString(1, result_query->getString("user_uuid"));
                prep_stmt->setString(2, action);
                prep_stmt->setString(3, price);
                prep_stmt->execute();

                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 200;
                response["message"] = "OK";
                response["data"] = "Success add billing usage.";

                res.status = 200;
                res.set_content(response.dump(), "application/json");
            }
            else {
                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 400;
                response["message"] = "Bad Request";
                response["data"] = "The token has been revoked.";

                res.status = 400;
                res.set_content(response.dump(), "application/json");
            }
        }
        else {
            response["code"] = 401;
            response["message"] = "Unauthorized";
            response["data"] = "You don't have access to perform this action.";

            res.status = 401;
            res.set_content(response.dump(), "application/json");
        }
    });

    srv.Options("/api/v1/billings", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    });

    srv.Get("/api/v1/billings", [](const httplib::Request& req, httplib::Response& res) {
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");

        if (req.has_header("Authorization")) {
            std::string bearer_token(req.get_header_value("Authorization").substr(7));

            sql::Driver* driver{ get_driver_instance() };
            sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
            sql::PreparedStatement* prep_stmt;
            sql::ResultSet* result_query;

            con->setSchema("stegz");

            prep_stmt = con->prepareStatement("SELECT * FROM access_tokens WHERE token = ? AND revoked_at IS NULL LIMIT 1");
            prep_stmt->setString(1, bearer_token);

            result_query = prep_stmt->executeQuery();
            if (result_query->next()) {
                prep_stmt = con->prepareStatement("SELECT * FROM billing_usages WHERE user_uuid = ? ORDER BY created_at DESC");
                prep_stmt->setString(1, result_query->getString("user_uuid"));
                result_query = prep_stmt->executeQuery();

                std::vector<NJson> billing_usages;
                while (result_query->next()) {
                    NJson billing_usage;
                    billing_usage["uuid"] = result_query->getString("uuid");
                    billing_usage["action"] = result_query->getString("action");
                    billing_usage["price"] = result_query->getString("price");
                    billing_usage["created_at"] = result_query->getString("created_at");

                    billing_usages.push_back(billing_usage);
                }

                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 200;
                response["message"] = "OK";
                response["data"] = billing_usages;

                res.status = 200;
                res.set_content(response.dump(), "application/json");
            }
            else {
                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 400;
                response["message"] = "Bad Request";
                response["data"] = "The token has been revoked.";

                res.status = 400;
                res.set_content(response.dump(), "application/json");
            }
        }
        else {
            response["code"] = 401;
            response["message"] = "Unauthorized";
            response["data"] = "You don't have access to perform this action.";

            res.status = 401;
            res.set_content(response.dump(), "application/json");
        }
    });

    srv.Options("/api/v1/sessions", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    });

    srv.Get("/api/v1/sessions", [](const httplib::Request& req, httplib::Response& res) {
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");

        if (req.has_header("Authorization")) {
            std::string bearer_token(req.get_header_value("Authorization").substr(7));

            sql::Driver* driver{ get_driver_instance() };
            sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
            sql::PreparedStatement* prep_stmt;
            sql::ResultSet* result_query;

            con->setSchema("stegz");

            prep_stmt = con->prepareStatement("SELECT * FROM access_tokens WHERE token = ? AND revoked_at IS NULL LIMIT 1");
            prep_stmt->setString(1, bearer_token);

            result_query = prep_stmt->executeQuery();
            if (result_query->next()) {
                prep_stmt = con->prepareStatement("SELECT * FROM access_tokens WHERE user_uuid = ? ORDER BY created_at DESC");
                prep_stmt->setString(1, result_query->getString("user_uuid"));
                result_query = prep_stmt->executeQuery();

                std::vector<NJson> access_tokens;
                while (result_query->next()) {
                    NJson access_token;
                    access_token["uuid"] = result_query->getString("uuid");
                    access_token["source"] = result_query->getString("source");
                    access_token["revoked_at"] = result_query->getString("revoked_at");
                    access_token["created_at"] = result_query->getString("created_at");
                    access_token["comment"] = result_query->getString("comment");

                    access_tokens.push_back(access_token);
                }

                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 200;
                response["message"] = "OK";
                response["data"] = access_tokens;

                res.status = 200;
                res.set_content(response.dump(), "application/json");
            }
            else {
                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 400;
                response["message"] = "Bad Request";
                response["data"] = "The token has been revoked.";

                res.status = 400;
                res.set_content(response.dump(), "application/json");
            }
        }
        else {
            response["code"] = 401;
            response["message"] = "Unauthorized";
            response["data"] = "You don't have access to perform this action.";

            res.status = 401;
            res.set_content(response.dump(), "application/json");
        }
    });

    srv.Options("/api/v1/logout", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    });

    srv.Post("/api/v1/logout", [](const httplib::Request& req, httplib::Response& res) {
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");

        if (req.has_header("Authorization")) {
            std::string bearer_token(req.get_header_value("Authorization").substr(7));

            sql::Driver* driver{ get_driver_instance() };
            sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
            sql::PreparedStatement* prep_stmt;
            sql::ResultSet* result_query;

            con->setSchema("stegz");

            prep_stmt = con->prepareStatement("SELECT * FROM access_tokens WHERE token = ? AND revoked_at IS NULL LIMIT 1");
            prep_stmt->setString(1, bearer_token);

            result_query = prep_stmt->executeQuery();
            if (result_query->next()) {
                prep_stmt = con->prepareStatement("UPDATE access_tokens SET revoked_at = NOW() WHERE user_uuid = ? AND revoked_at IS NULL");
                prep_stmt->setString(1, result_query->getString("user_uuid"));
                prep_stmt->execute();

                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 200;
                response["message"] = "OK";
                response["data"] = "Success logout.";

                res.status = 200;
                res.set_content(response.dump(), "application/json");
            }
            else {
                con->close();

                delete prep_stmt;
                delete con;
                delete result_query;

                response["code"] = 400;
                response["message"] = "Bad Request";
                response["data"] = "The token has been revoked.";

                res.status = 400;
                res.set_content(response.dump(), "application/json");
            }
        }
        else {
            response["code"] = 401;
            response["message"] = "Unauthorized";
            response["data"] = "You don't have access to perform this action.";

            res.status = 401;
            res.set_content(response.dump(), "application/json");
        }
    });

    srv.Options("/api/v1/login/ibm", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    });

    srv.Post("/api/v1/login/ibm", [](const httplib::Request& req, httplib::Response& res) {
        NJson request_body = NJson::parse(req.body);
        NJson response;
        
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");

        std::vector<std::string> error_fields;
        if (!request_body.contains("token")) {
            error_fields.push_back("token");
        }

        if (!request_body.contains("email") || !request_body["email"].is_string() || request_body["email"].get<std::string>().find('@') == std::string::npos) {
            error_fields.push_back("email");
        }

        if (!request_body.contains("first_name")) {
            error_fields.push_back("first_name");
        }

        if (!request_body.contains("last_name")) {
            error_fields.push_back("last_name");
        }

        if (error_fields.size() > 0) {
            response["code"] = 400;
            response["message"] = "Bad Request";
            response["data"] = error_fields;

            res.status = 400;
            res.set_content(response.dump(), "application/json");
        }
        else {
            SHA256 sha256;

            sql::Driver* driver{ get_driver_instance() };
            sql::Connection* con{ driver->connect("tcp://127.0.0.1:3306", "root", "") };
            sql::PreparedStatement* prep_stmt;
            sql::ResultSet* result_query;

            con->setSchema("stegz");

            std::string email(request_body.at("email"));
            std::string token(request_body.at("token"));

            prep_stmt = con->prepareStatement("SELECT * FROM users WHERE email = ? LIMIT 1");
            prep_stmt->setString(1, email);

            result_query = prep_stmt->executeQuery();
            if (result_query->next()) {
                NJson user_information;
                user_information["uuid"] = result_query->getString("uuid");
                user_information["email"] = result_query->getString("email");
                user_information["username"] = result_query->getString("username");
                user_information["first_name"] = result_query->getString("first_name");
                user_information["last_name"] = result_query->getString("last_name");
                user_information["role"] = result_query->getString("role");
                user_information["created_at"] = result_query->getString("created_at");
                user_information["session_id"] = generate_random_string(28);

                prep_stmt = con->prepareStatement("UPDATE access_tokens SET revoked_at = NOW() WHERE user_uuid = ? AND revoked_at IS NULL");
                prep_stmt->setString(1, result_query->getString("uuid"));
                prep_stmt->execute();

                prep_stmt = con->prepareStatement("INSERT INTO access_tokens(user_uuid, token, source) VALUES(?, ?, ?)");
                prep_stmt->setString(1, user_information["uuid"].get<std::string>());
                prep_stmt->setString(2, token);
                prep_stmt->setString(3, "IBM");
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
                std::string first_name(request_body.at("first_name"));
                std::string last_name(request_body.at("last_name"));
                std::string password(sha256(email));
                std::string username("stegz_" + email.substr(0, email.find("@")));

                prep_stmt = con->prepareStatement("INSERT INTO users(username, email, first_name, last_name, password, role) VALUES(?, ?, ?, ?, ?, 'USER')");

                prep_stmt->setString(1, username);
                prep_stmt->setString(2, email);
                prep_stmt->setString(3, first_name);
                prep_stmt->setString(4, last_name);
                prep_stmt->setString(5, password);
                prep_stmt->execute();

                prep_stmt = con->prepareStatement("SELECT * FROM users WHERE email = ? LIMIT 1");
                prep_stmt->setString(1, email);

                result_query = prep_stmt->executeQuery();
                if (result_query->next()) {
                    NJson user_information;
                    user_information["uuid"] = result_query->getString("uuid");
                    user_information["email"] = result_query->getString("email");
                    user_information["username"] = result_query->getString("username");
                    user_information["first_name"] = result_query->getString("first_name");
                    user_information["last_name"] = result_query->getString("last_name");
                    user_information["role"] = result_query->getString("role");
                    user_information["created_at"] = result_query->getString("created_at");
                    user_information["session_id"] = generate_random_string(28);

                    prep_stmt = con->prepareStatement("UPDATE access_tokens SET revoked_at = NOW() WHERE user_uuid = ? AND revoked_at IS NULL");
                    prep_stmt->setString(1, result_query->getString("uuid"));
                    prep_stmt->execute();

                    prep_stmt = con->prepareStatement("INSERT INTO access_tokens(user_uuid, token, source) VALUES(?, ?, ?)");
                    prep_stmt->setString(1, user_information["uuid"].get<std::string>());
                    prep_stmt->setString(2, token);
                    prep_stmt->setString(3, "IBM");
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
                    con->close();

                    delete prep_stmt;
                    delete con;
                    delete result_query;

                    response["code"] = 500;
                    response["message"] = "Internal Server Error";
                    response["data"] = "Error inserting user into DB.";

                    res.status = 500;
                    res.set_content(response.dump(), "application/json");
                }
            }
        }
    });

    srv.Options("/api/v1/login", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");        
    });

    srv.Post("/api/v1/login", [](const httplib::Request& req, httplib::Response& res) {
        NJson request_body = NJson::parse(req.body);
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        
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
                        user_information["session_id"] = generate_random_string(28);

                        std::string token{ base64_encode(reinterpret_cast<const unsigned char*>(user_information.dump().c_str()), user_information.dump().length()) };

                        prep_stmt = con->prepareStatement("UPDATE access_tokens SET revoked_at = NOW() WHERE user_uuid = ? AND revoked_at IS NULL");
                        prep_stmt->setString(1, result_query->getString("uuid"));
                        prep_stmt->execute();
                        
                        prep_stmt = con->prepareStatement("INSERT INTO access_tokens(user_uuid, token, source) VALUES(?, ?, ?)");
                        prep_stmt->setString(1, user_information["uuid"].get<std::string>());
                        prep_stmt->setString(2, token);
                        prep_stmt->setString(3, "STEGZ");
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

    srv.Options("/api/v1/register", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    });

    srv.Post("/api/v1/register", [](const httplib::Request& req, httplib::Response& res) {
        NJson request_body = NJson::parse(req.body);
        NJson response;

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");

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

                    res.status = 200;
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
