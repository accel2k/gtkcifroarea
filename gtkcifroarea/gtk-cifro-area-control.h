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

#ifndef __GTK_CIFRO_AREA_CONTROL_H__
#define __GTK_CIFRO_AREA_CONTROL_H__

#include <gtk-cifro-area.h>

G_BEGIN_DECLS

/**
 * GtkCifroAreaScrollMode:
 * @GTK_CIFRO_AREA_SCROLL_MODE_DISABLED: Обработка отключена.
 * @GTK_CIFRO_AREA_SCROLL_MODE_ZOOM: Режим масштабирования.
 * @GTK_CIFRO_AREA_SCROLL_MODE_COMBINED: Комбинированный режим.
 *
 * Определяет режим обработки прокрутки колёсика мышки
 *
 * В режиме масштабирования (@GTK_CIFRO_AREA_SCROLL_MODE_ZOOM) прокрутка колёсика
 * приводит к изменению масштаба при нажатой клавише Ctrl, Alt или Shift. При нажатой
 * клавише Ctrl изменяется масштаб по оси X, Alt - по оси Y, Shift - по обеим осям.
 *
 * В комбинированном режиме (@GTK_CIFRO_AREA_SCROLL_MODE_COMBINED) прокрутка колёсика
 * без нажатия клавиш приводит к перемещению изображения по оси Y, с нажатой клавишей
 * Ctrl к прокрутке по оси Y, Shift к масштабированию по обеим осям, Alt к повороту.
 *
 */
typedef enum
{
  GTK_CIFRO_AREA_SCROLL_MODE_DISABLED,
  GTK_CIFRO_AREA_SCROLL_MODE_ZOOM,
  GTK_CIFRO_AREA_SCROLL_MODE_COMBINED
} GtkCifroAreaScrollMode;

#define GTK_TYPE_CIFRO_AREA_CONTROL             (gtk_cifro_area_control_get_type ())
#define GTK_CIFRO_AREA_CONTROL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CIFRO_AREA_CONTROL, GtkCifroAreaControl))
#define GTK_IS_CIFRO_AREA_CONTROL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CIFRO_AREA_CONTROL))
#define GTK_CIFRO_AREA_CLASS_CONTROL(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CIFRO_AREA_CONTROL, GtkCifroAreaControlClass))
#define GTK_IS_CIFRO_AREA_CLASS_CONTROL(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CIFRO_AREA_CONTROL))
#define GTK_CIFRO_AREA_GET_CLASS_CONTROL(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CIFRO_AREA_CONTROL, GtkCifroAreaControlClass))

typedef struct _GtkCifroAreaControl GtkCifroAreaControl;
typedef struct _GtkCifroAreaControlPrivate GtkCifroAreaControlPrivate;
typedef struct _GtkCifroAreaControlClass GtkCifroAreaControlClass;

struct _GtkCifroAreaControl
{
  GtkCifroArea parent_instance;

  GtkCifroAreaControlPrivate *priv;
};

struct _GtkCifroAreaControlClass
{
  GtkCifroAreaClass parent_class;
};

GTK_CIFROAREA_EXPORT
GType                  gtk_cifro_area_control_get_type         (void);

GTK_CIFROAREA_EXPORT
GtkWidget             *gtk_cifro_area_control_new              (void);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_control_set_scroll_mode  (GtkCifroAreaControl    *control,
                                                                GtkCifroAreaScrollMode  scroll_mode);

GTK_CIFROAREA_EXPORT
GtkCifroAreaScrollMode gtk_cifro_area_control_get_scroll_mode  (GtkCifroAreaControl    *control);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_control_set_move_step    (GtkCifroAreaControl    *control,
                                                                gdouble                 move_step);

GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_control_get_move_step    (GtkCifroAreaControl    *control);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_control_set_rotate_step  (GtkCifroAreaControl    *control,
                                                                gdouble                 rotate_step);

GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_control_get_rotate_step  (GtkCifroAreaControl    *control);

G_END_DECLS

#endif /* __GTK_CIFRO_AREA_CONTROL_H__ */
