/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version. or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * SECTION:hildon-picker-dialog
 * @short_description: A utility widget that shows a #HildonTouchSelector widget
 *
 * HildonPickerDialog is a utility widget that shows a #HildonTouchSelector widget in
 * a new dialog (a #HildonDialog)
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <libintl.h>

#include "hildon-touch-selector.h"
#include "hildon-picker-dialog.h"

#define _(String)  dgettext("hildon-libs", String)

#define HILDON_PICKER_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_TYPE_PICKER_DIALOG, HildonPickerDialogPrivate))

G_DEFINE_TYPE (HildonPickerDialog, hildon_picker_dialog, HILDON_TYPE_DIALOG)

#define HILDON_TOUCH_SELECTOR_HEIGHT            320

struct _HildonPickerDialogPrivate
{
  GtkWidget *selector;
  GtkWidget *button;

  gulong signal_changed_id;
  gulong signal_columns_changed_id;

  GSList *current_selection;
};

/* properties */
enum
{
  PROP_0,
  PROP_DONE_BUTTON_TEXT,
  PROP_LAST
};

enum
{
  RESPONSE,
  LAST_SIGNAL
};

#define DEFAULT_DONE_BUTTON_TEXT        _("wdgt_bd_done")

static void
hildon_picker_dialog_set_property               (GObject * object,
                                                 guint prop_id,
                                                 const GValue * value,
                                                 GParamSpec * pspec);

static void
hildon_picker_dialog_get_property               (GObject * object,
                                                 guint prop_id,
                                                 GValue * value, GParamSpec * pspec);

static void
hildon_picker_dialog_finalize                   (GObject *object);

/* gtkwidget */
static void
hildon_picker_dialog_show                       (GtkWidget *widget);

/* private functions */
static gboolean
requires_done_button                            (HildonPickerDialog * dialog);

static void
setup_interaction_mode                          (HildonPickerDialog * dialog);

static void
_select_on_selector_changed_cb                  (HildonTouchSelector * dialog,
                                                 gint column,
                                                 gpointer data);

static gboolean
_hildon_picker_dialog_set_selector              (HildonPickerDialog * dialog,
                                                 HildonTouchSelector * selector);

static void
_update_title_on_selector_changed_cb            (HildonTouchSelector * selector,
                                                 gint column,
                                                 gpointer data);

static void
_on_dialog_response                             (GtkDialog *dialog,
                                                 gint response_id,
                                                 gpointer data);

static void
_save_current_selection                         (HildonPickerDialog *dialog);

static void
_restore_current_selection                      (HildonPickerDialog *dialog);

static void
_clean_current_selection                        (HildonPickerDialog *dialog);

/**********************************************************************************/

static void
hildon_picker_dialog_class_init (HildonPickerDialogClass * class)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  gobject_class = (GObjectClass *) class;
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  container_class = (GtkContainerClass *) class;

  /* GObject */
  gobject_class->set_property = hildon_picker_dialog_set_property;
  gobject_class->get_property = hildon_picker_dialog_get_property;
  gobject_class->finalize = hildon_picker_dialog_finalize;

  /* GtkWidget */
  widget_class->show = hildon_picker_dialog_show;

  /* HildonPickerDialog */
  class->set_selector = _hildon_picker_dialog_set_selector;

  /* signals */

  /* properties */
  /**
   * HildonPickerDialog
   *
   * Button label
   *
   * Since: 2.2
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DONE_BUTTON_TEXT,
                                   g_param_spec_string ("done-button-text",
                                                        "Done Button Label",
                                                        "Done Button Label",
                                                        DEFAULT_DONE_BUTTON_TEXT,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT));

  g_type_class_add_private (object_class, sizeof (HildonPickerDialogPrivate));
}


static void
hildon_picker_dialog_init (HildonPickerDialog * dialog)
{
  dialog->priv = HILDON_PICKER_DIALOG_GET_PRIVATE (dialog);

  dialog->priv->selector = NULL;
  dialog->priv->button =
    hildon_dialog_add_button (HILDON_DIALOG (dialog), "", GTK_RESPONSE_OK);
  gtk_widget_grab_default (dialog->priv->button);

  dialog->priv->signal_changed_id = 0;
  dialog->priv->signal_columns_changed_id = 0;

  dialog->priv->current_selection = NULL;

  g_signal_connect (G_OBJECT (dialog),
                    "response", G_CALLBACK (_on_dialog_response),
                    NULL);
}


static void
hildon_picker_dialog_set_property (GObject * object,
                                   guint param_id,
                                   const GValue * value, GParamSpec * pspec)
{
  switch (param_id) {
  case PROP_DONE_BUTTON_TEXT:
    hildon_picker_dialog_set_done_label (HILDON_PICKER_DIALOG (object),
                                         g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    break;
  }
}

static void
hildon_picker_dialog_get_property (GObject * object,
                                   guint param_id,
                                   GValue * value, GParamSpec * pspec)
{
  HildonPickerDialog *dialog;

  dialog = HILDON_PICKER_DIALOG (object);

  switch (param_id) {
  case PROP_DONE_BUTTON_TEXT:
    g_value_set_string (value, hildon_picker_dialog_get_done_label (dialog));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    break;
  }
}

static void
hildon_picker_dialog_finalize (GObject *object)
{
  _clean_current_selection (HILDON_PICKER_DIALOG (object));

  G_OBJECT_CLASS (hildon_picker_dialog_parent_class)->finalize (object);
}

static void
hildon_picker_dialog_show                       (GtkWidget *widget)
{
  _save_current_selection (HILDON_PICKER_DIALOG (widget));

  GTK_WIDGET_CLASS (hildon_picker_dialog_parent_class)->show (widget);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */
static void
_select_on_selector_changed_cb (HildonTouchSelector * selector,
                                gint column, gpointer data)
{
  HildonPickerDialog *dialog = NULL;

  g_return_if_fail (HILDON_IS_PICKER_DIALOG (data));

  dialog = HILDON_PICKER_DIALOG (data);

  gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
}

static void
_update_title_on_selector_changed_cb (HildonTouchSelector * selector,
                                      gint column, gpointer data)
{
  HildonPickerDialog *dialog = NULL;
  gchar *new_title = NULL;

  g_return_if_fail (HILDON_IS_PICKER_DIALOG (data));

  dialog = HILDON_PICKER_DIALOG (data);

  new_title = hildon_touch_selector_get_current_text (selector);

  gtk_window_set_title (GTK_WINDOW (dialog), new_title);

  g_free (new_title);
}

static void
_on_dialog_response                             (GtkDialog *dialog,
                                                 gint response_id,
                                                 gpointer data)
{
  if (response_id == GTK_RESPONSE_DELETE_EVENT) {
    _restore_current_selection (HILDON_PICKER_DIALOG (dialog));
  }
}

static void
on_selector_columns_changed (HildonTouchSelector * selector, gpointer userdata)
{
  HildonPickerDialog * dialog;

  dialog = HILDON_PICKER_DIALOG (userdata);

  setup_interaction_mode (dialog);
}

/**
 * hildon_picker_dialog_set_done_label:
 * @dialog: 
 * @label: 
 *
 * Since: 2.2
 **/
void
hildon_picker_dialog_set_done_label (HildonPickerDialog * dialog,
                                     const gchar * label)
{
  HildonPickerDialogPrivate *priv;

  g_return_if_fail (HILDON_IS_PICKER_DIALOG (dialog));
  g_return_if_fail (label != NULL);

  priv = HILDON_PICKER_DIALOG_GET_PRIVATE (dialog);

  gtk_button_set_label (GTK_BUTTON (priv->button), label);
}

/**
 * hildon_picker_dialog_get_done_label:
 * @dialog: 
 *
 * 
 *
 * Returns: 
 *
 * Since: 2.2
 **/
const gchar *
hildon_picker_dialog_get_done_label (HildonPickerDialog * dialog)
{
  HildonPickerDialogPrivate *priv;

  g_return_val_if_fail (HILDON_IS_PICKER_DIALOG (dialog), NULL);

  priv = HILDON_PICKER_DIALOG_GET_PRIVATE (dialog);

  return gtk_button_get_label (GTK_BUTTON (priv->button));
}

static void
_clean_current_selection (HildonPickerDialog *dialog)
{
  if (dialog->priv->current_selection) {
    g_slist_foreach (dialog->priv->current_selection,
                     (GFunc) (gtk_tree_path_free), NULL);
    g_slist_free (dialog->priv->current_selection);
    dialog->priv->current_selection = NULL;
  }
}

static void
_save_current_selection (HildonPickerDialog *dialog)
{
  HildonTouchSelector *selector = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  gint i = -1;
  GtkTreePath *path = NULL;

  selector = HILDON_TOUCH_SELECTOR (dialog->priv->selector);

  _clean_current_selection (dialog);

  for (i = 0; i  < hildon_touch_selector_get_num_columns (selector); i++) {
    hildon_touch_selector_get_selected (selector, i, &iter);
    model = hildon_touch_selector_get_model (selector, i);

    path = gtk_tree_model_get_path (model, &iter);
    dialog->priv->current_selection
      = g_slist_append (dialog->priv->current_selection, path);
  }
}

static void
_restore_current_selection (HildonPickerDialog *dialog)
{
  GSList *iter = NULL;
  GSList *current_selection = NULL;
  GtkTreePath *current_path = NULL;
  HildonTouchSelector *selector = NULL;
  gint i = -1;
  GtkTreeModel *model = NULL;
  GtkTreeIter tree_iter;

  if (dialog->priv->current_selection) {
    current_selection = dialog->priv->current_selection;
    selector = HILDON_TOUCH_SELECTOR (dialog->priv->selector);

    if (hildon_touch_selector_get_num_columns (selector) !=
        g_slist_length (current_selection)) {
      /* We conclude that if the current selection has the same
         numbers of columns that the selector, all this ok
         Anyway this shouldn't happen. */
      g_critical ("Trying to restore the selection on a selector after change"
                  " the number of columns. Are you removing columns while the"
                  " dialog is open?");
      return;
    }
    i = 0;
    for (iter = current_selection; iter; iter = g_slist_next (iter),i++) {
      current_path = (GtkTreePath *) (iter->data);
      model = hildon_touch_selector_get_model (selector, i);

      gtk_tree_model_get_iter (model, &tree_iter, current_path);

      hildon_touch_selector_select_iter (selector, i, &tree_iter, FALSE);
    }
  }
}


static gboolean
requires_done_button (HildonPickerDialog * dialog)
{
  return hildon_touch_selector_has_multiple_selection
    (HILDON_TOUCH_SELECTOR (dialog->priv->selector));
}

static void
setup_interaction_mode (HildonPickerDialog * dialog)
{
  if (dialog->priv->signal_changed_id) {
    g_signal_handler_disconnect (dialog->priv->selector,
                                 dialog->priv->signal_changed_id);
  }

  if (requires_done_button (dialog)) {
    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), TRUE);
    gtk_widget_show (GTK_DIALOG (dialog)->action_area);
        /* update the title */
    dialog->priv->signal_changed_id =
      g_signal_connect (G_OBJECT (dialog->priv->selector), "changed",
                        G_CALLBACK (_update_title_on_selector_changed_cb),
                        dialog);
  } else {
    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
    gtk_widget_hide (GTK_DIALOG (dialog)->action_area);
    dialog->priv->signal_changed_id =
      g_signal_connect (G_OBJECT (dialog->priv->selector), "changed",
                        G_CALLBACK (_select_on_selector_changed_cb), dialog);
  }
}

