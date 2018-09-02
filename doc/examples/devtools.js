// Implements some developer commands

function cmd_id(window_id, text)
{
    grumpy.log("ID: " + window_id);
    return 0;
}

function cmd_network(window_id, text)
{
    grumpy.log("Name: \'" + grumpy_network.get_network_name(window_id) + "\' your nick: " + grumpy_network.get_nick(window_id));
    return 0;
}

function cmd_fc(window_id, text)
{
    grumpy.log("Function available (" + text + "()): " + grumpy.has_function(text));
    return 0;
}

function cmd_reference(window_id, text)
{
    grumpy_ecma_print_help();
    return 0;
}

function cmd_info(window_id, text)
{
    var v = grumpy.get_version();
    grumpy.log("GrumpyChat version " + v.String);
    grumpy.log("ECMA lib version: " + grumpy_ecma_version());
    return 0;
}

function ext_get_info()
{
    var info = {};
    info["name"] = "devtools";
    info["description"] = "Implements commands for devs, type /devtools to list commands";
    info["version"] = "1.0.0";
    info["author"] = "Petr Bena";
    return info;
}

function cmd_list(window_id, text)
{
    var scrollbacks = grumpy_scrollback.list();
    var i, len;
    var networks = [];
    networks["null"] = "";
    for (i = 0, len = scrollbacks.length; i < len; i++)
    {
        var id = scrollbacks[i];
        var scrollback_type = grumpy_scrollback.get_type(id);
        if (scrollback_type === "system")
        {
            if (grumpy_scrollback.has_network(id))
            {
                var network_name = grumpy_network.get_network_name(id);
                if (!(network_name in networks))
                {
                    networks[network_name] = "";
                }
            } else
            {
                continue;
            }
        } else if (scrollback_type === "channel")
        {
            if (grumpy_scrollback.has_network(id))
            {
                var network_name = grumpy_network.get_network_name(id);
                networks[network_name] += grumpy_scrollback.get_target(id) + ",";
            } else
            {
                networks["null"] += grumpy_scrollback.get_target(id) + ",";
            }
        }
    }
    for (var key in networks)
    {
        grumpy.log(key + ": " + networks[key]);
    }
    return 0;
}

function cmd_scrollback_mk(window_id, text)
{
    if (text === "")
        text = "Test";
    grumpy_scrollback.create(0, text);
}

function cmd_scrollback_rm(window_id, text)
{
    grumpy_scrollback.remove(text);
}

function cmd_help(window_id, text)
{
    grumpy.log("/dev.channels - show channels per network");
    grumpy.log("/dev.session.info - session info");
    grumpy.log("/dev.network.info - network info");
    grumpy.log("/dev.scrollback.id - print scrollback id");
    grumpy.log("/dev.scrollback.make - creates a new scrollback");
    grumpy.log("/dev.scrollback.remove - deletes a scrollback");
    grumpy.log("/dev.function.help - print help for all functions");
    grumpy.log("/dev.has <function name> - check if this version has ECMA function");
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
    safe_cmd_reg("devtools", "cmd_help");
    safe_cmd_reg("dev.session.info", "cmd_info");
    safe_cmd_reg("dev.scrollback.id", "cmd_id");
    safe_cmd_reg("dev.network.info", "cmd_network");
    safe_cmd_reg("dev.function.help", "cmd_reference");
    safe_cmd_reg("dev.channels", "cmd_list");
    safe_cmd_reg("dev.scrollback.make", "cmd_scrollback_mk");
    safe_cmd_reg("dev.scrollback.remove", "cmd_scrollback_rm");
    safe_cmd_reg("dev.has", "cmd_fc");
    return true;
}

function ext_is_working()
{
    return true;
}
