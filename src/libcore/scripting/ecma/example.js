// This is a sample extension to GrumpyChat
// You can load this using command /grumpy.script /path/to/file.js
// See end of this file for a reference of functions available

// Hooks
// void ext_shutdown(): called on shutdown

function cmd(window_id, text)
{
    grumpy.log("Hello world :)");
    return 0;
}

function cmd_print_id(window_id, text)
{
    grumpy_scrollback.write(window_id, "ID of this scrollback: " + window_id);
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
    grumpy.log("Loaded");
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
    info["name"] = "sample";
    info["description"] = "This is a sample extension, it implements some channel commands and debug commands";
    info["version"] = "1.0.0";
    info["author"] = "Petr Bena";
    return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Function reference
///////////////////////////////////////////////////////////////////////////////////////////////
// grumpy.debug_log(string text): writes to debug log
// grumpy.error_log(string text): write to error log
// grumpy.get_cfg(key, default): returns stored value from ini file
// grumpy.get_context(): return execution context, either core, grumpyd or GrumpyChat (core doesn't have ui functions and hooks)
// grumpy.get_context_id(): return execution context id, either core, grumpyd or GrumpyChat
// grumpy.get_function_help(string function): return help for a function
// grumpy.get_function_list(): return list of functions
// grumpy.get_hook_list(): returns a list of all hooks
// grumpy.get_version(): returns version object with properties: Major, Minor, Revision, String
// grumpy.has_function(function_name): return true or false whether function is present
// grumpy.is_unsafe(): returns if unsafe functions are enabled
// grumpy.log(string text): writes text to system log
// grumpy.register_cmd(command, function): register new grumpy command that will execute function
// grumpy.register_hook(string hook, string function_id): creates a hook
// grumpy.set_cfg(key, value): stores value as key in settings
// grumpy.unregister_hook(string hook): removes hook
// grumpy_network.get_host(scrollback_id): return your host for network of scrollback
// grumpy_network.get_ident(scrollback_id): return your ident for network of scrollback
// grumpy_network.get_network_name(scrollback_id): return network for network of scrollback
// grumpy_network.get_nick(scrollback_id): return your nickname for network of scrollback
// grumpy_network.get_server_host(scrollback_id): return server hostname for network of scrollback
// grumpy_network.send_message(scrollback_id, target, message): sends a silent message to target for network of scrollback
// grumpy_network.send_raw(scrollback_id, text): sends RAW data to network of scrollback
// grumpy_scrollback.create(parent, name): creates a new scrollback, parent can be 0 if this should be root scrollback, returns false on error otherwise scrollback_id
// grumpy_scrollback.get_target(scrollback_id): return target name of scrollback (channel name, user name)
// grumpy_scrollback.get_type(scrollback_id): return type of scrollback; system, channel, user
// grumpy_scrollback.has_network(scrollback_id): return true if scrollback belongs to network
// grumpy_scrollback.has_network_session(scrollback_id): returns true if scrollback has existing IRC session
// grumpy_scrollback.list(): returns a list of all scrollback IDs
// grumpy_scrollback.remove(scrollback_id): destroy scrollback, can be only used for scrollbacks created with this script
// grumpy_scrollback.write(scrollback_id, text): write text
// grumpy_ui.clear_history(scrollback_id)
// grumpy_ui.load_history(scrollback_id, text)
// grumpy_ui.message_box(id, title, text)
// grumpy_unsafe.process(window_id, text): !unsafe! sends input to command processor, esentially same as entering text to input box in program
// alert(text): display alert text
// grumpy_ecma_loaded(): returns true if ecma lib is present
// grumpy_ecma_version(): returns version string of ecma lib
// grumpy_ecma_print_help(): show this help
// Hooks:
// ext_get_info(): should return version
// ext_init(): called on start, must return true, otherwise load of extension is considered as failure
// ext_is_working(): must exist and must return true, if returns false, extension is considered crashed
// ext_unload(): called when extension is being unloaded from system
// scrollback_destroyed(int scrollback_id): called when scrollback is deleted
// shutdown(): called on exit
// ui_exit(): Called on exit
// ui_history(scrollback, text): when a command is being stored to history
// ui_main(): Called when main window is loaded
// ui_scrollback_frame_created(scrollback): When scrollback is created
// ui_window_switch(scrollback): Called when window is being switched to
