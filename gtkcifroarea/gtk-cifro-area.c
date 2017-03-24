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

/**
 * SECTION: gtk-cifro-area
 * @Short_description: GTK+ виджет показа многослойных изображений
 * @Title: GtkCifroArea
 *
 * Виджет предназначен для показа многослойных изображений сформированных внешними
 * модулями. Виджет управляет логической системой координат учитывая такие параметры
 * как: масштаб, диапазон допустимых значений координат, угол поворота и зеркальное
 * отражение по осям. Для отображения данных пользователь должен отрисовать их
 * в буфер #cairo_t с учётом текущих значений координат и масштаба. Поворот изображения
 * и его зеркальное отражениевыполняется виджетом самостоятельно.
 *
 * Графическая структура виджета приведена ниже.
 *
 * |[
 *
 * +-------------------------+
 * | О      Окантовка      О |
 * | к +-----------------+ к |
 * | а |                 | а |
 * | н |                 | н |
 * | т | Видимая область | т |
 * | о |                 | о |
 * | в |                 | в |
 * | к +-----------------+ к |
 * | а      Окантовка      а |
 * +-------------------------+
 *
 * ]|
 *
 * Видимая область находится в центре изображения. Вокруг неё может иметься окантовка.
 * Видимая область может масштабироваться, поворачиваться вокруг центра изображения, зеркально
 * отражаться по обоим осям. Для видимой области отслеживаются границы (в логических координатах)
 * отображаемых данных. Размер видимой области в пикселях может превышать размер окна виджета,
 * но при отображении обрезается по границе окантовки.
 *
 * Для полноценного использования #GtkCifroArea необходимо чтобы разрабатываемый класс был
 * унаследован от #GtkCifroArea или #GtkCifroAreaControl. В нём необходимо переопределить
 * виртуальные функции базового класса #GtkCifroAreaClass и добавить обработчики сигналов
 * #GtkCifroArea::area-draw и #GtkCifroArea::visible-draw.
 *
 */

#include "gtk-cifro-area.h"

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>

enum
{
  SIGNAL_VISIBLE_DRAW,
  SIGNAL_AREA_DRAW,
  SIGNAL_LAST
};

enum
{
  SCROLL_ACTION_NONE,
  SCROLL_ACTION_MOVE_X,
  SCROLL_ACTION_MOVE_Y,
  SCROLL_ACTION_ZOOM_X,
  SCROLL_ACTION_ZOOM_Y,
  SCROLL_ACTION_ZOOM_BOTH,
  SCROLL_ACTION_ROTATE
};

struct _GtkCifroAreaPrivate
{
  cairo_t               *visible_cairo;        /* Объект для рисования в видимой области. */

  gboolean               swap_x;               /* TRUE - ось x направлена влево, FALSE - вправо. */
  gboolean               swap_y;               /* TRUE - ось y направлена вниз, FALSE - вверх. */

  gboolean               rotate;               /* Разрешение поворота. */
  gdouble                angle;                /* Угол поворота изображения в радианах. */
  gdouble                angle_cos;            /* Косинус угла поворота изображения. */
  gdouble                angle_sin;            /* Синус угла поворота изображения. */

  GtkCifroAreaStickType  stick_x;              /* Выравнивание по оси x, если видимая область больше отображаемой. */
  GtkCifroAreaStickType  stick_y;              /* Выравнивание по оси y, если видимая область больше отображаемой. */

  guint                  border_left;          /* Размер области обрамления слева. */
  guint                  border_right;         /* Размер области обрамления справа. */
  guint                  border_top;           /* Размер области обрамления сверху. */
  guint                  border_bottom;        /* Размер области обрамления снизу. */

  guint                  widget_width;         /* Ширина виджета CifroArea. */
  guint                  widget_height;        /* Высота виджета CifroArea. */
  guint                  visible_width;        /* Ширина видимой области для отрисовки данных. */
  guint                  visible_height;       /* Высота видимой области для отрисовки данных. */
  guint                  clip_width;           /* Ширина окна для отображения видимой области. */
  guint                  clip_height;          /* Высота окна для отображения видимой области. */

  gdouble                min_x;                /* Минимально возможное значение по оси x. */
  gdouble                max_x;                /* Максимально возможное значение по оси x. */
  gdouble                min_y;                /* Минимально возможное значение по оси y. */
  gdouble                max_y;                /* Максимально возможное значение по оси y. */

  gdouble                from_x;               /* Граница отображения по оси x слева. */
  gdouble                to_x;                 /* Граница отображения по оси x справа. */
  gdouble                from_y;               /* Граница отображения по оси y снизу. */
  gdouble                to_y;                 /* Граница отображения по оси y сверху. */

  gboolean               scale_on_resize;      /* Изменять (TRUE) или нет (FALSE) масштаб при изменении размера окна. */
  gdouble                scale_x;              /* Текущий коэффициент масштаба по оси x. */
  gdouble                scale_y;              /* Текущий коэффициент масштаба по оси y. */
};

static void            gtk_cifro_area_object_finalize          (GObject                       *carea);

static gint            gtk_cifro_area_get_visible_width        (gdouble                        width,
                                                                gdouble                        height,
                                                                gdouble                        angle);

static gint            gtk_cifro_area_get_visible_height       (gdouble                        width,
                                                                gdouble                        height,
                                                                gdouble                        angle);

static void            gtk_cifro_area_update_visible           (GtkCifroArea                  *carea,
                                                                gboolean                       update_scale);

static gboolean        gtk_cifro_area_configure                (GtkWidget                     *widget,
                                                                GdkEventConfigure             *event);

static gboolean        gtk_cifro_area_draw                     (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);

#ifdef CIFRO_AREA_WITH_GTK2
static gboolean        gtk_cifro_area_expose                   (GtkWidget                     *widget,
                                                                GdkEventExpose                *event);
#endif

static guint gtk_cifro_area_signals [SIGNAL_LAST] = {0};

