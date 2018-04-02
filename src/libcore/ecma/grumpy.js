// This is an internal library for use with ECMA

function grumpy_ecma_loaded()
{
    return true;
}

function grumpy_ecma_version()
{
    return "1.0.0";
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

