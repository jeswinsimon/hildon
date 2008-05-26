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

#ifndef                                         __HILDON_APP_MENU_H__
#define                                         __HILDON_APP_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define                                         HILDON_TYPE_APP_MENU \
                                                (hildon_app_menu_get_type())

#define                                         HILDON_APP_MENU(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_APP_MENU, HildonAppMenu))

#define                                         HILDON_APP_MENU_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_APP_MENU, HildonAppMenuClass))

#define                                         HILDON_IS_APP_MENU(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HILDON_TYPE_APP_MENU))

#define                                         HILDON_IS_APP_MENU_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_APP_MENU))

#define                                         HILDON_APP_MENU_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_APP_MENU, HildonAppMenuClass))

typedef struct                                  _HildonAppMenu HildonAppMenu;

typedef struct                                  _HildonAppMenuClass HildonAppMenuClass;

struct                                          _HildonAppMenuClass
{
    GtkWindowClass parent_class;
};

struct                                          _HildonAppMenu
{
    GtkWindow parent;
};

GType
hildon_app_menu_get_type                        (void) G_GNUC_CONST;

GtkWidget *
hildon_app_menu_new                             (void);

void
hildon_app_menu_append                          (HildonAppMenu *menu,
                                                 GtkButton *item);

GtkWidget *
hildon_app_menu_add_filter                      (HildonAppMenu *menu,
                                                 GtkButton *filter,
                                                 GtkWidget *group);

GtkWidget *
hildon_app_menu_get_group_from_filter           (HildonAppMenu *menu,
                                                 GtkButton *filter);

void
hildon_app_menu_popup                           (HildonAppMenu *menu);

G_END_DECLS

#endif /* __HILDON_APP_MENU_H__ */