/*------------------------- PUBLIC METHODS ---------------------------- */

/**
 * hildon_picker_dialog_new:
 * @parent: the parent window
 *
 * Creates a new #HildonPickerDialog
 *
 * Returns: a new #HildonPickerDialog
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_picker_dialog_new (GtkWindow * parent)
{
  GtkDialog *dialog = NULL;

  dialog = g_object_new (HILDON_TYPE_PICKER_DIALOG, NULL);

  if (parent) {
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
  }

  return GTK_WIDGET (dialog);
}


static gboolean
_hildon_picker_dialog_set_selector (HildonPickerDialog * dialog,
                                    HildonTouchSelector * selector)
{
  if (dialog->priv->selector != NULL) {
    gtk_container_remove (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
                          dialog->priv->selector);
    g_object_unref (dialog->priv->selector);
    gtk_widget_set_size_request (GTK_WIDGET (dialog->priv->selector), -1,
                                 HILDON_TOUCH_SELECTOR_HEIGHT);
    dialog->priv->selector = NULL;
  }

  dialog->priv->selector = GTK_WIDGET (selector);

  if (dialog->priv->selector != NULL) {
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                        dialog->priv->selector, TRUE, TRUE, 0);

   /* NOTE: this is a little hackish, but required at this moment
      in order to ensure a correct height visualization */
    gtk_widget_set_size_request (GTK_WIDGET (dialog->priv->selector), -1,
                                 HILDON_TOUCH_SELECTOR_HEIGHT);

    gtk_widget_show (dialog->priv->selector);
    g_object_ref (dialog->priv->selector);
  }

  setup_interaction_mode (dialog);

  if (dialog->priv->signal_columns_changed_id) {
     g_signal_handler_disconnect (dialog->priv->selector,
                                  dialog->priv->signal_columns_changed_id);
   }

  dialog->priv->signal_columns_changed_id = g_signal_connect (G_OBJECT (dialog->priv->selector),
                                                              "columns-changed",
                                                              G_CALLBACK (on_selector_columns_changed), dialog);
  return TRUE;
}

/**
 * hildon_picker_dialog_set_selector:
 * @dialog: 
 * @selector: 
 *
 * 
 *
 * Returns: 
 *
 * Since: 2.2
 **/
gboolean
hildon_picker_dialog_set_selector (HildonPickerDialog * dialog,
                                   HildonTouchSelector * selector)
{
  g_return_val_if_fail (HILDON_IS_PICKER_DIALOG (dialog), FALSE);
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), FALSE);

  return HILDON_PICKER_DIALOG_GET_CLASS (dialog)->set_selector (dialog, selector);
}

/**
 * hildon_picker_dialog_get_selector:
 * @dialog: 
 *
 * 
 *
 * Returns: 
 *
 * Since: 2.2
 **/
HildonTouchSelector *
hildon_picker_dialog_get_selector (HildonPickerDialog * dialog)
{
  g_return_val_if_fail (HILDON_IS_PICKER_DIALOG (dialog), NULL);

  return HILDON_TOUCH_SELECTOR (dialog->priv->selector);
}
