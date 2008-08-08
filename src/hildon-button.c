/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
 * Contact: Karl Lattimer <karl.lattimer@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 */

/**
 * SECTION:hildon-button
 * @short_description: Widget representing a button in the Hildon framework.
 *
 * The #HildonButton is a GTK widget which represents a clickable
 * button. It is derived from the GtkButton widget and provides
 * additional commodities specific to the Hildon framework.
 *
 * The height of a #HildonButton can be set to either "finger" height
 * or "thumb" height. It can also be configured to use halfscreen or
 * fullscreen width. Alternatively, either dimension can be set to
 * "auto" so it behaves like a standard GtkButton.
 *
 * The #HildonButton can hold any valid child widget, but it usually
 * contains two labels, named title and value. To change the alignment
 * of the labels, use gtk_button_set_alignment()
 */

#include                                        "hildon-button.h"
#include                                        "hildon-enum-types.h"

G_DEFINE_TYPE                                   (HildonButton, hildon_button, GTK_TYPE_BUTTON);

#define                                         HILDON_BUTTON_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                HILDON_TYPE_BUTTON, HildonButtonPrivate));

typedef struct                                  _HildonButtonPrivate HildonButtonPrivate;

struct                                          _HildonButtonPrivate
{
    GtkLabel *title;
    GtkLabel *value;
    GtkBox *hbox;
    GtkWidget *label_box;
    GtkWidget *alignment;
    GtkWidget *image;
    GtkPositionType image_position;
};

enum {
    PROP_TITLE = 1,
    PROP_VALUE,
    PROP_SIZE,
    PROP_ARRANGEMENT
};

static void
hildon_button_set_arrangement                   (HildonButton            *button,
                                                 HildonButtonArrangement  arrangement);

static void
hildon_button_construct_child                   (HildonButton *button);

