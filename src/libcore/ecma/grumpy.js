// This is an internal library for use with ECMA

var grumpy_system_window_id = 1;

function grumpy_ecma_loaded()
{
    return true;
}

function grumpy_ecma_version()
{
    return "1.0.0";
}

function grumpy_ecma_print_help()
{
    grumpy_log("ECMA " + grumpy_ecma_version());
    var function_help = grumpy_get_function_list();
    function_help.sort();
    var i, len;
    for (i = 0, len = function_help.length; i < len; i++)
    {
        grumpy_log(function_help[i] + grumpy_get_function_help(function_help[i]));
    }
    grumpy_log("alert(text): display alert text");
    grumpy_log("grumpy_ecma_loaded(): returns true if ecma lib is present");
    grumpy_log("grumpy_ecma_version(): returns version string of ecma lib");
    grumpy_log("grumpy_ecma_print_help(): show this help");
    function_help = grumpy_get_hook_list();
    function_help.sort();
    grumpy_log("Hooks:");
    for (i = 0, len = function_help.length; i < len; i++)
    {
        grumpy_log(function_help[i] + grumpy_get_function_help(function_help[i]));
    }
}

function alert(text)
{
    if (grumpy_get_context() === "GrumpyChat")
    {
        return grumpy_ui_message_box("ecma-generic-alert", "Alert", text);
    } else
    {
        return grumpy_error_log(text);
    }
}

function console() {}
console.assert = function(eval, txt) { if (eval) { grumpy_log(txt); } }
console.log = function(txt) { grumpy_log(txt); }
console.error = function(txt) { grumpy_error_log(txt); }
console.debug = function(txt) { grumpy_debug_log(txt, 1); }
console.warn = function(txt) { grumpy_log("WARNING: " + txt); }
