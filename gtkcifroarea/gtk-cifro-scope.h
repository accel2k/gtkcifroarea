/*
 * GtkCifroArea - 2D layers image management library.
 *
 * Copyright 2013-2017 Andrei Fadeev (andrei@webcontrol.ru)
 *
 * This file is part of GtkCifroArea.
 *
 * GtkCifroArea is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GtkCifroArea is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Alternatively, you can license this code under a commercial license.
 * Contact the author in this case.
 *
 */

#ifndef __GTK_CIFRO_SCOPE_H__
#define __GTK_CIFRO_SCOPE_H__

#include <gtk-cifro-area-control.h>

G_BEGIN_DECLS

/**
 * GtkCifroScopeGravity:
 * @GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP: Ось X - вправо, ось Y - вверх.
 * @GTK_CIFRO_SCOPE_GRAVITY_LEFT_UP: Ось X - влево,  ось Y - вверх.
 * @GTK_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN: Ось X - вправо, ось Y - вниз.
 * @GTK_CIFRO_SCOPE_GRAVITY_LEFT_DOWN: сь X - влево,  ось Y - вниз.
 * @GTK_CIFRO_SCOPE_GRAVITY_UP_RIGHT: Ось X - вверх,  ось Y - вправо.
 * @GTK_CIFRO_SCOPE_GRAVITY_UP_LEFT: Ось X - вверх,  ось Y - влево.
 * @GTK_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT: Ось X - вниз,   ось Y - вправо.
 * @GTK_CIFRO_SCOPE_GRAVITY_DOWN_LEFT: Ось X - вниз,   ось Y - влево.
 *
 * Типы ориентации осей осциллографа.
 *
 */
typedef enum
{
  GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP = 1,
  GTK_CIFRO_SCOPE_GRAVITY_LEFT_UP,
  GTK_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN,
  GTK_CIFRO_SCOPE_GRAVITY_LEFT_DOWN,
  GTK_CIFRO_SCOPE_GRAVITY_UP_RIGHT,
  GTK_CIFRO_SCOPE_GRAVITY_UP_LEFT,
  GTK_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT,
  GTK_CIFRO_SCOPE_GRAVITY_DOWN_LEFT
} GtkCifroScopeGravity;

/**
 * GtkCifroScopeDrawType:
 * @GTK_CIFRO_SCOPE_LINED: Данные отображаются линиями.
 * @GTK_CIFRO_SCOPE_DOTTED: Данные отображаются отдельными точками.
 * @GTK_CIFRO_SCOPE_DOTTED2: Данные отображаются отдельными точками увеличенного размера.
 * @GTK_CIFRO_SCOPE_DOTTED_LINE: Данные отображаются точками соединёнными линиями.
 * @GTK_CIFRO_SCOPE_CROSSED: Данные отображаются перекрестиями.
 * @GTK_CIFRO_SCOPE_CROSSED_LINE: Данные отображаются перекрестиями соединёнными линиями.
 *
 * Типы осциллограмм.
 *
 */
typedef enum
{
  GTK_CIFRO_SCOPE_LINED = 1,
  GTK_CIFRO_SCOPE_DOTTED,
  GTK_CIFRO_SCOPE_DOTTED2,
  GTK_CIFRO_SCOPE_DOTTED_LINE,
  GTK_CIFRO_SCOPE_CROSSED,
  GTK_CIFRO_SCOPE_CROSSED_LINE
} GtkCifroScopeDrawType;

#define GTK_TYPE_CIFRO_SCOPE             (gtk_cifro_scope_get_type ())
#define GTK_CIFRO_SCOPE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CIFRO_SCOPE, GtkCifroScope))
#define GTK_IS_CIFRO_SCOPE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CIFRO_SCOPE))
#define GTK_CIFRO_SCOPE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CIFRO_SCOPE, GtkCifroScopeClass))
#define GTK_IS_CIFRO_SCOPE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CIFRO_SCOPE))
#define GTK_CIFRO_SCOPE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CIFRO_SCOPE, GtkCifroScopeClass))

typedef struct _GtkCifroScope GtkCifroScope;
typedef struct _GtkCifroScopePrivate GtkCifroScopePrivate;
typedef struct _GtkCifroScopeClass GtkCifroScopeClass;

struct _GtkCifroScope
{
  GtkCifroAreaControl parent_instance;

  GtkCifroScopePrivate *priv;
};

struct _GtkCifroScopeClass
{
  GtkCifroAreaControlClass parent_class;
};

GTK_CIFROAREA_EXPORT
GType                  gtk_cifro_scope_get_type                (void);

GTK_CIFROAREA_EXPORT
GtkWidget             *gtk_cifro_scope_new                     (GtkCifroScopeGravity   gravity);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_limits              (GtkCifroScope         *cscope,
                                                                gdouble                min_x,
                                                                gdouble                max_x,
                                                                gdouble                min_y,
                                                                gdouble                max_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_scale_limits        (GtkCifroScope         *cscope,
                                                                gdouble                min_scale_x,
                                                                gdouble                max_scale_x,
                                                                gdouble                min_scale_y,
                                                                gdouble                max_scale_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_info_show           (GtkCifroScope         *cscope,
                                                                gboolean               show);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_axis_name           (GtkCifroScope         *cscope,
                                                                const gchar           *time_axis_name,
                                                                const gchar           *value_axis_name);

GTK_CIFROAREA_EXPORT
guint                  gtk_cifro_scope_add_channel             (GtkCifroScope         *cscope);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_remove_channel          (GtkCifroScope         *cscope,
                                                                guint                  channel_id);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_channel_name        (GtkCifroScope         *cscope,
                                                                guint                  channel_id,
                                                                const gchar           *axis_name);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_channel_time_param  (GtkCifroScope         *cscope,
                                                                guint                  channel_id,
                                                                gdouble                time_shift,
                                                                gdouble                time_step);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_channel_value_param (GtkCifroScope         *cscope,
                                                                guint                  channel_id,
                                                                gdouble                value_shift,
                                                                gdouble                value_scale);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_channel_draw_type   (GtkCifroScope         *cscope,
                                                                guint                  channel_id,
                                                                GtkCifroScopeDrawType  draw_type);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_channel_color       (GtkCifroScope         *cscope,
                                                                guint                  channel_id,
                                                                gdouble                red,
                                                                gdouble                green,
                                                                gdouble                blue);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_channel_data        (GtkCifroScope         *cscope,
                                                                guint                  channel_id,
                                                                guint                  n_values,
                                                                const gfloat          *values);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_scope_set_channel_show        (GtkCifroScope         *cscope,
                                                                guint                  channel_id,
                                                                gboolean               show);

G_END_DECLS

#endif /* __GTK_CIFRO_SCOPE_H__ */