G_DEFINE_TYPE_WITH_PRIVATE (GtkCifroArea, gtk_cifro_area, GTK_TYPE_DRAWING_AREA)

static void
gtk_cifro_area_init (GtkCifroArea *carea)
{
  GtkCifroAreaPrivate *priv = gtk_cifro_area_get_instance_private (carea);
  gint event_mask = 0;

  carea->priv = priv;

  priv->angle = 0.0;
  priv->angle_cos = 1.0;
  priv->angle_sin = 0.0;

  priv->from_x = -1.0;
  priv->to_x = 1.0;
  priv->from_y = -1.0;
  priv->to_y = 1.0;

  event_mask |= GDK_KEY_PRESS_MASK;
  event_mask |= GDK_KEY_RELEASE_MASK;
  event_mask |= GDK_BUTTON_PRESS_MASK;
  event_mask |= GDK_BUTTON_RELEASE_MASK;
  event_mask |= GDK_POINTER_MOTION_MASK;
  event_mask |= GDK_POINTER_MOTION_HINT_MASK;
  event_mask |= GDK_SCROLL_MASK;
  gtk_widget_add_events (GTK_WIDGET (carea), event_mask);
  gtk_widget_set_can_focus (GTK_WIDGET (carea), TRUE);
}

static void
gtk_cifro_area_class_init (GtkCifroAreaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtk_cifro_area_object_finalize;
  widget_class->configure_event = gtk_cifro_area_configure;
#ifdef CIFRO_AREA_WITH_GTK2
  widget_class->expose_event = gtk_cifro_area_expose;
#else
  widget_class->draw = gtk_cifro_area_draw;
#endif

  /**
   * GtkCifroArea::visible-draw:
   * @carea: объект получивший сигнал
   * @cairo: объект #cairo_t для рисования
   *
   * Сигнал отправляется при необходимости перерисовки изображения. Этот сигнал используется
   * для формирования изображения в видимой области. Конетекст cairo всегда имеет тип
   * [Cairo Image Surface] и допускает прямой доступ к памяти для ускорения формирования
   * изображения.
   *
   */
  gtk_cifro_area_signals[SIGNAL_VISIBLE_DRAW] =
    g_signal_new ("visible-draw",
                  GTK_TYPE_CIFRO_AREA,
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * GtkCifroArea::area-draw:
   * @carea: объект получивший сигнал
   * @cairo: объект #cairo_t для рисования
   *
   * Сигнал отправляется при необходимости перерисовки изображения. Этот сигнал
   * аналогичен сигналам #GtkWidget::expose для Gtk 2.0 и #GtkWidget::draw для Gtk 3.0,
   * но отправляется в самом конце цикла перерисовки, после #GtkCifroArea::visible-draw.
   *
   */
  gtk_cifro_area_signals[SIGNAL_AREA_DRAW] =
    g_signal_new ("area-draw",
                  GTK_TYPE_CIFRO_AREA,
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
gtk_cifro_area_object_finalize (GObject *object)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (object);
  GtkCifroAreaPrivate *priv = carea->priv;

  g_clear_pointer (&priv->visible_cairo, cairo_destroy);

  G_OBJECT_CLASS (gtk_cifro_area_parent_class)->finalize (object);
}

/* Функция расчёта ширины видимой области для прямоугольника width x height, повёрнутого на угол angle. */
static gint
gtk_cifro_area_get_visible_width (gdouble width,
                                  gdouble height,
                                  gdouble angle)
{
  gint visible_width;

  angle = fabs (angle);
  if (angle > G_PI_2)
    angle = G_PI - angle;

  visible_width = round (fabs (cos (G_PI_2 - atan (width / height) - angle)) * sqrt (width * width + height * height));
  visible_width += (visible_width % 2);

  return visible_width;
}

/* Функция расчёта высоты видимой области для прямоугольника width x height, повёрнутого на угол angle. */
static gint
gtk_cifro_area_get_visible_height (gdouble width,
                                   gdouble height,
                                   gdouble angle)
{
  gint visible_height;

  angle = fabs (angle);
  if (angle > G_PI_2)
    angle = G_PI - angle;

  visible_height = round (fabs (cos (atan (width / height) - angle)) * sqrt (width * width + height * height));
  visible_height += (visible_height % 2);

  return visible_height;
}

/* Функция перерасчёта параметров отображения. */
static void
gtk_cifro_area_update_visible (GtkCifroArea *carea,
                               gboolean      update_scale)
{
  GtkCifroAreaPrivate *priv = carea->priv;

  gint visible_width, visible_height;
  gdouble x_width, y_height;

  /* Параметры отображения. */
  priv->rotate = gtk_cifro_area_get_rotate (carea);
  gtk_cifro_area_get_swap (carea, &priv->swap_x, &priv->swap_y);
  gtk_cifro_area_get_stick (carea, &priv->stick_x, &priv->stick_y);
  gtk_cifro_area_get_border (carea, &priv->border_top, &priv->border_bottom,
                                    &priv->border_left, &priv->border_right);

  priv->clip_width = 0;
  priv->clip_height = 0;
  priv->visible_width = 0;
  priv->visible_height = 0;

  /* Проверяем размеры виджета. */
  if ((priv->widget_width <= (priv->border_left + priv->border_right)) ||
      (priv->widget_height <= (priv->border_top + priv->border_bottom)))
    {
      return;
    }

  /* Размер видимой области отображения. */
  priv->clip_width = priv->widget_width - priv->border_left - priv->border_right;
  priv->clip_height = priv->widget_height - priv->border_top - priv->border_bottom;

  /* Размер видимой области отображения с учётом поворота. */
  visible_width = gtk_cifro_area_get_visible_width (priv->clip_width, priv->clip_height, priv->angle);
  visible_height = gtk_cifro_area_get_visible_height (priv->clip_width, priv->clip_height, priv->angle);

  /* Обновляем коэффициенты масштабирования. */
  if (update_scale)
    {
      priv->scale_x = (priv->to_x - priv->from_x) / visible_width;
      priv->scale_y = (priv->to_y - priv->from_y) / visible_height;

      gtk_cifro_area_check_scale (carea, &priv->scale_x, &priv->scale_y);
    }

  /* Пределы перемещения изображения. */
  gtk_cifro_area_get_limits (carea, &priv->min_x, &priv->max_x, &priv->min_y, &priv->max_y);

  /* Обновляем границы отображения если пределы перемещения меньше чем границы видимости. */
  x_width = priv->scale_x * visible_width;
  if (x_width > (priv->max_x - priv->min_x))
    {
      if (priv->stick_x == GTK_CIFRO_AREA_STICK_LEFT)
        {
          priv->from_x = priv->min_x;
          priv->to_x = priv->min_x + x_width;
        }
      else if (priv->stick_x == GTK_CIFRO_AREA_STICK_RIGHT)
        {
          priv->from_x = priv->max_x - x_width;
          priv->to_x = priv->max_x;
        }
      else
        {
          priv->from_x = (priv->min_x + ((priv->max_x - priv->min_x) / 2.0)) - (x_width / 2.0);
          priv->to_x = priv->from_x + x_width;
        }
    }
  else
    {
      if (priv->from_x < priv->min_x)
        {
          priv->from_x = priv->min_x;
          priv->to_x = priv->min_x + x_width;
        }
      if (priv->to_x > priv->max_x)
        {
          priv->from_x = priv->max_x - x_width;
          priv->to_x = priv->max_x;
        }
    }

  y_height = priv->scale_y * visible_height;
  if (y_height > (priv->max_y - priv->min_y))
    {
      if (priv->stick_y == GTK_CIFRO_AREA_STICK_TOP)
        {
          priv->from_y = priv->max_y - y_height;
          priv->to_y = priv->max_y;
        }
      else if (priv->stick_y == GTK_CIFRO_AREA_STICK_BOTTOM)
        {
          priv->from_y = priv->min_y;
          priv->to_y = priv->min_y + y_height;
        }
      else
        {
          priv->from_y = (priv->min_y + ((priv->max_y - priv->min_y) / 2.0)) - (y_height / 2.0);
          priv->to_y = priv->from_y + y_height;
        }
    }
  else
    {
      if (priv->from_y < priv->min_y)
        {
          priv->from_y = priv->min_y;
          priv->to_y = priv->min_y + y_height;
        }
      if (priv->to_y > priv->max_y)
        {
          priv->from_y = priv->max_y - y_height;
          priv->to_y = priv->max_y;
        }
    }

  /* Размеры видимой области изменились. */
  if ((priv->visible_width != visible_width) || (priv->visible_height != visible_height))
    {
      /* Если новые размеры стали больше, пересоздаём объекты рисования в видимой области. */
      if (visible_width > priv->visible_width || visible_height > priv->visible_height)
        {
          cairo_surface_t *surface;

          if (priv->visible_cairo != NULL)
            cairo_destroy (priv->visible_cairo);

          surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, visible_width, visible_height);
          priv->visible_cairo = cairo_create (surface);
          cairo_surface_destroy (surface);
        }

      /* Запоминаем новые размеры видимой области и ссобщаем их. */
      priv->visible_width = visible_width;
      priv->visible_height = visible_height;
    }
}

/* Обработчик изменения размеров виджета. */
static gboolean
gtk_cifro_area_configure (GtkWidget         *widget,
                          GdkEventConfigure *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;

  /* Новые размеры виджета. */
  priv->widget_width = event->width - (event->width % 2);
  priv->widget_height = event->height - (event->height % 2);
  gtk_cifro_area_update_visible (carea, priv->scale_on_resize);

  return FALSE;
}

/* Обработчик рисования содержимого виджета GTK 3. */
static gboolean
gtk_cifro_area_draw (GtkWidget *widget,
                     cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;

  gdouble cairo_width = priv->widget_width;
  gdouble cairo_height = priv->widget_height;
  gdouble shift_width = (priv->widget_width - priv->visible_width) / 2.0;
  gdouble shift_height = (priv->widget_height - priv->visible_height) / 2.0;
  gdouble angle = priv->angle;

  if ((priv->clip_width == 0) || (priv->clip_height == 0))
    return FALSE;

  if (priv->swap_x)
    angle = -angle;
  if (priv->swap_y)
    angle = -angle;

  /* Перерисовка видимой области. */
  if (priv->visible_cairo != NULL)
    {
      cairo_surface_t *surface = cairo_get_target (priv->visible_cairo);
      gpointer data = cairo_image_surface_get_data (surface);
      gssize dsize = cairo_image_surface_get_stride (surface);
      dsize *= cairo_image_surface_get_height (surface);

      /* Перед перерисовкой очищаем поверхность до прозрачного состояния
       * и выполняем перерисовку видимой области. */
      cairo_surface_flush (surface);

      memset (data, 0, dsize);
      g_signal_emit (GTK_CIFRO_AREA (widget), gtk_cifro_area_signals[SIGNAL_VISIBLE_DRAW],
                     0, priv->visible_cairo);

      cairo_surface_mark_dirty (surface);

      cairo_save (cairo);

      cairo_set_operator (cairo, CAIRO_OPERATOR_OVER);

      if (priv->swap_x)
        {
          cairo_scale (cairo, -1.0, 1.0);
          cairo_translate (cairo, -cairo_width, 0);
        }

      if (priv->swap_y)
        {
          cairo_scale (cairo, 1.0, -1.0);
          cairo_translate (cairo, 0, -cairo_height);
        }

      if (priv->rotate)
        {
          cairo_rectangle (cairo, priv->border_left, priv->border_top, priv->clip_width, priv->clip_height);
          cairo_clip (cairo);

          cairo_translate (cairo, cairo_width / 2.0, cairo_height / 2.0);
          cairo_rotate (cairo, angle);

          cairo_translate (cairo, -cairo_width / 2.0, -cairo_height / 2.0);
        }

      cairo_set_source_surface (cairo, cairo_get_target (priv->visible_cairo), shift_width, shift_height);
      cairo_paint (cairo);

      cairo_restore (cairo);
    }

  /* Отображаем область всего виджета. */
  cairo_save (cairo);
  g_signal_emit (GTK_CIFRO_AREA (widget), gtk_cifro_area_signals[SIGNAL_AREA_DRAW], 0, cairo);
  cairo_restore (cairo);

  return TRUE;
}

#ifdef CIFRO_AREA_WITH_GTK2

/* Обработчик рисования содержимого виджета GTK 2. */
static gboolean
gtk_cifro_area_expose (GtkWidget      *widget,
                       GdkEventExpose *event)
{
  cairo_t *cairo = gdk_cairo_create (widget->window);
  gboolean status = gtk_cifro_area_draw (widget, cairo);

  cairo_destroy (cairo);

  return status;
}

#endif

/**
 * gtk_cifro_area_new:
 *
 * Функция создаёт виджет #GtkCifroArea.
 *
 * Returns: #GtkCifroArea.
 *
 */
GtkWidget *
gtk_cifro_area_new (void)
{
  return g_object_new (GTK_TYPE_CIFRO_AREA, NULL);
}

/**
 * gtk_cifro_area_get_rotate:
 * @carea: указатель на #GtkCifroArea
 *
 * Функция возвращает значение параметра разрешения поворота изображения. Данная
 * функция является виртуальной и может быть переопределена классом, наследуемым
 * от #GtkCifroArea, для изменения поведения объекта.
 *
 * Returns: %TRUE если разрешено изменение угла поворота изображения.
 *
 */
gboolean
gtk_cifro_area_get_rotate (GtkCifroArea *carea)
{
  GtkCifroAreaClass *klass;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  klass = GTK_CIFRO_AREA_GET_CLASS (carea);

  if (klass->get_rotate != NULL)
    return klass->get_rotate (carea);

  return TRUE;
}

/**
 * gtk_cifro_area_get_swap:
 * @carea: указатель на #GtkCifroArea
 * @swap_x: (out) (allow-none): признак отражения относительно оси X или %NULL
 * @swap_y: (out) (allow-none): признак отражения относительно оси Y или %NULL
 *
 * Функция возвращает параметры зеркального отражения изображения по осям. Данная функция
 * является виртуальной и может быть переопределена классом, наследуемым от #GtkCifroArea,
 * для изменения поведения объекта.
 *
 */
void
gtk_cifro_area_get_swap (GtkCifroArea *carea,
                         gboolean     *swap_x,
                         gboolean     *swap_y)
{
  GtkCifroAreaClass *klass;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  klass = GTK_CIFRO_AREA_GET_CLASS (carea);

  if (klass->get_swap != NULL)
    {
      klass->get_swap (carea, swap_x, swap_y);
    }
  else
    {
      (swap_x != NULL) ? *swap_x = FALSE : 0;
      (swap_y != NULL) ? *swap_y = FALSE : 0;
    }
}

/**
 * gtk_cifro_area_get_stick:
 * @carea: указатель на #GtkCifroArea
 * @stick_x: (out) (allow-none): направление смещения по оси X или %NULL
 * @stick_y: (out) (allow-none): направление смещения по оси Y или %NULL
 *
 * Функция возвращает направление, в котором будут смещаться координаты, если текущий
 * масштаб требует отображения большего размера, чем установлено пределом перемещения
 * (см. gtk_cifro_area_get_limits()). Данная функция является виртуальной и может быть
 * переопределена классом, наследуемым от #GtkCifroArea, для изменения поведения объекта.
 *
 */
void
gtk_cifro_area_get_stick (GtkCifroArea          *carea,
                          GtkCifroAreaStickType *stick_x,
                          GtkCifroAreaStickType *stick_y)
{
  GtkCifroAreaClass *klass;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  klass = GTK_CIFRO_AREA_GET_CLASS (carea);

  if (klass->get_stick != NULL)
    {
      klass->get_stick (carea, stick_x, stick_y);
    }
  else
    {
      (stick_x != NULL) ? *stick_x = GTK_CIFRO_AREA_STICK_CENTER : 0;
      (stick_y != NULL) ? *stick_y = GTK_CIFRO_AREA_STICK_CENTER : 0;
    }
}

/**
 * gtk_cifro_area_get_border:
 * @carea: указатель на #GtkCifroArea
 * @border_left: (out) (allow-none): размер окантовки с левой стороны или %NULL
 * @border_right: (out) (allow-none): размер окантовки с правой стороны или %NULL
 * @border_top: (out) (allow-none): размер окантовки сверху или %NULL
 * @border_bottom: (out) (allow-none): размер окантовки снизу или %NULL
 *
 * Функция возвращает текущие размеры области окантовки виджета. Данная функция является
 * виртуальной и может быть переопределена классом, наследуемым от #GtkCifroArea, для
 * изменения поведения объекта.
 *
 */
void
gtk_cifro_area_get_border (GtkCifroArea *carea,
                           guint        *border_top,
                           guint        *border_bottom,
                           guint        *border_left,
                           guint        *border_right)
{
  GtkCifroAreaClass *klass;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  klass = GTK_CIFRO_AREA_GET_CLASS (carea);

  if (klass->get_border != NULL)
    {
      klass->get_border (carea, border_top, border_bottom, border_left, border_right);
    }
  else
    {
      (border_top != NULL) ? *border_top = 0 : 0;
      (border_bottom != NULL) ? *border_bottom = 0 : 0;
      (border_left != NULL) ? *border_left = 0 : 0;
      (border_right != NULL) ? *border_right = 0 : 0;
    }
}

/**
 * gtk_cifro_area_get_limits:
 * @carea: указатель на #GtkCifroArea
 * @min_x: (out) (allow-none): минимально возможное значение по оси X или %NULL
 * @max_x: (out) (allow-none): максимально возможное значение по оси X или %NULL
 * @min_y: (out) (allow-none): минимально возможное значение по оси Y или %NULL
 * @max_y: (out) (allow-none): максимально возможное значение по оси Y или %NULL
 *
 * Функция возвращает текущие значения пределов перемещения изображения. Данная функция
 * является виртуальной и может быть переопределена классом, наследуемым от #GtkCifroArea,
 * для изменения поведения объекта.
 *
 */
void
gtk_cifro_area_get_limits (GtkCifroArea *carea,
                           gdouble      *min_x,
                           gdouble      *max_x,
                           gdouble      *min_y,
                           gdouble      *max_y)
{
  GtkCifroAreaClass *klass;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  klass = GTK_CIFRO_AREA_GET_CLASS (carea);

  if (klass->get_limits != NULL)
    {
      klass->get_limits (carea, min_x, max_x, min_y, max_y);
    }
  else
    {
      (min_x != NULL) ? *min_x = -G_MAXDOUBLE : 0;
      (max_x != NULL) ? *max_x = +G_MAXDOUBLE : 0;
      (min_y != NULL) ? *min_y = -G_MAXDOUBLE : 0;
      (max_y != NULL) ? *max_y = +G_MAXDOUBLE : 0;
    }
}

/**
 * gtk_cifro_area_check_scale:
 * @carea: указатель на #GtkCifroArea
 * @scale_x: (out): значение масштаба по оси X
 * @scale_y: (out): значение масштаба по оси Y
 *
 * Функция используется для проверки устанавливаемых масштабов, на предмет допустимости.
 * Значения масштабов передаются в функцию, которая при необходимости может их изменить
 * на допустимые. Данная функция обычно используется только внутри класса #GtkCifroArea
 * и мало применима для обычных пользователей. Данная функция является виртуальной и может
 * быть переопределена классом, наследуемым от #GtkCifroArea, для изменения поведения объекта.
 *
 */
void
gtk_cifro_area_check_scale (GtkCifroArea *carea,
                            gdouble      *scale_x,
                            gdouble      *scale_y)
{
  GtkCifroAreaClass *klass;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  klass = GTK_CIFRO_AREA_GET_CLASS (carea);

  if (klass->check_scale != NULL)
    klass->check_scale (carea, scale_x, scale_y);
}

/**
 * gtk_cifro_area_set_scale_on_resize:
 * @carea: указатель на #GtkCifroArea
 * @scale_on_resize: признак изменения масштаба при изменении размеров виджета
 *
 * Функция задаёт поведение масштаба при изменении размеров виджета. При изменении размеров
 * виджета возможно изменение коэффициента масштабирования (@scale_on_resize = %TRUE)
 * или изменение видимой области изображения (@scale_on_resize = %FALSE).
 *
 */
void
gtk_cifro_area_set_scale_on_resize (GtkCifroArea *carea,
                                    gboolean      scale_on_resize)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  carea->priv->scale_on_resize = scale_on_resize;
}

