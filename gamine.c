 /* Gamine - GnunuX 19/12/10
  * This program is free software. It comes without any warranty, to
  * the extent permitted by applicable law. You can redistribute it
  * and/or modify it under the terms of the Do What The Fuck You Want
  * To Public License, Version 2, as published by Sam Hocevar. See
  * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <gtk/gtk.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <gdk/gdkx.h>
#include <math.h>
#include <time.h>
#include <gst/gst.h>
#include <gdk/gdkkeysyms.h>
#include <gconf/gconf-client.h>

//gettext
#include <libintl.h>
#include <locale.h>


typedef struct {
    GstElement *elt;
    gboolean repeat;
} closure_t;

typedef struct {
    GstBus *bus;
    GtkWidget *da;
    GConfClient* gc;
    cairo_surface_t *surface;
    cairo_t *cr;
    gboolean *is_cairo;
} gamine_t;

gdouble xold;
gdouble yold;
gint linewidth;
gint objectweight;
gint fontweight;

static void
load_conf (gamine_t *cb)
{
    linewidth = gconf_client_get_int(cb->gc, "/apps/gamine/line_width", NULL);
    if ( linewidth == 0 )
        linewidth = 10;
    objectweight = gconf_client_get_int(cb->gc, "/apps/gamine/object_weight", 
        NULL);
    if ( objectweight == 0 )
        objectweight = 15;
    fontweight = gconf_client_get_int(cb->gc, "/apps/gamine/font_weight", NULL);
    if ( fontweight == 0 )
        fontweight = 50;
}

static void
eos_message_received (GstBus * bus, GstMessage * message, closure_t *closure)
{
    if (closure->repeat == TRUE)
    {
        gst_element_set_state (GST_ELEMENT(closure->elt), GST_STATE_NULL);
        gst_element_set_state (GST_ELEMENT(closure->elt), GST_STATE_PLAYING);
    } else {
        gst_element_set_state (GST_ELEMENT(closure->elt), GST_STATE_NULL);
        gst_object_unref (GST_OBJECT(closure->elt));
        closure->elt = NULL;
    }
}

static void
gst_play_background (GstBus *bus, gchar *filesnd, gboolean repeat)
{
    gchar *filename;
    GstElement *pipeline;
    pipeline = gst_element_factory_make("playbin", "playbin");
    if (pipeline != NULL)
    {
        closure_t *closure = g_new (closure_t, 1);
        closure->elt = pipeline;
        closure->repeat = repeat;
        bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
        gst_bus_add_signal_watch_full (bus, G_PRIORITY_HIGH);
        g_signal_connect (bus, "message::eos", 
            (GCallback) eos_message_received, closure);
        gst_object_unref (bus);
        filename = g_build_filename(DATADIR, "sounds", filesnd, NULL);
        if (!g_file_test (filename, G_FILE_TEST_EXISTS))
            printf(gettext("** error: %s does not exists\n"), filename);
        else {
            filename = g_strdup_printf("file://%s", filename);
            g_object_set (G_OBJECT(pipeline), "uri", filename, NULL);
            gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_PLAYING);
        }
        g_free(filename);
    }
}

static cairo_t *
get_cairo_context (gamine_t *cb)
{
    Display *dpy;
    Drawable xid;
    Visual *visual;
    GdkDrawable *drawable;
    gint x_offset, y_offset;
    gint width, height;
    cairo_surface_t *surface;
    cairo_t *context;
    if (cb->is_cairo)
        return cb->cr;
    //if (GDK_IS_WINDOW (window)) {
    gdk_window_get_internal_paint_info ((cb->da)->window, &drawable,&x_offset,
            &y_offset);
    //} else {
    //    drawable = window;
    //};
    dpy = gdk_x11_drawable_get_xdisplay (drawable);
    xid = gdk_x11_drawable_get_xid (drawable);
    gdk_drawable_get_size (drawable, &width, &height);
    visual = GDK_VISUAL_XVISUAL(gdk_drawable_get_visual (drawable));
    //create an xlib surface 
    surface = cairo_xlib_surface_create (dpy, xid, visual,
        width, height);
    //create context
    context = cairo_create (surface);
    //cairo_surface_destroy (surface);
    
    //if (GDK_IS_WINDOW (window))
    cairo_translate (context, -x_offset, -y_offset);
    cb->is_cairo = TRUE;
    cb->cr = context;
    cb->surface = surface;
    return context;
}

static void
play_random_sound (GstBus *bus)
{
    static gchar *tab[] = {"bonus.wav", "crash.wav", "eat.wav", "flip.wav",
        "level.wav", "plick.ogg", "tri.ogg", "brick.wav", "darken.wav",
        "eraser1.wav", "gobble.wav", "line_end.wav", "prompt.wav", "tuxok.wav",
        "bleep.wav", "bubble.wav", "drip.wav", "eraser2.wav", "grow.wav",
        "paint1.wav", "receive.wav", "youcannot.wav"};
    gint snd = rand() % (sizeof tab / sizeof *tab);
    gst_play_background (bus, tab[snd], FALSE);
}

static void
draw_line (GtkWidget      *widget,
    GdkEventButton *event,
    gamine_t *cb)
{
    cairo_t *drawingline;
    //for the first draw
    if (! xold)
    {
        xold=event->x+1;
        yold=event->y;
    } else {
        float red = ((rand() % 128) + 127) /255.0;
        float green = ((rand() % 128) + 127) /255.0;
        float blue = ((rand() % 128) + 127) /255.0;
        drawingline = get_cairo_context(cb);
        cairo_new_path (drawingline);
        cairo_set_source_rgba (drawingline, red, green, blue, 0.5);
        cairo_set_line_width (drawingline, linewidth);
        //move the cursor
        cairo_move_to (drawingline, xold, yold);
        //line for old cursor to new one's
        cairo_line_to (drawingline, event->x+1, event->y);
        //draw
        cairo_stroke (drawingline);
        xold=event->x+1;
        yold=event->y;
    }
}

static void
draw_string (
        gamine_t *cb,
        gint cx, 
        gint cy, 
        gchar *str)
{
    cairo_t *cr;
    gfloat red = random() % 10 * 0.1;
    gfloat green = random() % 10 * 0.1;
    gfloat blue = random() % 10 * 0.1;
    play_random_sound(cb->bus);
    cr = get_cairo_context(cb);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, fontweight);
    cairo_move_to (cr, cx - 15, cy + 15);
    cairo_set_source_rgb (cr, red, green, blue);
    cairo_show_text (cr, str);
    cairo_stroke (cr);
}

static void
draw_star (GtkWidget    *widget,
       GdkEventButton *event,
       gamine_t *cb)
{
    const gint spike_count = random() % 6 + 2;
    const gint inner_radius = objectweight;
    const gint outer_radius = 20;
    gfloat red = random() % 10 * 0.1;
    gfloat green = random() % 10 * 0.1;
    gfloat blue = random() % 10 * 0.1;
    cairo_t *cr;
    gdouble x, y;
    int i;
    cr = get_cairo_context(cb);
    play_random_sound(cb->bus);
    
    cairo_set_source_rgb (cr, red, green, blue);
    cairo_new_path (cr);
    for (i = 0; i < spike_count + 1; i++) {
        x = event->x + cos ((i * 2) * M_PI / spike_count) * inner_radius;
        y = event->y + sin ((i * 2) * M_PI / spike_count) * inner_radius;
        if (i == 0)
            cairo_move_to (cr, x, y);
        else
            cairo_line_to (cr, x, y);
    
        x = event->x + cos ((i * 2 + 1) * M_PI / spike_count) * outer_radius;
        y = event->y + sin ((i * 2 + 1) * M_PI / spike_count) * outer_radius;
    
        cairo_line_to (cr, x, y);
    }
    cairo_fill (cr);
}

static void
save_picture(gamine_t *cb)
{
    struct stat st;
    gchar *dirname;
    gchar *filename;
    char buf[20];
    time_t timestamp;
    struct tm * t;
    timestamp = time(NULL);
    t = localtime(&timestamp);

    dirname = g_build_filename(g_get_home_dir(), "gamine", NULL);
    //if dirname not exists
    if(stat(dirname,&st) != 0)
        if (mkdir(dirname, 0750) < 0)
            printf(gettext("*** error: failed to create directory %s***\n"),
                    dirname);
    sprintf(buf,"%d-%d-%d_%d-%d-%d.png",1900 + t->tm_year,
            t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
    filename = g_build_filename(dirname, buf, NULL);
    if (cairo_surface_write_to_png(cb->surface, filename) < 0)
        printf(gettext("*** error: failed to create file %s***\n"), filename);
}

static void
gamine_on_key (GtkWidget *pWidget,
    GdkEventKey* pKey,
    gamine_t *cb)
{
    gint mx, my;
    if (pKey->type == GDK_KEY_PRESS)
    {
        switch (pKey->keyval)
        {
            case GDK_Escape :
                gtk_main_quit ();
            break;
            case GDK_space :
                gtk_widget_queue_draw(pWidget);
            break;
            case GDK_Print:
                save_picture(cb);
            break;
            default:
                gdk_window_get_pointer(pWidget->window, &mx, &my, NULL);
                draw_string(cb, mx, my, pKey->string);
            break;
        }
    };
}

static void
display_help (GtkWidget      *widget,
       GdkEventExpose *eev,
       gpointer        data,
       gamine_t cb)
{
    gint height;
    cairo_t *cr;
    height = widget->allocation.height - 5;
    cr = gdk_cairo_create (widget->window);
    cb.cr = cr;
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 15);
    cairo_move_to (cr, 0, height);
    cairo_set_source_rgb (cr, 0,0,0);
    cairo_show_text (cr, gettext("Quit: esc | Clear: space | Save: printscr"));
    cairo_stroke (cr);
    cairo_destroy(cr);
}

gint
main (gint argc, gchar *argv[])
{
    Window root;
    //gettext
    bindtextdomain( "gamine", LOCALDIR );
    textdomain( "gamine" );
    gamine_t cb;
    GtkWidget *window;
    GdkWindow *gdkwindow;
    GtkWindow *gtkwindow;
    GdkScreen *screen;
    GdkPixbuf *cursor_pixbuf;
    GdkPixbuf *icon_pixbuf;
    GdkCursor *cursor;
    GdkColor bg_color;
    gchar *cursorfile;
    gchar *iconfile;

    cb.is_cairo = FALSE;
    gtk_init (&argc, &argv);
    gst_init (&argc, &argv);
    gconf_init(argc, argv, NULL);
    gst_play_background (cb.bus,
              "BachJSBrandenburgConcertNo2inFMajorBWV1047mvmt1.ogg", TRUE);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
/* Create the drawing area and configuration */
    cb.da = gtk_drawing_area_new ();
    bg_color.red   = 65535;
    bg_color.green = 65535;
    bg_color.blue  = 65535;
    gtk_widget_modify_bg (cb.da, GTK_STATE_NORMAL, &bg_color);
    gtk_container_add (GTK_CONTAINER (window), cb.da);
    cb.gc = gconf_client_get_default();

    gtkwindow = GTK_WINDOW(window);
    gtk_window_set_title (gtkwindow, "Gamine");
    gtk_window_set_wmclass(gtkwindow, "gamine", "Gamine");
    gtk_container_set_border_width (GTK_CONTAINER (gtkwindow), 0);


