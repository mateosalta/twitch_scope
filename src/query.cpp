#include <query.h>
#include <localization.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace sc = unity::scopes;

using namespace std;

/**
 * Define the larger "current weather" layout.
 *
 * The icons are larger.
 */

const static string STREAMS_TEMPLATE =
    R"(
        {
            "schema-version": 1,
            "template": {
                "category-layout": "grid",
                "card-layout": "vertical"
            },
            "components": {
                "title": "title",
                "art" : {
                    "field": "art"
                },
                "subtitle": "name"
            }
        }
    )";

const static string CHANNELS_TEMPLATE =
    R"(
        {
            "schema-version": 1,
            "template": {
                "category-layout": "horizontal-list",
                "card-layout": "horizontal"
            },
            "components": {
                "title": "title",
                "art" : {
                    "field": "art"
                },
                "subtitle": "name"
            }
        }
    )";

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             Client::Config::Ptr config) :
    sc::SearchQueryBase(query, metadata), client_(config) {
}

void Query::cancelled() {
    client_.cancel();
}

void Query::run(sc::SearchReplyProxy const& reply) {
    try {

        auto s_thumbnail = settings().at("thumbnail").get_bool();
        auto s_results = settings().at("results").get_string();
        auto s_channels = settings().at("channels").get_bool();

        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Get the query string
        string query_string = query.query_string();

        Client::StreamRes streamslist;
        Client::ChannelRes channelslist;

        if (query_string.empty()) {
            // If the string is empty, provide a default one
            streamslist = client_.streams("development", s_thumbnail, s_results);
            if (s_channels) {
                channelslist = client_.channels("development", s_results);
            }
        } else {
            // otherwise, use the query string
            streamslist = client_.streams(query_string, s_thumbnail, s_results);

            if (s_channels) {
                channelslist = client_.channels(query_string, s_results);
            }
        }

        // Register a category for channels and streams
        auto streams_cat = reply->register_category("streams", "Currently live!", "",
            sc::CategoryRenderer(STREAMS_TEMPLATE));
        // register_category(arbitrary category id, header title, header icon, template)

        if (s_channels) {

            auto channels_cat = reply->register_category("channels", "Channels", "",
                sc::CategoryRenderer(CHANNELS_TEMPLATE));

            for (const auto &stream : channelslist.channels) {

                // Iterate over the trackslist
                sc::CategorisedResult res(channels_cat);

                // We must have a URI
                res.set_uri(stream.url);

                // We also need the track title
                res.set_title(stream.name);

                // This takes care of non-existent custom art
                std::string art404 = "http://static-cdn.jtvnw.net/jtv_user_pictures/xarth/404_user_150x150.png";
                if (stream.logo != ""){
                    res.set_art(stream.logo);
                }
                else {
                    res.set_art(art404);
                }

                // Set the rest of the attributes, art, artist, etc
                res["title"] = stream.name;

                res["name"] = stream.channel.user;

                res["url"] = stream.url;

                res["game"] = stream.game;

                res["thumb"] = stream.thumbnail;

                // Push the result
                if (!reply->push(res)) {
                    // If we fail to push, it means the query has been cancelled.
                    // So don't continue;
                    return;
                }
            }
        }

        for (const auto &stream : streamslist.streams) {
            // Iterate over the trackslist
            sc::CategorisedResult res(streams_cat);

            // We must have a URI
            res.set_uri(stream.url);

            // We also need the track title
            res.set_title(stream.name);

            // This takes care of non-existent custom art
            std::string art404 = "http://static-cdn.jtvnw.net/jtv_user_pictures/xarth/404_user_150x150.png";

            std::cout << stream.logo << std::endl;

            if (stream.logo != ""){
                res.set_art(stream.logo);
            }
            else {
                res.set_art(art404);
            }

            // Set the rest of the attributes, art, artist, etc
            res["title"] = stream.name;

            res["name"] = stream.channel.user;

            res["url"] = stream.url;

            res["mature"] = "Mature: " + stream.mature;

            res["viewers"] = "Viewers: " + stream.viewers;

            res["game"] = stream.game;

            res["thumb"] = stream.thumbnail;

            // Push the result
            if (!reply->push(res)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
        }

    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}

