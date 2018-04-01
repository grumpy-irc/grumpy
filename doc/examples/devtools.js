// Implements some developer commands

function cmd_id(window_id, text)
{
    grumpy_log("ID: " + window_id);
    return 0;
}

function cmd_network(window_id, text)
{
    grumpy_log("Name: \'" + grumpy_network_get_network_name(window_id) + "\' your nick: " + grumpy_network_get_nick(window_id));
    return 0;
}

function cmd_fc(window_id, text)
{
    grumpy_log("Function available (" + text + "()): " + grumpy_has_function(text));
    return 0;
}

function cmd_info(window_id, text)
{
    var v = grumpy_get_version();
    grumpy_log("GrumpyChat version " + v.String);
    return 0;
}

function cmd_help(window_id, text)
{
    grumpy_log("/dev.session.info - session info");
    grumpy_log("/dev.network.info - network info");
    grumpy_log("/dev.scrollback.id - print scrollback id");
    grumpy_log("/dev.has <function name> - check if this version has ECMA function");
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
    // Register new cmds
    safe_cmd_reg("devtools", "cmd_help");
    safe_cmd_reg("dev.session.info", "cmd_info");
    safe_cmd_reg("dev.scrollback.id", "cmd_id");
    safe_cmd_reg("dev.network.info", "cmd_network");
    safe_cmd_reg("dev.has", "cmd_fc");
    return true;
}

function ext_is_working()
{
    return true;
}

function ext_get_name()
{
    return "devtools";
}

function ext_get_desc()
{
    return "Implements commands for devs, type /devtools to list commands";
}

function ext_get_version()
{
    return "1.0.0";
}

function ext_get_author()
{
    return "Petr Bena";
}

// function reference
//
// void grumpy_set_cfg(key, val)                    // save a key
// bool grumpy_get_cfg(key, val)                    // retrieve a key
// bool grumpy_register_cmd(name, fc)               // register new command
// bool grumpy_debug_log(text, verbosity)           // writes to debug log
// bool grumpy_error_log(text)                      // writes to error log
// bool grumpy_log(text)                            // writes to current scrollback
// bool grumpy_network_send_raw(window_id, text)    // sends RAW data to IRC
// bool grumpy_network_send_message(window_id, target, text) // sends IRC message to channel or user
// bool grumpy_network_get_network_name(window_id)  // Network name
// bool grumpy_network_get_server_host(window_id)   // 
// bool grumpy_network_get_nick(window_id)          // Your nick
// bool grumpy_network_get_ident(window_id)         // Your ident
// bool grumpy_network_get_host(window_id)          // Your host
// bool grumpy_scrollback_has_network_session(window_id) // true if scrollback belongs to some network, if not you can't use any functions that are related to IRC or grumpyd on it
// bool grumpy_scrollback_has_network(window_id)    // true if scrollback belongs to some IRC network, if not you can't use any functions that are related to IRC on it
// bool grumpy_scrollback_get_type(window_id)       // returns channel / system / user
// bool grumpy_scrollback_get_target(window_id)     // returns target name
// bool grumpy_scrollback_write(window_id, text)    // writes to scrollback
