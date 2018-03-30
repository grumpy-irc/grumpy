// This is a sample extension to GrumpyChat
// You can load this using command /grumpy.script /path/to/file.js
// Use this file as a reference to functions available

// These functions are all mandatory

function cmd(window_id, text)
{
    grumpy_log("Hello world");
    return 0;
}

function cmd_op(window_id, text)
{
    grumpy_network_send_raw(window_id, "PRIVMSG ChanServ :op " + grumpy_scrollback_get_target(window_id));
}

function ext_init()
{
    grumpy_log("Loaded");
    // Register a new command /hello, which calls cmd()
    if (!grumpy_register_cmd("hello", "cmd"))
    {
        grumpy_error("Unable to register command :(");
    }
    grumpy_register_cmd("op", "cmd_op");
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
// bool grumpy_register_cmd(name, fc) // register new command
// bool grumpy_debug_log(text, verbosity) // writes to debug log
// bool grumpy_error_log(text) // writes to error log
// bool grumpy_log(text) // writes to current scrollback
// bool grumpy_scrollback_write(window_id, text) // writes to scrollback
// bool grumpy_scrollback_get_target(window_id) // returns target name
// bool grumpy_network_send_raw(window_id, text) // sends RAW data to IRC
