// This extension preserves a history of each scrollback by its name
// which is actually pretty bad way to do it.

//////////////////////////////////////////
// Configuration
// How many items to remember, note this will only affect saving of history, not the size of actual history in GrumpyChat
var history_max = 20;
// Save only commands
var only_command = false;
// Print extra debug
var debugging_on = false;
//////////////////////////////////////////

//////////////////////////////////////////
//////////////////////////////////////////
// Code starts here
// This is where we store history for every single scrollback
var global_history = [];

function debug(text)
{
    if (debugging_on)
        grumpy_scrollback_write(grumpy_system_window_id, "DEBUG: " + text);
}

function get_win(id)
{
    // Here we construct the name of key, because there might be multiple same channels on different networks we prefix each window with network
    var name = "history_";
    if (grumpy_scrollback_has_network(id))
    {
        name += grumpy_network_get_network_name(id) + "_";
    }
    var target = grumpy_scrollback_get_target(id);
    if (target == "")
        target = "system";
    name += target;
    return name;
}

//! Stores the history of window to a file
function save_hist(key)
{
    debug("saving history for " + key + " to config file");
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

function ext_on_scrollback_destroyed(scrollback_id)
{
    // Scrollback was closed, so let's delete it from cache here, so that in case it's reopened, the history will reload
    var name = get_win(scrollback_id);
    if (name in global_history)
    {
        save_hist(name);
        delete global_history[name];
    }
}

function cmd_history_list(window_id, text)
{
    var name = get_win(window_id);
    if (!(name in global_history))
    {
        grumpy_error_log("No history found for this window");
        return 2;
    }
    list = global_history[name].split("\n");

    for (var i = 0, len = list.length; i < len; i++)
        grumpy_scrollback_write(window_id, list[i]);

    return 0;
}

function cmd_history_search(window_id, text)
{
    var name = get_win(window_id);
    var search_string = text.toLowerCase();
    if (search_string.length == 0)
    {
        grumpy_error_log("This command requires a parameter to search");
        return 1;
    }
    if (!(name in global_history))
    {
        grumpy_error_log("No history found for this window");
        return 2;
    }

    var list = global_history[name].split("\n");
    var something_found = false;

    for (var i = 0, len = list.length; i < len; i++)
    {
        lc = list[i].toLowerCase();
        if (lc.indexOf(search_string) !== -1)
        {
            // We got a result, let's print it
            something_found = true;
            grumpy_scrollback_write(window_id, list[i]);
        }
    }

    if (!something_found)
        grumpy_scrollback_write(window_id, "Nothing found");

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
    safe_cmd_reg("history.list", "cmd_history_list");
    safe_cmd_reg("history.search", "cmd_history_search");
    history_max = grumpy_get_cfg("history_max", history_max);
    only_command = grumpy_get_cfg("only_command", only_command);
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

function ext_ui_on_exit()
{
    for (var key in global_history)
    {
        save_hist(key);
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
    debug("loading history for " + window_name);
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
