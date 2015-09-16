#ifndef CLIENT_H_
#define CLIENT_H_

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <core/net/http/request.h>
#include <core/net/uri.h>

#include <QJsonDocument>

/**
 * Provide a nice way to access the HTTP API.
 *
 * We don't want our scope's code to be mixed together with HTTP and JSON handling.
 */
class Client {
public:

    /**
     * Client configuration
     */
    struct Config {
        typedef std::shared_ptr<Config> Ptr;

        // The root of all API request URLs
        std::string apiroot { "https://api.twitch.tv/kraken" };

        // The custom HTTP user agent string for this library
        std::string user_agent { "example-network-scope 0.1; (foo)" };
    };

    /**
    * Our Channel object.
    */
    struct Channel {
        unsigned int id;
        std::string user;
    };

    /**
    * Stream info, including the channel.
    */
    struct Stream {
        std::string game;       // Channel Game
        std::string name;       // Channel Name
        std::string viewers;    // Stream Viewers
        std::string url;        // Stream URL
        std::string logo;       // Stream Logo URL
        std::string mature;     // Is the stream mature
        std::string isLive;     // Is the channel live
        std::string thumbnail;  // Stream thumbnail
        Channel channel;
    };

    /**
    * A list of Track objects.
    */
    typedef std::deque<Stream> StreamList;

    /**
    * Track results.
    */
    struct StreamRes {
        StreamList streams;
    };

    int IsLive(std::string &name);

    Client(Config::Ptr config);

    virtual ~Client() = default;

    /**
     * Get the track list for a query
     */
    virtual StreamRes streams(const std::string &query, const bool &s_thumbnail, const std::string &s_results);

    /**
     * Cancel any pending queries (this method can be called from a different thread)
     */
    virtual void cancel();

    virtual Config::Ptr config();

protected:
    void get(const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             QJsonDocument &root);
    /**
     * Progress callback that allows the query to cancel pending HTTP requests.
     */
    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    /**
     * Hang onto the configuration information
     */
    Config::Ptr config_;

    /**
     * Thread-safe cancelled flag
     */
    std::atomic<bool> cancelled_;
};

#endif // CLIENT_H_