/**
 * gtk_cifro_area_get_scale_on_resize:
 * @carea: указатель на #GtkCifroArea
 *
 * Функция возвращает текущее поведение масштаба при изменении размеров виджета.
 *
 * Returns: TRUE если при изменении размеров виджета изменяется масштаб.
 *
 */
gboolean
gtk_cifro_area_get_scale_on_resize (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  return carea->priv->scale_on_resize;
}

/**
 * gtk_cifro_area_set_scale:
 * @carea: указатель на #GtkCifroArea
 * @scale_x: масштаб по оси X
 * @scale_y: масштаб по оси Y
 * @center_x: центр изменения масштаба по оси X
 * @center_y: центр изменения масштаба по оси Y
 *
 * Функция задаёт масштабы отображения. Перед установкой масштабы проверяются с помощью
 * функции gtk_cifro_area_check_scale(). Масштабирование производится относительно центра
 * изображения, определённого параметрами @center_x и @center_y.
 *
 */
void
gtk_cifro_area_set_scale (GtkCifroArea *carea,
                          gdouble       scale_x,
                          gdouble       scale_y,
                          gdouble       center_x,
                          gdouble       center_y)
{
  GtkCifroAreaPrivate *priv;
  gdouble view_x1, view_x2;
  gdouble view_y1, view_y2;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  gtk_cifro_area_check_scale (carea, &scale_x, &scale_y);

  priv = carea->priv;

  center_x = CLAMP (center_x, priv->from_x, priv->to_x);
  center_y = CLAMP (center_y, priv->from_y, priv->to_y);

  /* Расстояния от центра масштабирования до границ отображения. */
  view_x1 = (center_x - priv->from_x) / priv->scale_x;
  view_x2 = (priv->to_x - center_x) / priv->scale_x;
  view_y1 = (center_y - priv->from_y) / priv->scale_y;
  view_y2 = (priv->to_y - center_y) / priv->scale_y;

  priv->scale_x = scale_x;
  priv->scale_y = scale_y;

  /* Новые расстояния от центра до границ отображения. */
  view_x1 *= priv->scale_x;
  view_x2 *= priv->scale_x;
  view_y1 *= priv->scale_y;
  view_y2 *= priv->scale_y;

  /* Новые границы отображения. */
  priv->from_x = center_x - view_x1;
  priv->to_x = center_x + view_x2;
  priv->from_y = center_y - view_y1;
  priv->to_y = center_y + view_y2;

  gtk_cifro_area_update_visible (carea, FALSE);

  gtk_widget_queue_draw (GTK_WIDGET (carea));
}