/* Event signals */
    g_signal_connect (gtkwindow, "destroy",
         G_CALLBACK (gtk_main_quit), &gtkwindow);
    g_signal_connect (G_OBJECT (cb.da), "expose-event",
        G_CALLBACK (display_help), &cb);
    g_signal_connect (cb.da, "motion_notify_event",
        G_CALLBACK (draw_line), &cb);
    g_signal_connect (cb.da, "button_press_event",
        G_CALLBACK (draw_star), &cb);
    g_signal_connect (gtkwindow, "key-press-event",
        G_CALLBACK (gamine_on_key), &cb);
    gtk_widget_set_events (cb.da, gtk_widget_get_events (cb.da)
        | GDK_LEAVE_NOTIFY_MASK
        | GDK_BUTTON_PRESS_MASK
        | GDK_BUTTON_RELEASE_MASK
        | GDK_POINTER_MOTION_MASK
        | GDK_POINTER_MOTION_HINT_MASK);
/* Set fullscreen, grab mouse/keyboard, ...*/
    gtk_widget_show_all (GTK_WIDGET(gtkwindow));
    gdkwindow = GDK_WINDOW(GTK_WIDGET(gtkwindow)->window);
    screen = gtk_widget_get_screen (cb.da);
    gtk_window_present (gtkwindow);
    gtk_window_stick(gtkwindow);

    //gtk_window_set_keep_above (gtkwindow), True);
    //gtk_window_set_transient_for (gtkwindow, NULL);
    //set fullscreen
    gdk_window_fullscreen (gdkwindow);
    gtk_window_fullscreen (gtkwindow);
    gdk_window_raise (gdkwindow);
    //set full screen without window manager
    XMoveResizeWindow(GDK_WINDOW_XDISPLAY(gdkwindow), GDK_WINDOW_XID(gdkwindow),
        0, 0, gdk_screen_get_width (screen), gdk_screen_get_height (screen));
    root = DefaultRootWindow(GDK_WINDOW_XDISPLAY (gdkwindow));
    XGrabPointer(GDK_WINDOW_XDISPLAY (gdkwindow), root, True, PointerMotionMask,
        GrabModeAsync, GrabModeAsync, root, None, CurrentTime); 
    XGrabKeyboard(GDK_WINDOW_XDISPLAY (gdkwindow), root, True,
                    GrabModeAsync, GrabModeAsync, CurrentTime);
    //remove keyboard repeat
    XAutoRepeatOff(GDK_WINDOW_XDISPLAY (gdkwindow));
    gtk_window_has_toplevel_focus (gtkwindow);
