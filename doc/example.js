// This is a sample extension to GrumpyChat
// You can load this using command /grumpy.script /path/to/file.js
// See end of this file for a reference of functions available

// Hooks
// void ext_shutdown(): called on shutdown

function cmd(window_id, text)
{
    grumpy_log("Hello world :)");
    return 0;
}

function cmd_print_id(window_id, text)
{
    grumpy_scrollback_write(window_id, "ID of this scrollback: " + window_id);
    return 0;
}

// This function try to register a command in grumpy and print error if it fails
function safe_cmd_reg(command_name, callback)
{
    if (!grumpy_register_cmd(command_name, callback))
        grumpy_error_log("Unable to register command: " + command_name);
}

function ext_init()
{
    grumpy_log("Loaded");
    // Register a new command /hello, which calls cmd()
    safe_cmd_reg("hello", "cmd");
    safe_cmd_reg("debug.scrollback", "cmd_print_id");
    return true;
}

function ext_is_working()
{
    return true;
}

function ext_get_info()
{
    var info = {};
    info["name"] = "Sample extension";
    info["description"] = "This is a sample extension, it implements some channel commands and debug commands";
    info["version"] = "1.0.0";
    info["author"] = "Petr Bena";
    return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Function reference
///////////////////////////////////////////////////////////////////////////////////////////////
// You can execute grumpy_ecma_print_help() to get an up-to-date help for your current grumpy scripting engine
//
// bool grumpy.has_function(name)                   // check if function (any in this list) is available in this grumpy version
// object grumpy.get_version()                      // return version
// void grumpy.set_cfg(key, val)                    // save a key
// bool grumpy.get_cfg(key, val)                    // retrieve a key
// bool grumpy.register_cmd(name, fc)               // register new command
// bool grumpy.debug_log(text, verbosity)           // writes to debug log
// bool grumpy.error_log(text)                      // writes to error log
// bool grumpy.log(text)                            // writes to current scrollback
// bool grumpy_network.send_raw(window_id, text)    // sends RAW data to IRC
// bool grumpy_network.send_message(window_id, target, text) // sends IRC message to channel or user
// bool grumpy_network.get_network_name(window_id)  // Network name
// bool grumpy_network.get_server_host(window_id)   // 
// bool grumpy_network.get_nick(window_id)          // Your nick
// bool grumpy_network.get_ident(window_id)         // Your ident
// bool grumpy_network.get_host(window_id)          // Your host
// bool grumpy_scrollback.has_network_session(window_id) // true if scrollback belongs to some network, if not you can't use any functions that are related to IRC or grumpyd on it
// bool grumpy_scrollback.has_network(window_id)    // true if scrollback belongs to some IRC network, if not you can't use any functions that are related to IRC on it
// bool grumpy_scrollback.get_type(window_id)       // returns channel / system / user
// bool grumpy_scrollback.get_target(window_id)     // returns target name
// bool grumpy_scrollback.write(window_id, text)    // writes to scrollback
// string grumpy_ecma_version()                     // returns version of ECMA lib
//
///////////////////////////////////////////////////////////////////////////////////////////////
// Hooks
// All of these hooks needs to be registered first
///////////////////////////////////////////////////////////////////////////////////////////////
// void shutdown()                                // On shutdown of core
// void scrollback_destroyed(int scrollback_id)   // Scrollback is deleted from memory
///////////////////////////////////////////////////////////////////////////////////////////////
// Functions available with UI only
///////////////////////////////////////////////////////////////////////////////////////////////
// bool grumpy_ui.wipe_history(window_id)
// bool grumpy_ui.load_history(window_id)
// void ext_ui_on_exit()                     // On exit
// void ext_ui_on_history(window_id, text)   // When new item is added to history
// void ext_ui_on_main_window_start()        // On main
// void ext_ui_on_window_switch(window_id)   // When frame is changed
// void ext_ui_on_new_scrollback_frame(window_id) // When new scrollback frame is created
