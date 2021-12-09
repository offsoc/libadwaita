#include "adw-demo-page-avatar.h"

#include <glib/gi18n.h>

struct _AdwDemoPageAvatar
{
  AdwBin parent_instance;

  AdwAvatar *avatar;
  GtkEntry *text;
  GtkLabel *file_chooser_label;
  GtkListBox *contacts;
};

G_DEFINE_TYPE (AdwDemoPageAvatar, adw_demo_page_avatar, ADW_TYPE_BIN)

static char *
create_random_name (void)
{
  static const char *first_names[] = {
    "Adam",
    "Adrian",
    "Anna",
    "Charlotte",
    "Frédérique",
    "Ilaria",
    "Jakub",
    "Jennyfer",
    "Julia",
    "Justin",
    "Mario",
    "Miriam",
    "Mohamed",
    "Nourimane",
    "Owen",
    "Peter",
    "Petra",
    "Rachid",
    "Rebecca",
    "Sarah",
    "Thibault",
    "Wolfgang",
  };
  static const char *last_names[] = {
    "Bailey",
    "Berat",
    "Chen",
    "Farquharson",
    "Ferber",
    "Franco",
    "Galinier",
    "Han",
    "Lawrence",
    "Lepied",
    "Lopez",
    "Mariotti",
    "Rossi",
    "Urasawa",
    "Zwickelman",
  };

  return g_strdup_printf ("%s %s",
                          first_names[g_random_int_range (0, G_N_ELEMENTS (first_names))],
                          last_names[g_random_int_range (0, G_N_ELEMENTS (last_names))]);
}

static void
populate_contacts (AdwDemoPageAvatar *self)
{
  for (int i = 0; i < 30; i++) {
    g_autofree char *name = create_random_name ();
    GtkWidget *contact = adw_action_row_new ();
    GtkWidget *avatar = adw_avatar_new (40, name, TRUE);

    gtk_widget_set_margin_top (avatar, 12);
    gtk_widget_set_margin_bottom (avatar, 12);

    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (contact), name);
    adw_action_row_add_prefix (ADW_ACTION_ROW (contact), avatar);
    gtk_list_box_append (self->contacts, contact);
  }
}

static void
open_response_cb (AdwDemoPageAvatar *self,
                  GtkResponseType    response,
                  GtkFileChooser    *chooser)
{
  if (response == GTK_RESPONSE_ACCEPT) {
    g_autoptr (GFile) file = gtk_file_chooser_get_file (chooser);
    g_autoptr (GFileInfo) info = NULL;
    g_autoptr (GdkTexture) texture = NULL;
    g_autoptr (GError) error = NULL;

    info = g_file_query_info (file,
                              G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              NULL);

    if (info)
      gtk_label_set_label (self->file_chooser_label,
                           g_file_info_get_display_name (info));

    gtk_widget_action_set_enabled (GTK_WIDGET (self), "avatar.remove", TRUE);

    texture = gdk_texture_new_from_file (file, &error);
    if (error)
      g_critical ("Failed to create texture from file: %s", error->message);

    adw_avatar_set_custom_image (self->avatar, texture ? GDK_PAINTABLE (texture) : NULL);
  }

  gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (chooser));
}

static void
avatar_open_cb (AdwDemoPageAvatar *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  GtkFileChooserNative *chooser =
    gtk_file_chooser_native_new (_("Select an Avatar"),
                                 GTK_WINDOW (root),
                                 GTK_FILE_CHOOSER_ACTION_OPEN,
                                 _("_Select"),
                                 _("_Cancel"));
  gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (chooser), TRUE);

  g_signal_connect_swapped (chooser, "response", G_CALLBACK (open_response_cb), self);

  gtk_native_dialog_show (GTK_NATIVE_DIALOG (chooser));
}

static void
avatar_remove_cb (AdwDemoPageAvatar *self)
{
  gtk_label_set_label (self->file_chooser_label, _("(None)"));
  gtk_widget_action_set_enabled (GTK_WIDGET (self), "avatar.remove", FALSE);
  adw_avatar_set_custom_image (self->avatar, NULL);
}

static void
save_response_cb (AdwDemoPageAvatar *self,
                  GtkResponseType    response,
                  GtkFileChooser    *chooser)
{
  if (response == GTK_RESPONSE_ACCEPT) {
    g_autoptr (GFile) file = gtk_file_chooser_get_file (chooser);
    g_autoptr (GdkTexture) texture =
      adw_avatar_draw_to_texture (self->avatar,
                                  gtk_widget_get_scale_factor (GTK_WIDGET (self)));

    gdk_texture_save_to_png (texture, g_file_peek_path (file));
  }

  gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (chooser));
}

static void
avatar_save_cb (AdwDemoPageAvatar *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  GtkFileChooserNative *chooser =
    gtk_file_chooser_native_new (_("Save Avatar"),
                                 GTK_WINDOW (root),
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 _("_Save"),
                                 _("_Cancel"));
  gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (chooser), TRUE);

  g_signal_connect_swapped (chooser, "response", G_CALLBACK (save_response_cb), self);

  gtk_native_dialog_show (GTK_NATIVE_DIALOG (chooser));
}

static void
adw_demo_page_avatar_class_init (AdwDemoPageAvatarClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/avatar/adw-demo-page-avatar.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageAvatar, avatar);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageAvatar, text);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageAvatar, file_chooser_label);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageAvatar, contacts);

  gtk_widget_class_install_action (widget_class, "avatar.open", NULL, (GtkWidgetActionActivateFunc) avatar_open_cb);
  gtk_widget_class_install_action (widget_class, "avatar.remove", NULL, (GtkWidgetActionActivateFunc) avatar_remove_cb);
  gtk_widget_class_install_action (widget_class, "avatar.save", NULL, (GtkWidgetActionActivateFunc) avatar_save_cb);
}

static void
adw_demo_page_avatar_init (AdwDemoPageAvatar *self)
{
  g_autofree char *name = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  name = create_random_name ();
  gtk_editable_set_text (GTK_EDITABLE (self->text), name);

  populate_contacts (self);
  avatar_remove_cb (self);
}