/*cursor*/
    cursorfile = g_build_filename(DATADIR, "pencil.png", NULL);
    if (!g_file_test (cursorfile, G_FILE_TEST_EXISTS)) {
        printf(gettext("*** error: %s does not exists***\n"), cursorfile);
    } else {
        cursor_pixbuf = gdk_pixbuf_new_from_file(cursorfile, NULL);
        cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(),
            cursor_pixbuf, 0, 38);
        gdk_window_set_cursor(gdkwindow, cursor);
        gdk_cursor_unref(cursor);
        gdk_pixbuf_unref(cursor_pixbuf);
    }
    g_free(cursorfile);
/*Set icon*/
    iconfile = g_build_filename(DATADIR, "gamine.png", NULL);
    if (!g_file_test (iconfile, G_FILE_TEST_EXISTS))
        printf(gettext("*** error: %s does not exists***\n"), iconfile);
    else {
        icon_pixbuf = gdk_pixbuf_new_from_file(iconfile, NULL);
        gtk_window_set_icon (gtkwindow, icon_pixbuf);
        gdk_pixbuf_unref (icon_pixbuf);
    }
    g_free(iconfile);

    load_conf(&cb);
    gtk_main ();
    //set keyboard repeat
    XAutoRepeatOn(GDK_WINDOW_XDISPLAY (gdkwindow));
    XCloseDisplay(GDK_WINDOW_XDISPLAY (gdkwindow));
    return 0;
}

