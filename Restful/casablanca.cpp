#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <random>

#include <cpp_redis/cpp_redis>

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/json.h"
#include "cpprest/filestream.h"
#include "cpprest/containerstream.h"
#include "cpprest/producerconsumerstream.h"

using namespace std;
using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace http;
using namespace http::experimental::listener;

#define TRACE(msg)            wcout << msg

class handler
{
public:
    handler() = default;
    handler(utility::string_t url) : m_listener{ url }
    {
        m_listener.support(methods::GET, std::bind(&handler::handle_get, this, std::placeholders::_1));
        m_listener.support(methods::PUT, std::bind(&handler::handle_put, this, std::placeholders::_1));
        m_listener.support(methods::POST, std::bind(&handler::handle_post, this, std::placeholders::_1));
        m_listener.support(methods::DEL, std::bind(&handler::handle_delete, this, std::placeholders::_1));

        cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);

        client.connect("redis-server", 6379, [](const std::string & host, std::size_t port, cpp_redis::client::connect_state status) {
            if (status == cpp_redis::client::connect_state::dropped) {
                std::cout << "client disconnected from " << host << ":" << port << std::endl;
            }
            });
        
        client.set("visits", "0", [](cpp_redis::reply & reply) {
            std::cout << "set visits 0: " << reply << std::endl;
        });

        client.sync_commit();
    }

    virtual ~handler() = default;

    pplx::task<void>open() { return m_listener.open(); }
    pplx::task<void>close() { return m_listener.close(); }

protected:

private:
    void handle_get(http_request message)
    {
        client.incrby("visits", 1, [](cpp_redis::reply & reply) {
            std::cout << "Visits increased: " << reply << std::endl;
        });

        client.sync_commit();

        ucout << message.to_string() << endl;

        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

        message.relative_uri().path();

        concurrency::streams::fstream::open_istream(U("static/index.html"), std::ios::in).then([=](concurrency::streams::istream is)
            {
                message.reply(status_codes::OK, is, U("text/html"))
                    .then([](pplx::task<void> t)
                        {
                            try {
                                t.get();
                            }
                            catch (...) {
                                //
                            }
                        });
            }).then([=](pplx::task<void>t)
                {
                    try {
                        t.get();
                    }
                    catch (...) {
                        message.reply(status_codes::InternalError, U("INTERNAL ERROR "));
                    }
                });

            return;
    }

    void handle_put(http_request message)
    {
        ucout << message.to_string() << endl;
        string rep = "WRITE YOUR OWN PUT OPERATION";
        message.reply(status_codes::OK, rep);
        return;
    }

    void handle_post(http_request message)
    {
        ucout << message.to_string() << endl;
        message.reply(status_codes::OK, message.to_string());
        return;
    }

    void handle_delete(http_request message)
    {
        ucout << message.to_string() << endl;
        string rep = "WRITE YOUR OWN DELETE OPERATION";
        message.reply(status_codes::OK, rep);
        return;
    }

    void handle_error(pplx::task<void>& t)
    {
        try
        {
            t.get();
        }
        catch (...)
        {
            // Ignore the error, Log it if a logger is available
        }
    }
    http_listener m_listener;
    cpp_redis::client client;
};

std::unique_ptr<handler> g_httpHandler;

void on_initialize(const string_t& address)
{
    uri_builder uri(address);
    
    auto addr = uri.to_uri().to_string();
    g_httpHandler = std::unique_ptr<handler>(new handler(addr));

    try
    {
        g_httpHandler->open()
                       .wait();

        while (true);
    }
    catch (exception const& e)
    {
        wcout << e.what() << endl;
    }

    g_httpHandler->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_shutdown()
{
    g_httpHandler->close().wait();
    return;
}


int main()
{
    utility::string_t port = U("8080");
    utility::string_t address = U("http://+:");
    address.append(port);

    on_initialize(address);
    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    on_shutdown();
    return 0;
}