/**
 * gtk_cifro_area_get_scale:
 * @carea: указатель на #GtkCifroArea
 * @scale_x: (out) (allow-none): масштаб по оси X
 * @scale_y: (out) (allow-none): масштаб по оси Y
 *
 * Функция возвращает значения текущих масштабов отображения.
 *
 */
void
gtk_cifro_area_get_scale (GtkCifroArea *carea,
                          gdouble      *scale_x,
                          gdouble      *scale_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  (scale_x != NULL) ? *scale_x = priv->scale_x : 0;
  (scale_y != NULL) ? *scale_y = priv->scale_y : 0;
}

/**
 * gtk_cifro_area_set_view:
 * @carea: указатель на #GtkCifroArea
 * @from_x: минимальная граница изображения по оси X
 * @to_x: минимальная граница изображения по оси X
 * @from_y: минимальная граница изображения по оси Y
 * @to_y: минимальная граница изображения по оси Y
 *
 * Функция задаёт границы текущей видимости изображения.
 *
 */
void
gtk_cifro_area_set_view (GtkCifroArea *carea,
                         gdouble       from_x,
                         gdouble       to_x,
                         gdouble       from_y,
                         gdouble       to_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  g_return_if_fail (from_x < to_x);
  g_return_if_fail (from_y < to_y);

  priv = carea->priv;

  priv->from_x = from_x;
  priv->to_x = to_x;
  priv->from_y = from_y;
  priv->to_y = to_y;

  gtk_cifro_area_update_visible (carea, TRUE);

  gtk_widget_queue_draw (GTK_WIDGET (carea));
}

