// This is a sample extension to GrumpyChat
// Implements some commands for ops

function check_window(window_id)
{
    if (grumpy_scrollback_get_type(window_id) != "channel")
    {
        grumpy_error_log("You can only use this command in channel windows");
        return false;
    }
    return true;
}

function cmd_deex(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network_send_raw(window_id, "MODE " + grumpy_scrollback_get_target(window_id) + " -e *!*@" + grumpy_network_get_host(window_id));
    return 0;
}

function cmd_ex(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network_send_raw(window_id, "MODE " + grumpy_scrollback_get_target(window_id) + " +e *!*@" + grumpy_network_get_host(window_id));
    return 0;
}

function cmd_op(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network_send_raw(window_id, "PRIVMSG ChanServ :op " + grumpy_scrollback_get_target(window_id));
    return 0;
}

function cmd_deop(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network_send_raw(window_id, "MODE " + grumpy_scrollback_get_target(window_id) + " -o " + grumpy_network_get_nick(window_id));
    return 0;
}

function cmd_optools(window_id, text)
{
    grumpy_log("* /op - gives you OP in current window");
    grumpy_log("* /deop - removes your OP in current window");
    grumpy_log("* /ex - setup ban exemption for yourself");
    grumpy_log("* /deex - removes ban exemption");
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
    safe_cmd_reg("optools", "cmd_optools");
    safe_cmd_reg("deex", "cmd_deex");
    safe_cmd_reg("op", "cmd_op");
    safe_cmd_reg("ex", "cmd_ex");
    safe_cmd_reg("deop", "cmd_deop");
    return true;
}

function ext_is_working()
{
    return true;
}

function ext_get_name()
{
    return "optools";
}

function ext_get_desc()
{
    return "Implements commands for ops, type /optools to list commands";
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
