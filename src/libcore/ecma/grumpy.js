// This is an internal library for use with ECMA

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
    for (var i = 0, len = function_help.length; i < len; i++)
    {
        grumpy_log(function_help[i] + grumpy_get_function_help(function_help[i]));
    }
    grumpy_log("alert(text): display alert text");
    grumpy_log("grumpy_ecma_loaded(): returns true if ecma lib is present");
    grumpy_log("grumpy_ecma_version(): returns version string of ecma lib");
    grumpy_log("grumpy_ecma_print_help(): show this help");
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

