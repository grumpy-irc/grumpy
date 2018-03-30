// This extension preserves a history of each scrollback by its name
// which is actually pretty bad way to do it.

function ext_init()
{
    return true;
}

function ext_is_working()
{
    return true;
}

function ext_get_name()
{
    return "history_store";
}

function ext_get_desc()
{
    return "Preserves the history of windows by their name";
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
