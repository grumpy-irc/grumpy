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
    grumpy.log("ECMA " + grumpy_ecma_version());
    var function_help = grumpy.get_function_list();
    function_help.sort();
    var i, len;
    for (i = 0, len = function_help.length; i < len; i++)
    {
        grumpy.log(function_help[i] + grumpy.get_function_help(function_help[i]));
    }
    grumpy.log("alert(text): display alert text");
    grumpy.log("grumpy_ecma_loaded(): returns true if ecma lib is present");
    grumpy.log("grumpy_ecma_version(): returns version string of ecma lib");
    grumpy.log("grumpy_ecma_print_help(): show this help");
    function_help = grumpy.get_hook_list();
    function_help.sort();
    grumpy.log("Hooks:");
    for (i = 0, len = function_help.length; i < len; i++)
    {
        grumpy.log(function_help[i] + grumpy.get_function_help(function_help[i]));
    }
}

function alert(text)
{
    if (grumpy.get_context() === "GrumpyChat")
    {
        return grumpy.ui.message_box("ecma-generic-alert", "Alert", text);
    } else
    {
        return grumpy.error_log(text);
    }
}

function console() {}
console.assert = function(eval, txt) { if (eval) { grumpy.log(txt); } }
console.log = function(txt) { grumpy.log(txt); }
console.error = function(txt) { grumpy.error_log(txt); }
console.debug = function(txt) { grumpy.debug_log(txt, 1); }
console.warn = function(txt) { grumpy.log("WARNING: " + txt); }
