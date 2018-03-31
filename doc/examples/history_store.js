// This extension preserves a history of each scrollback by its name
// which is actually pretty bad way to do it.

//////////////////////////////////////////
// Configuration
// How many items to remember, note this will only affect saving of history, not the size of actual history in GrumpyChat
var history_max = 20;
// Save only commands
var only_command = false;
//////////////////////////////////////////

//////////////////////////////////////////
//////////////////////////////////////////
// Code starts here
// This is where we store history for every single scrollback
var global_history = [];

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

function get_win(id)
{
    // Here we construct the name of key, because there might be multiple same channels on different networks we prefix each window with network
    name = "";
    if (grumpy_scrollback_has_network(id))
    {
        name = grumpy_network_get_network_name(id) + "_";
    }
    name = name + grumpy_scrollback_get_target(id);
    if (name == "")
        name = "system";
    return name;
}

function ext_ui_on_exit()
{
    for (var key in global_history)
    {
        // Split the items so that we can count them
        list = global_history[key].split("\n");
        // Remove extra items
        while (list.length > history_max)
            list.shift()
        // Turn it back to one huge string
        temp = "";
        for (var i = 0, len = list.length; i < len; i++)
            temp += list[i] + "\n";
        // Remove the extra newline
        temp = temp.substring(0, temp.length - 1);
        grumpy_set_cfg(key, temp);
    }
}

function ext_ui_on_history(window_id, text)
{
    window_name = get_win(window_id);
    if (!(window_name in global_history))
    {
        global_history[window_name] = text;
    } else
    {
        global_history[window_name] += "\n" + text;
    }
}

function refresh_hist(window_id)
{
    window_name = get_win(window_id);
    if (window_name in global_history)
        return;
    // try to get a history for that window
    hist = grumpy_get_cfg(window_name, "");
    if (hist == "")
        return;
    // store to memory
    ext_ui_on_history(window_id, hist);  
    grumpy_ui_load_history(window_id, hist);
}

function ext_ui_on_window_switch(window_id)
{
    refresh_hist(window_id);
}

function ext_ui_on_main_window_start()
{
    // System window is always 1
    refresh_hist(1);
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
