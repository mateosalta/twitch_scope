#include <preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>

namespace sc = unity::scopes;

using namespace std;

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    // Support three different column layouts
    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);

    auto s_thumbnail = settings().at("thumbnail").get_bool();

    // We define 3 different layouts, that will be used depending on the
    // device. The shell (view) will decide which layout fits best.
    // If, for instance, we are executing in a tablet probably the view will use
    // 2 or more columns.
    // Column layout definitions are optional.
    // However, we recommend that scopes define layouts for the best visual appearance.

    // Single column layout
    layout1col.add_column( { "image_widget", "header_widget", "text_game", "text_mature", "actions_widget" } );

    // Two column layout
    layout2col.add_column( { "image_widget" } );
    layout2col.add_column( { "header_widget", "text_game", "text_mature", "actions_widget" } );

    // Three cokumn layout
    layout3col.add_column( { "image_widget" });
    layout3col.add_column( { "header_widget", "text_game", "text_mature", "actions_widget" } );
    layout3col.add_column( { } );

    // Register the layouts we just created
    reply->register_layout( { layout1col, layout2col, layout3col } );

    // Define the image section
    sc::PreviewWidget image("image_widget", "image");
    // If the thumbnail setting is true, then ...
    if (s_thumbnail) {
        // Set the image equal to the stream thumbnail - feature
        image.add_attribute_mapping("source", "thumb");
    }
    else {
        // Set the image equal to the art - speed
        image.add_attribute_mapping("source", "art");
    }


    // Define the header section
    sc::PreviewWidget header("header_widget", "header");
    // It has a "title" and a "subtitle" property
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "subtitle");

    // Define the game/viewers section
    sc::PreviewWidget game("text_game", "text");
    game.add_attribute_mapping("title", "game");
    game.add_attribute_mapping("text", "viewers");

    // Define the mature warning section
    sc::PreviewWidget mature("text_mature", "text");
    mature.add_attribute_mapping("text", "mature");

    // Define the actions section
    sc::PreviewWidget actions("actions_widget", "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
        {"id", sc::Variant("open")},
        {"label", sc::Variant("Open")}
    });

    actions.add_attribute_value("actions", builder.end());

    // Push each of the sections
    reply->push( { image, header, game, mature, actions } );
}

