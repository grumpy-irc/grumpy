// This is a sample extension to GrumpyChat
// Implements some commands for ops

function check_window(window_id)
{
    if (grumpy_scrollback.get_type(window_id) != "channel")
    {
        grumpy.error_log("You can only use this command in channel windows");
        return false;
    }
    return true;
}

function cmd_deex(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network.send_raw(window_id, "MODE " + grumpy_scrollback.get_target(window_id) + " -e *!*@" + grumpy_network.get_host(window_id));
    return 0;
}

function cmd_ex(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network.send_raw(window_id, "MODE " + grumpy_scrollback.get_target(window_id) + " +e *!*@" + grumpy_network.get_host(window_id));
    return 0;
}

function cmd_op(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network.send_raw(window_id, "PRIVMSG ChanServ :op " + grumpy_scrollback.get_target(window_id));
    return 0;
}

function cmd_deop(window_id, text)
{
    if (!check_window(window_id))
        return 1;
    grumpy_network.send_raw(window_id, "MODE " + grumpy_scrollback.get_target(window_id) + " -o " + grumpy_network.get_nick(window_id));
    return 0;
}

function cmd_optools(window_id, text)
{
    grumpy.log("* /op - gives you OP in current window");
    grumpy.log("* /deop - removes your OP in current window");
    grumpy.log("* /ex - setup ban exemption for yourself");
    grumpy.log("* /deex - removes ban exemption");
    return 0;
}

// This function try to register a command in grumpy and print error if it fails
function safe_cmd_reg(command_name, callback)
{
    if (!grumpy.register_cmd(command_name, callback))
        grumpy.error_log("Unable to register command: " + command_name);
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

function ext_get_info()
{
    var info = {};
    info["name"] = "optools";
    info["description"] = "Implements commands for ops, type /optools to list commands";
    info["version"] = "1.0.0";
    info["author"] = "Petr Bena";
    return info;
}