/**
 * gtk_cifro_area_get_view:
 * @carea: указатель на #GtkCifroArea
 * @from_x: (out) (allow-none): минимальная граница изображения по оси X
 * @to_x: (out) (allow-none): минимальная граница изображения по оси X
 * @from_y: (out) (allow-none): минимальная граница изображения по оси Y
 * @to_y: (out) (allow-none): минимальная граница изображения по оси Y
 *
 * Функция возвращает границы текущей видимости изображения.
 *
 */
void
gtk_cifro_area_get_view (GtkCifroArea *carea,
                         gdouble      *from_x,
                         gdouble      *to_x,
                         gdouble      *from_y,
                         gdouble      *to_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  (from_x != NULL) ? *from_x = priv->from_x : 0;
  (to_x != NULL) ? *to_x = priv->to_x : 0;
  (from_y != NULL) ? *from_y = priv->from_y : 0;
  (to_y != NULL) ? *to_y = priv->to_y : 0;
}

/**
 * gtk_cifro_area_set_angle:
 * @carea: указатель на #GtkCifroArea
 * @angle: угол поворота изображения в радианах
 *
 * Функция задаёт угол поворота изображения.
 *
 */
void
gtk_cifro_area_set_angle (GtkCifroArea *carea,
                          gdouble       angle)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (!priv->rotate)
    return;

  angle = fmodf (angle, 2.0 * G_PI);
  if (angle > G_PI)
    angle = angle - 2.0 * G_PI;
  if (angle < -G_PI)
    angle = angle + 2.0 * G_PI;

  priv->angle = angle;
  priv->angle_cos = cos (priv->angle);
  priv->angle_sin = sin (priv->angle);

  if (priv->rotate)
    {
      gtk_cifro_area_update_visible (carea, FALSE);

      gtk_widget_queue_draw (GTK_WIDGET (carea));
    }
}

