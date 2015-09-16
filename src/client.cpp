#include <client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <QVariantMap>
#include <iostream>

namespace http = core::net::http;
namespace net = core::net;

using namespace std;

Client::Client(Config::Ptr config) :
    config_(config), cancelled_(false) {
}


void Client::get(const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot, path, parameters);
    configuration.uri = client->uri_to_string(uri);

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Client::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());

        // Open weather map API error code can either be a string or int
        QVariant cod = root.toVariant().toMap()["cod"];
        if ((cod.canConvert<QString>() && cod.toString() != "200")
                || (cod.canConvert<unsigned int>() && cod.toUInt() != 200)) {
            throw domain_error(root.toVariant().toMap()["message"].toString().toStdString());
        }
    } catch (net::Error &) {
    }
}

int Client::IsLive(string &name){

    QJsonDocument root;

    // Get the response from the channels/CHANNELNAME endpoint
    get( { "channels", name }, {}, root );

    // Convert the returned root JSON object to a QVariantMap (Dictionary, sorta)
    QVariantMap variant = root.toVariant().toMap();

    // If it's not null it will be 0
    return variant["stream"].toUInt();

}

Client::StreamRes Client::streams(const string &query, const bool &s_thumbnail) {
    // This is the method that we will call from the Query class.
    // It connects to an HTTP source and returns the results.
    // In this case we are going to retrieve JSON data.
    QJsonDocument root;

    // Build a URI and get the contents.
    // The fist parameter forms the path part of the URI.
    // The second parameter forms the CGI parameters.
    get( { "search", "streams" }, {{ "q", query }}, root);

    StreamRes result;

    QVariantMap variant = root.toVariant().toMap();

    // Iterate through the stream data
    for (const QVariant &i : variant["streams"].toList()) {

        QVariantMap channel = i.toMap();
        QVariantMap previews = channel["preview"].toMap();

        std::string viewers = channel["viewers"].toString().toStdString();
        channel = channel["channel"].toMap();

        std::string mature = channel["mature"].toString().toStdString();
        std::string name = channel["name"].toString().toStdString();
        std::string live = "";

        if (s_thumbnail){
            // Checks if the stream is live
            if (IsLive(name) == 0){
                live = "Live! - ";
            }
            else{
                live = "h";
            }
         }

        // Convert boolean string from "true" or "false" to "Yes" or "No"
        if (mature == "true"){
            mature = "Yes";
        }
        else if (mature == "false"){
            mature = "No";
        }


        // We add each result to our list
        result.streams.emplace_back(
            Stream {
                channel["game"].toString().toStdString(),
                channel["status"].toString().toStdString(),
                viewers,
                channel["url"].toString().toStdString(),
                channel["logo"].toString().toStdString(),
                mature,
                live,
                previews["large"].toString().toStdString(),
                Channel {
                    channel["_id"].toUInt(),
                    channel["display_name"].toString().toStdString()
                }
            }
        );
    }

    return result;
}


http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Client::Config::Ptr Client::config() {
    return config_;
}

