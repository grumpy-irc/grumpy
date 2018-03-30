// This is a sample extension to GrumpyChat
// You can load this using command /grumpy.script /path/to/file.js
// Use this file as a reference to functions available

// These functions are all mandatory

function cmd(window_id, text)
{
    grumpy_log("Hello world :)");
    return 0;
}

function cmd_op(window_id, text)
{
    if (grumpy_scrollback_get_type(window_id) != "channel")
    {
        grumpy_error_log("You can only use this command in channel windows");
        return 1;
    }
    grumpy_network_send_raw(window_id, "PRIVMSG ChanServ :op " + grumpy_scrollback_get_target(window_id));
    return 0;
}

function cmd_deop(window_id, text)
{
    if (grumpy_scrollback_get_type(window_id) != "channel")
    {
        grumpy_error_log("You can only use this command in channel windows");
        return 1;
    }
    grumpy_network_send_raw(window_id, "MODE " + grumpy_scrollback_get_target(window_id) + " -o " + grumpy_network_get_nick(window_id));
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
    safe_cmd_reg("deop", "cmd_deop");
    safe_cmd_reg("op", "cmd_op");
    return true;
}

function ext_is_working()
{
    return true;
}

function ext_get_name()
{
    return "Sample extension";
}

function ext_get_desc()
{
    return "This is just a sample extension, it doesn't do much useful";
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