/**
 * gtk_cifro_area_get_angle:
 * @carea: указатель на #GtkCifroArea
 *
 * Функция возвращает текущее значение угола поворота изображения.
 *
 * Returns: угол текущего поворота изображения в радианах.
 *
 */
gdouble
gtk_cifro_area_get_angle (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), 0.0);

  return carea->priv->angle;
}

/**
 * gtk_cifro_area_move:
 * @carea: указатель на #GtkCifroArea
 * @step_x: шаг перемещения по оси X, точки экрана
 * @step_y: шаг перемещения по оси Y, точки экрана
 *
 * Функция смещает изображение на определенный шаг.
 *
 */
void
gtk_cifro_area_move (GtkCifroArea *carea,
                     gint          step_x,
                     gint          step_y)
{
  GtkCifroAreaPrivate *priv;
  gdouble shift_x, shift_y;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  shift_x = step_x * priv->angle_cos - step_y * priv->angle_sin;
  shift_y = step_y * priv->angle_cos + step_x * priv->angle_sin;

  shift_x *= priv->scale_x;
  shift_y *= priv->scale_y;

  if (priv->swap_x)
    shift_x = -shift_x;
  if (priv->swap_y)
    shift_y = -shift_y;

  priv->from_x += shift_x;
  priv->to_x += shift_x;
  priv->from_y += shift_y;
  priv->to_y += shift_y;

  gtk_cifro_area_update_visible (carea, FALSE);

  gtk_widget_queue_draw (GTK_WIDGET (carea));
}

/**
 * gtk_cifro_area_rotate:
 * @carea: указатель на #GtkCifroArea
 * @angle: угол поворота изображения, радианы
 *
 * Функция поворачивает изображение на определенный угол. Угол добавляется к
 * текущему углу на который повернуто изображение.
 *
 */
void
gtk_cifro_area_rotate (GtkCifroArea *carea,
                       gdouble       angle)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  angle += carea->priv->angle;

  gtk_cifro_area_set_angle (carea, angle);
}

/**
 * gtk_cifro_area_zoom:
 * @carea: указатель на #GtkCifroArea
 * @direction_x: направление масштабирования по оси X
 * @direction_y: направление масштабирования по оси Y
 * @center_x: центр масштабирования по оси X, логические единицы
 * @center_y: центр масштабирования по оси Y, логические единицы
 *
 * Функция изменяет текущий масштаб изображения.
 *
 */