static void
hildon_button_set_property                      (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    HildonButton *button = HILDON_BUTTON (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        hildon_button_set_title (button, g_value_get_string (value));
        break;
    case PROP_VALUE:
        hildon_button_set_value (button, g_value_get_string (value));
        break;
    case PROP_SIZE:
        hildon_gtk_widget_set_theme_size (GTK_WIDGET (button), g_value_get_flags (value));
        break;
    case PROP_ARRANGEMENT:
        hildon_button_set_arrangement (button, g_value_get_enum (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
hildon_button_get_property                      (GObject    *object,
                                                 guint       prop_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
    HildonButton *button = HILDON_BUTTON (object);
    HildonButtonPrivate *priv = HILDON_BUTTON_GET_PRIVATE (button);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_value_set_string (value, gtk_label_get_text (priv->title));
        break;
    case PROP_VALUE:
        g_value_set_string (value, gtk_label_get_text (priv->value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
hildon_button_class_init                        (HildonButtonClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

    gobject_class->set_property = hildon_button_set_property;
    gobject_class->get_property = hildon_button_get_property;

    g_object_class_install_property (
        gobject_class,
        PROP_TITLE,
        g_param_spec_string (
            "title",
            "Title",
            "Text of the title label inside the button",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (
        gobject_class,
        PROP_VALUE,
        g_param_spec_string (
            "value",
            "Value",
            "Text of the value label inside the button",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (
        gobject_class,
        PROP_SIZE,
        g_param_spec_flags (
            "size",
            "Size",
            "Size request for the button",
            HILDON_TYPE_SIZE_TYPE,
            HILDON_SIZE_AUTO,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_ARRANGEMENT,
        g_param_spec_enum (
            "arrangement",
            "Arrangement",
            "How the button contents must be arranged",
            HILDON_TYPE_BUTTON_ARRANGEMENT,
            HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "horizontal-spacing",
            "Horizontal spacing between labels",
            "Horizontal spacing between the title and value labels, when in horizontal mode",
            0, G_MAXUINT, 25,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "vertical-spacing",
            "Vertical spacing between labels",
            "Vertical spacing between the title and value labels, when in vertical mode",
            0, G_MAXUINT, 5,
            G_PARAM_READABLE));

    g_type_class_add_private (klass, sizeof (HildonButtonPrivate));
}

static void
hildon_button_init                              (HildonButton *self)
{
    HildonButtonPrivate *priv = HILDON_BUTTON_GET_PRIVATE (self);

    priv->title = GTK_LABEL (gtk_label_new (NULL));
    priv->value = GTK_LABEL (gtk_label_new (NULL));
    priv->alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    priv->image = NULL;
    priv->image_position = GTK_POS_LEFT;
    priv->hbox = NULL;
    priv->label_box = NULL;

    gtk_widget_set_name (GTK_WIDGET (priv->title), "hildon-button-title");
    gtk_widget_set_name (GTK_WIDGET (priv->value), "hildon-button-value");

    gtk_misc_set_alignment (GTK_MISC (priv->title), 0, 0.5);
    gtk_misc_set_alignment (GTK_MISC (priv->value), 0, 0.5);

    /* The labels are not shown automatically, see hildon_button_set_(title|value) */
    gtk_widget_set_no_show_all (GTK_WIDGET (priv->title), TRUE);
    gtk_widget_set_no_show_all (GTK_WIDGET (priv->value), TRUE);
}

/**
 * hildon_button_add_title_size_group:
 * @button: a #HildonButton
 * @size_group: A #GtkSizeGroup for the button title (main label)
 *
 * Adds the title label of @button to @size_group.
 **/
void
hildon_button_add_title_size_group              (HildonButton *button,
                                                 GtkSizeGroup *size_group)
{
    HildonButtonPrivate *priv;

    g_return_if_fail (HILDON_IS_BUTTON (button));
    g_return_if_fail (GTK_IS_SIZE_GROUP (size_group));

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    gtk_size_group_add_widget (size_group, GTK_WIDGET (priv->title));
}

/**
 * hildon_button_add_value_size_group:
 * @button: a #HildonButton
 * @size_group: A #GtkSizeGroup for the button value (secondary label)
 *
 * Adds the value label of @button to @size_group.
 **/
void
hildon_button_add_value_size_group              (HildonButton *button,
                                                 GtkSizeGroup *size_group)
{
    HildonButtonPrivate *priv;

    g_return_if_fail (HILDON_IS_BUTTON (button));
    g_return_if_fail (GTK_IS_SIZE_GROUP (size_group));

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    gtk_size_group_add_widget (size_group, GTK_WIDGET (priv->value));
}

/**
 * hildon_button_add_image_size_group:
 * @button: a #HildonButton
 * @size_group: A #GtkSizeGroup for the button image
 *
 * Adds the image of @button to @size_group. You must add an image
 * using hildon_button_set_image before calling this function.
 **/
void
hildon_button_add_image_size_group              (HildonButton *button,
                                                 GtkSizeGroup *size_group)
{
    HildonButtonPrivate *priv;

    g_return_if_fail (HILDON_IS_BUTTON (button));
    g_return_if_fail (GTK_IS_SIZE_GROUP (size_group));
    g_return_if_fail (GTK_IS_WIDGET (priv->image));

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    gtk_size_group_add_widget (size_group, GTK_WIDGET (priv->image));
}

/**
 * hildon_button_add_size_groups:
 * @button: a #HildonButton
 * @title_size_group: A #GtkSizeGroup for the button title (main label), or %NULL
 * @value_size_group: A #GtkSizeGroup group for the button value (secondary label), or %NULL
 * @image_size_group: A #GtkSizeGroup group for the button image, or %NULL
 *
 * Convenience function to add title, value and image to size
 * groups. %NULL size groups will be ignored.
 **/
void
hildon_button_add_size_groups                   (HildonButton *button,
                                                 GtkSizeGroup *title_size_group,
                                                 GtkSizeGroup *value_size_group,
                                                 GtkSizeGroup *image_size_group)
{
    if (title_size_group)
        hildon_button_add_title_size_group (button, title_size_group);

    if (value_size_group)
        hildon_button_add_value_size_group (button, value_size_group);

    if (image_size_group)
        hildon_button_add_image_size_group (button, image_size_group);
}

/**
 * hildon_button_new:
 * @size: Flags to set the size of the button.
 * @arrangement: How the labels must be arranged.
 *
 * Creates a new #HildonButton. To add a child widget use gtk_container_add().
 *
 * Returns: a new #HildonButton
 **/
GtkWidget *
hildon_button_new                               (HildonSizeType          size,
                                                 HildonButtonArrangement arrangement)
{
    return hildon_button_new_with_text (size, arrangement, NULL, NULL);
}

/**
 * hildon_button_new_with_text:
 * @size: Flags to set the size of the button.
 * @arrangement: How the labels must be arranged.
 * @title: Title of the button (main label), or %NULL
 * @value: Value of the button (secondary label), or %NULL
 *
 * Creates a new #HildonButton with two labels, @title and @value.
 *
 * If you just don't want to use one of the labels, set it to
 * %NULL. You can set it to a non-%NULL value at any time later.
 *
 * Returns: a new #HildonButton
 **/
GtkWidget *
hildon_button_new_with_text                     (HildonSizeType           size,
                                                 HildonButtonArrangement  arrangement,
                                                 const gchar             *title,
                                                 const gchar             *value)
{
    GtkWidget *button;

    /* Create widget */
    button = g_object_new (HILDON_TYPE_BUTTON,
                           "size", size,
                           "title", title,
                           "value", value,
                           "arrangement", arrangement,
                           NULL);

    return button;
}

static void
hildon_button_set_arrangement                   (HildonButton            *button,
                                                 HildonButtonArrangement  arrangement)
{
    HildonButtonPrivate *priv;
    guint horizontal_spacing;
    guint vertical_spacing;

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    /* Pack everything */
    gtk_widget_style_get (GTK_WIDGET (button),
                          "horizontal-spacing", &horizontal_spacing,
                          "vertical-spacing", &vertical_spacing,
                          NULL);

    if (arrangement == HILDON_BUTTON_ARRANGEMENT_VERTICAL) {
        priv->label_box = gtk_vbox_new (FALSE, vertical_spacing);
    } else {
        priv->label_box = gtk_hbox_new (FALSE, horizontal_spacing);
    }

    gtk_box_pack_start (GTK_BOX (priv->label_box), GTK_WIDGET (priv->title), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (priv->label_box), GTK_WIDGET (priv->value), FALSE, FALSE, 0);

    hildon_button_construct_child (button);
}

/**
 * hildon_button_set_title:
 * @button: a #HildonButton
 * @title: a new title (main label) for the button, or %NULL
 *
 * Sets the title (main label) of @button to @title.
 *
 * This will clear the previously set title.
 *
 * If @title is set to %NULL, the title label will be hidden and the
 * value label will be realigned.
 **/
void
hildon_button_set_title                         (HildonButton *button,
                                                 const gchar  *title)
{
    HildonButtonPrivate *priv;

    g_return_if_fail (HILDON_IS_BUTTON (button));

    priv = HILDON_BUTTON_GET_PRIVATE (button);
    gtk_label_set_text (priv->title, title);

    /* If the button has no title, hide the label so the value is
     * properly aligned */
    if (title) {
        hildon_button_construct_child (button);
        gtk_widget_show (GTK_WIDGET (priv->title));
    } else {
        gtk_widget_hide (GTK_WIDGET (priv->title));
    }

    g_object_notify (G_OBJECT (button), "title");
}

/**
 * hildon_button_set_value:
 * @button: a #HildonButton
 * @value: a new value (secondary label) for the button, or %NULL
 *
 * Sets the value (secondary label) of @button to @value.
 *
 * This will clear the previously set value.
 *
 * If @value is set to %NULL, the value label will be hidden and the
 * title label will be realigned.
 *
 **/
void
hildon_button_set_value                         (HildonButton *button,
                                                 const gchar  *value)
{
    HildonButtonPrivate *priv;

    g_return_if_fail (HILDON_IS_BUTTON (button));

    priv = HILDON_BUTTON_GET_PRIVATE (button);
    gtk_label_set_text (priv->value, value);

    /* If the button has no value, hide the label so the title is
     * properly aligned */
    if (value) {
        hildon_button_construct_child (button);
        gtk_widget_show (GTK_WIDGET (priv->value));
    } else {
        gtk_widget_hide (GTK_WIDGET (priv->value));
    }

    g_object_notify (G_OBJECT (button), "value");
}

/**
 * hildon_button_get_title:
 * @button: a #HildonButton
 *
 * Gets the text from the main label (title) of @button, or %NULL if
 * none has been set.
 *
 * Returns: The text of the title label. This string is owned by the
 * widget and must not be modified or freed.
 **/
const gchar *
hildon_button_get_title                         (HildonButton *button)
{
    HildonButtonPrivate *priv;

    g_return_val_if_fail (HILDON_IS_BUTTON (button), NULL);

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    return gtk_label_get_text (priv->title);
}

/**
 * hildon_button_get_value:
 * @button: a #HildonButton
 *
 * Gets the text from the secondary label (value) of @button, or %NULL
 * if none has been set.
 *
 * Returns: The text of the value label. This string is owned by the
 * widget and must not be modified or freed.
 **/
const gchar *
hildon_button_get_value                         (HildonButton *button)
{
    HildonButtonPrivate *priv;

    g_return_val_if_fail (HILDON_IS_BUTTON (button), NULL);

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    return gtk_label_get_text (priv->value);
}

/**
 * hildon_button_set_text:
 * @button: a #HildonButton
 * @title: new text for the button title (main label)
 * @value: new text for the button value (secondary label)
 *
 * Convenience function to change both labels of a #HildonButton
 **/
void
hildon_button_set_text                          (HildonButton *button,
                                                 const gchar  *title,
                                                 const gchar  *value)
{
    hildon_button_set_title (button, title);
    hildon_button_set_value (button, value);
}

/**
 * hildon_button_set_image:
 * @button: a #HildonButton
 * @image: a widget to set as the button image
 *
 * Sets the image of @button to the given widget. The previous image
 * (if any) will be removed.
 **/
void
hildon_button_set_image                         (HildonButton *button,
                                                 GtkWidget    *image)
{
    HildonButtonPrivate *priv;

    g_return_if_fail (HILDON_IS_BUTTON (button));
    g_return_if_fail (!image || GTK_IS_WIDGET (image));

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    /* Return if there's nothing to do */
    if (image == priv->image)
        return;

    if (priv->image && priv->image->parent)
        gtk_container_remove (GTK_CONTAINER (priv->image->parent), priv->image);

    priv->image = image;

    hildon_button_construct_child (button);
}

/**
 * hildon_button_set_image_position:
 * @button: a #HildonButton
 * @position: the position of the image (%GTK_POS_LEFT or %GTK_POS_RIGHT)
 *
 * Sets the position of the image inside @button. Only left and right
 * are supported.
 **/
void
hildon_button_set_image_position                (HildonButton    *button,
                                                 GtkPositionType  position)
{
    HildonButtonPrivate *priv;

    g_return_if_fail (HILDON_IS_BUTTON (button));
    g_return_if_fail (position == GTK_POS_LEFT || position == GTK_POS_RIGHT);

    priv = HILDON_BUTTON_GET_PRIVATE (button);

    /* Return if there's nothing to do */
    if (priv->image_position == position)
        return;

    priv->image_position = position;

    hildon_button_construct_child (button);
}

static void
hildon_button_construct_child                   (HildonButton *button)
{
    HildonButtonPrivate *priv = HILDON_BUTTON_GET_PRIVATE (button);
    GtkWidget *child;

    /* Don't do anything if the button is not constructed yet */
    if (priv->label_box == NULL)
        return;

    /* Save a ref to the image if necessary */
    if (priv->image && priv->image->parent != NULL) {
        g_object_ref (priv->image);
        gtk_container_remove (GTK_CONTAINER (priv->image->parent), priv->image);
    }

    /* Save a ref to the label box if necessary */
    if (priv->label_box->parent != NULL) {
        g_object_ref (priv->label_box);
        gtk_container_remove (GTK_CONTAINER (priv->label_box->parent), priv->label_box);
    }

    /* Remove the child from the container and add priv->alignment */
    child = gtk_bin_get_child (GTK_BIN (button));
    if (child != NULL && child != priv->alignment) {
        gtk_container_remove (GTK_CONTAINER (button), child);
        child = NULL;
    }

    if (child == NULL) {
        gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (priv->alignment));
    }

    /* Create a new hbox */
    if (priv->hbox) {
        gtk_container_remove (GTK_CONTAINER (priv->alignment), GTK_WIDGET (priv->hbox));
    }
    priv->hbox = GTK_BOX (gtk_hbox_new (FALSE, 10));
    gtk_container_add (GTK_CONTAINER (priv->alignment), GTK_WIDGET (priv->hbox));

    /* Pack the image and the alignment in the new hbox */
    if (priv->image && priv->image_position == GTK_POS_LEFT)
        gtk_box_pack_start (priv->hbox, priv->image, FALSE, FALSE, 0);

    gtk_box_pack_start (priv->hbox, priv->label_box, TRUE, TRUE, 0);

    if (priv->image && priv->image_position == GTK_POS_RIGHT)
        gtk_box_pack_start (priv->hbox, priv->image, FALSE, FALSE, 0);

    /* Show everything */
    gtk_widget_show_all (GTK_WIDGET (priv->alignment));
}
