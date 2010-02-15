#include <gtk/gtk.h>
#include "def.h"
#include "interface.h"

static gboolean debug = FALSE;
static gboolean cli_mode = FALSE;

static GOptionEntry arguments[] =
{
    {
        "debug", 0, 0, G_OPTION_ARG_NONE, &debug,
        "toggle debug mode (for developing)", NULL
    },
    {
        "cli_mode", 0, 0, G_OPTION_ARG_NONE, &cli_mode,
        "disable GUI (command line only)", NULL
    },
    { NULL }
};

int main(int argc, const char *argv[])
{
    GOptionContext *context;
    GError *error = NULL;

    /* initialization of game objects */
    app app =
    {
        /* lists */
        NULL, NULL, NULL, NULL,
        /* pixmaps and widgets */
        NULL, NULL, NULL, NULL, NULL,
        /* game state */
        LOADING,
        /* re player */
        NULL,
        /* integers */
        2, -1 , -1, 1, 1,
        /* hand/null game */
        FALSE, FALSE,
        /* options */
        TRUE,
        /* player names */
        NULL
    };

    /* parse command line arguments */
    context = g_option_context_new(" - GTK Skat");

    if (context)
    {
        g_option_context_add_main_entries(context, arguments, NULL);
        g_option_context_add_group(context, gtk_get_option_group(TRUE));

        if (!g_option_context_parse(context, &argc, (gchar ***) &argv, &error))
        {
            g_printerr("failed to parse arguments: %s\n", error->message);
            return 1;
        }

        g_option_context_free(context);

        /* toggle gui if desired */
        if (cli_mode)
            app.gui = FALSE;

        /* load configuration */
        load_config(&app);

        /* initialize interface */
        create_interface(&app);

        /* show all widgets after being initialized */
        if (app.allwidgets != NULL)
        {
            gtk_widget_show_all(app.allwidgets[0]);
            gtk_main();

            free_app(&app);
        }
    }

    return 0;
}

/* vim:set et sw=4 ts=4 tw=80: */