void
gtk_cifro_area_zoom (GtkCifroArea         *carea,
                     GtkCifroAreaZoomType  direction_x,
                     GtkCifroAreaZoomType  direction_y,
                     gdouble               center_x,
                     gdouble               center_y)
{
  GtkCifroAreaClass *klass;
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  klass = GTK_CIFRO_AREA_GET_CLASS (carea);
  priv = carea->priv;

  if (klass->zoom != NULL)
    {
      klass->zoom (carea, direction_x, direction_y, center_x, center_y);
    }
  else
    {
      gdouble scale_x = priv->scale_x;
      gdouble scale_y = priv->scale_y;

      if (direction_x == GTK_CIFRO_AREA_ZOOM_IN)
        scale_x = 0.9 * priv->scale_x;
      else if (direction_x == GTK_CIFRO_AREA_ZOOM_OUT)
        scale_x = 1.1 * priv->scale_x;

      if (direction_y == GTK_CIFRO_AREA_ZOOM_IN)
        scale_y = 0.9 * priv->scale_y;
      else if (direction_y == GTK_CIFRO_AREA_ZOOM_OUT)
        scale_y = 1.1 * priv->scale_y;

      gtk_cifro_area_set_scale (carea, scale_x, scale_y, center_x, center_y);
    }
}

/**
 * gtk_cifro_area_get_size:
 * @carea: указатель на #GtkCifroArea
 * @width: (out) (allow-none): используемая ширина виджета
 * @height: (out) (allow-none): используемая высота виджета
 *
 * Функция возвращает используемый размер виджета.
 *
 */
void
gtk_cifro_area_get_size (GtkCifroArea *carea,
                         guint        *width,
                         guint        *height)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  (width != NULL) ? *width = priv->widget_width : 0;
  (height != NULL) ? *height = priv->widget_height : 0;
}

/**
 * gtk_cifro_area_get_visible_size:
 * @carea: указатель на #GtkCifroArea
 * @width: (out) (allow-none): ширина видимой области
 * @height: (out) (allow-none): используемая высота виджета
 *
 * Функция возвращает текущие значения размеров видимой области.
 *
 */
void
gtk_cifro_area_get_visible_size (GtkCifroArea *carea,
                                 guint        *width,
                                 guint        *height)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  (width != NULL) ? *width = priv->visible_width : 0;
  (height != NULL) ? *height = priv->visible_height : 0;
}

/**
 * gtk_cifro_area_point_to_value:
 * @carea: указатель на #GtkCifroArea
 * @x: координата x в окне виджета
 * @y: координата y в окне виджета
 * @x_val: (out) (allow-none): значение x в логической системе координат
 * @y_val: (out) (allow-none): значение y в логической системе координат
 *
 * Функция преобразовает координаты из системы координат виджета в логические координаты.
 *
 */
void
gtk_cifro_area_point_to_value (GtkCifroArea *carea,
                               gdouble       x,
                               gdouble       y,
                               gdouble      *x_val,
                               gdouble      *y_val)
{
  GtkCifroAreaPrivate *priv;

  gdouble x_val_tmp, y_val_tmp;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  /* Переносим систему координат в центр отображаемой области (виджета или изображения).
   * Этот перенос необходим для корректного расчёта логических координат при повороте.*/
  x = x - priv->widget_width / 2.0;
  y = priv->widget_height / 2.0 - y;

  /* Расчитываем логическую координату с учётом возможного поворота изображения. */
  x_val_tmp = (x * priv->angle_cos - y * priv->angle_sin) * priv->scale_x;
  y_val_tmp = (y * priv->angle_cos + x * priv->angle_sin) * priv->scale_y;

  /* Если включено зеркальное отражение по осям - переносим координаты. */
  if (priv->swap_x)
    x_val_tmp = -x_val_tmp;
  if (priv->swap_y)
    y_val_tmp = -y_val_tmp;

  /* Выполняем обратный перенос системы координат. */
  x_val_tmp = ((priv->to_x - priv->from_x) / 2.0) + priv->from_x + x_val_tmp;
  y_val_tmp = ((priv->to_y - priv->from_y) / 2.0) + priv->from_y + y_val_tmp;

  (x_val != NULL) ? *x_val = x_val_tmp : 0;
  (y_val != NULL) ? *y_val = y_val_tmp : 0;
}

/**
 * gtk_cifro_area_value_to_point:
 * @carea: указатель на #GtkCifroArea
 * @x: (out) (allow-none): координата x в окне виджета
 * @y: (out) (allow-none): координата y в окне виджета
 * @x_val: значение x в логической системе координат
 * @y_val: значение y в логической системе координат
 *
 * Функция преобразовает координаты из логических координат в систему координат виджета.
 *
 */
void
gtk_cifro_area_value_to_point (GtkCifroArea *carea,
                               gdouble      *x,
                               gdouble      *y,
                               gdouble       x_val,
                               gdouble       y_val)
{
  GtkCifroAreaPrivate *priv;

  gdouble x_tmp, y_tmp;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  /* Переносим систему координат в центр видимой области.
   * Этот перенос необходим для корректного расчёта координат при повороте. */
  x_val = x_val - ((priv->to_x - priv->from_x) / 2.0) - priv->from_x;
  y_val = y_val - ((priv->to_y - priv->from_y) / 2.0) - priv->from_y;

  /* Перводим из логических координат в точки. */
  x_val = x_val / priv->scale_x;
  y_val = y_val / priv->scale_y;

  /* Если включено зеркальное отражение по осям - переносим координаты. */
  if (priv->swap_x)
    x_val = -x_val;
  if (priv->swap_y)
    y_val = -y_val;

  /* Расчитываем координаты с учётом возможного поворота изображения. */
  x_tmp = x_val * priv->angle_cos + y_val * priv->angle_sin;
  y_tmp = y_val * priv->angle_cos - x_val * priv->angle_sin;

  /* Выполняем обратный перенос системы координат. */
  x_tmp = x_tmp + priv->widget_width / 2.0;
  y_tmp = priv->widget_height / 2.0 - y_tmp;

  (x != NULL) ? *x = x_tmp : 0;
  (y != NULL) ? *y = y_tmp : 0;
}

/**
 * gtk_cifro_area_visible_point_to_value:
 * @carea: указатель на #GtkCifroArea
 * @x: координата x в видимой области
 * @y: координата y в видимой области
 * @x_val: (out) (allow-none): значение x в логической системе координат
 * @y_val: (out) (allow-none): значение y в логической системе координат
 *
 * Функция преобразовает координаты из прямоугольной системы видимой области
 * в логические координаты.
 *
 */
void
gtk_cifro_area_visible_point_to_value (GtkCifroArea *carea,
                                       gdouble       x,
                                       gdouble       y,
                                       gdouble      *x_val,
                                       gdouble      *y_val)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  (x_val != NULL) ? *x_val = (priv->from_x + x * priv->scale_x) : 0;
  (y_val != NULL) ? *y_val = (priv->to_y - y * priv->scale_y) : 0;
}

/**
 * gtk_cifro_area_visible_value_to_point:
 * @carea: указатель на #GtkCifroArea
 * @x: (out) (allow-none): координата x в видимой области
 * @y: (out) (allow-none): координата y в видимой области
 * @x_val: значение x в логической системе координат
 * @y_val: значение y в логической системе координат
 *
 * Функция преобразовает координаты из логических в прямоугольную систему
 * координат видимой области.
 *
 */
void
gtk_cifro_area_visible_value_to_point (GtkCifroArea *carea,
                                       gdouble      *x,
                                       gdouble      *y,
                                       gdouble       x_val,
                                       gdouble       y_val)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  (x != NULL) ? *x = ((x_val - priv->from_x) / priv->scale_x) : 0;
  (y != NULL) ? *y = ((priv->to_y - y_val) / priv->scale_y) : 0;
}

/**
 * gtk_cifro_area_get_axis_step:
 * @scale: масштаб, число точек экрана в одной логической единице
 * @step_width: желаемое расстояние между координатными осями, точки экрана
 * @from: (out): логическая координата первой линии сетки
 * @step: (out) (allow-none): логические шаг между линиями сетки
 * @range: (out) (allow-none): цена деления координатной сетки
 * @power: (out) (allow-none): степень цены деления координатной сетки
 *
 * Функция расчитывает параметры прямоугольной координатной сетки. В функцию должны быть
 * переданы масштаб, желаемое расстояние между осями и начало области. Функция вернёт
 * выровненную по шагу между линиями сетки координату первой линии. При необходимости
 * функция вернёт шаг между линиями сетки, цену деления сетки и степень цены деления сетки.
 *
 * Returns: %TRUE если параметры рассчитаны.
 *
 */
gboolean
gtk_cifro_area_get_axis_step (gdouble  scale,
                              gdouble  step_width,
                              gdouble *from,
                              gdouble *step,
                              guint   *range,
                              gint    *power)
{
  gdouble step_length;

  gdouble axis_1_width_delta;
  gdouble axis_2_width_delta;
  gdouble axis_5_width_delta;

  gdouble axis_1_score;
  gdouble axis_2_score;
  gdouble axis_5_score;

  gdouble from_ret;
  gdouble step_ret;
  guint range_ret;
  gint power_ret;

  /* Расстояние между соседними линиями координатной сетки в логических координатах. */
  step_length = scale * step_width;
  if (step_length <= 0.0)
    return FALSE;

  /* Нормируем расстояние в диапазон значений от 1 до 10. */
  power_ret = 0;
  while ((step_length > 10) || (step_length < 1))
    {
      if (step_length > 10)
        {
          step_length /= 10.0;
          power_ret = power_ret + 1;
        }
      if (step_length < 1)
        {
          step_length *= 10.0;
          power_ret = power_ret - 1;
        }
    }
  if (step_length > 5)
    {
      step_length /= 10.0;
      power_ret = power_ret + 1;
    }

  /* Выбираем с каким шагом рисовать сетку: 1, 2 или 5 (плюс их степени). */

  /* Расчитываем разность между размером ячейки для трёх возможных вариантов сетки и
   * предпочтительным размером ячейки определённым пользователем. */
  axis_1_width_delta = (1.0 * pow (10.0, power_ret) / scale) - step_width;
  axis_2_width_delta = (2.0 * pow (10.0, power_ret) / scale) - step_width;
  axis_5_width_delta = (5.0 * pow (10.0, power_ret) / scale) - step_width;

  /* Расчитываем "вес" каждого варианта. */
  axis_1_score = (axis_1_width_delta >= 0.0) ? 1.0 / axis_1_width_delta : -0.1 / axis_1_width_delta;
  axis_2_score = (axis_2_width_delta >= 0.0) ? 1.0 / axis_2_width_delta : -0.1 / axis_2_width_delta;
  axis_5_score = (axis_5_width_delta >= 0.0) ? 1.0 / axis_5_width_delta : -0.1 / axis_5_width_delta;

  /* Выбираем предпочтительный шаг координатной сетки. */
  if ((axis_1_score > axis_2_score) && (axis_1_score > axis_5_score))
    range_ret = 1;
  else if (axis_2_score > axis_5_score)
    range_ret = 2;
  else
    range_ret = 5;

  /* Расчитываем истиный шаг координатной сетки с учётом нормировки. */
  step_ret = range_ret * pow (10.0, power_ret);

  /* Корректируем начало координатной сетки. */
  from_ret = step_ret * floor (*from / step_ret);
  if (from && from_ret < *from)
    from_ret += step_ret;

  /* Возвращаем результаты. */
  *from = from_ret;
  (step != NULL) ? *step = step_ret : 0;
  (range != NULL) ? *range = range_ret : 0;
  (power != NULL) ? *power = power_ret : 0;

  return TRUE;
}

/**
 * gtk_cifro_area_point_to_cairo:
 * @point: координата для выравнивания
 *
 * Функция выравнивает координаты для использования в библиотеке cairo.
 *
 * Returns: координата для использования в библиотеке cairo.
 *
 */
gdouble
gtk_cifro_area_point_to_cairo (gdouble point)
{
  gdouble ipoint = (glong) point;

  return ((point - ipoint) > 0.5) ? ipoint + 0.5 : ipoint - 0.5;
}
