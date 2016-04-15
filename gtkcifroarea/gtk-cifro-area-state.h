/*
 * GtkCifroArea - 2D layers image management library.
 *
 * Copyright 2013-2016 Andrei Fadeev (andrei@webcontrol.ru)
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

/*!
 * \file gtk-cifro-area-state.h
 *
 * \brief Заголовочный файл класса отслеживания состояния GtkCifroArea
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup GtkCifroAreaState GtkCifroAreaState - класс отслеживания состояния GtkCifroArea
 *
 * В процессе использования объекта \link GtkCifroArea \endlink возникает необходимость получения
 * информации о состоянии области отображения. Класс GtkCifroAreaState может использоваться
 * объектами формирующими изображения для получения текущего состояния области отображения,
 * а также, для получения уведомлений об изменениях в параметрах. Кроме этого, класс GtkCifroAreaState содержит
 * вспомогательные функции для преобразования координат и расчета параметров прямоугольной координатной сетки.
 *
 * Сторонние модули должны получить указатель на объект GtkCifroAreaState из объекта \link GtkCifroArea \endlink
 * при помощи функции \link gtk_cifro_area_get_state \endlink.
 *
 * В процессе работы \link GtkCifroArea \endlink будет обновлять параметры состояния области отображения
 * через вызов соответствующих функций. При изменении параметров будут посылаться сигналы с новыми значениями
 * параметров. Пользователю запрещено вызывать функции изменяющие параметры.
 *
 * Список функций и соответствующие им сигналы:
 *
 * - #gtk_cifro_area_state_set_area_size (сигнал - "area-changed") - при изменении размера области вывода (виджета или изображения);
 * - #gtk_cifro_area_state_set_visible_size (сигнал - "visible-changed") - при изменении "видимой" области;
 * - #gtk_cifro_area_state_set_border (сигнал - "border-changed") - при изменении размера окантовки вокруг видимой области;
 * - #gtk_cifro_area_state_set_swap (сигнал - "swap-changed") - при изменении параметров зеркального отражения по осям;
 * - #gtk_cifro_area_state_set_angle (сигнал - "angle-changed") - при изменении угла поворота изображения "видимой" области;
 * - #gtk_cifro_area_state_set_view_limits (сигнал - "view-limits-changed") - при изменении пределов отображения;
 * - #gtk_cifro_area_state_set_view (сигнал - "view-changed") - при изменении текущей границы отображения.
 *
 * Вместе с сигналом посылается указатель на одну из структур с новыми значениями изменённых параметров:
 *
 * - \link GtkCifroAreaSize \endlink - для сигналов "area-changed" и "visible-changed";
 * - \link GtkCifroAreaBorder \endlink - для сигнала "border-changed";
 * - \link GtkCifroAreaSwap \endlink - для сигнала "swap-chaged";
 * - \link GtkCifroAreaView \endlink - для сигнала "view-limits-changed" и "view-changed".
 *
 * Для сигнала "angle-changed" новое значение угла передаётся как число gdouble.
 *
 * Прототипы функций обработчиков сигналов должны быть следующими:
 *
 * - "area-changed" - void area_changed_cb (GtkCifroAreaState *state, GtkCifroAreaSize *size, gpointer user_data);
 * - "visible-changed" - void visible_changed_cb (GtkCifroAreaState *state, GtkCifroAreaSize *size, gpointer user_data);
 * - "border-changed" - void border_changed_cb (GtkCifroAreaState *state, GtkCifroAreaBorder *border, gpointer user_data);
 * - "swap-changed" - void swap_changed_cb (GtkCifroAreaState *state, GtkCifroAreaSwap *swap, gpointer user_data);
 * - "angle-changed" - void angle_changed_cb (GtkCifroAreaState *state, gdouble angle, gpointer user_data);
 * - "view-limits-changed" - void view_limits_changed_cb (GtkCifroAreaState *state, GtkCifroAreaView *view, gpointer user_data).
 * - "view-changed" - void view_changed_cb (GtkCifroAreaState *state, GtkCifroAreaView *view, gpointer user_data).
 *
 * В любой момент времени можно получить значения параметров состояния области отображения при помощи функций:
 *
 * - #gtk_cifro_area_state_get_area_size - размер области вывода (виджета или изображения);
 * - #gtk_cifro_area_state_get_visible_size - размер "видимой" области;
 * - #gtk_cifro_area_state_get_border - размер окантовки вокруг видимой области;
 * - #gtk_cifro_area_state_get_swap - параметры зеркального отражения по осям;
 * - #gtk_cifro_area_state_get_angle - угол поворота изображения "видимой" области;
 * - #gtk_cifro_area_state_get_view_limits - пределы отображения;
 * - #gtk_cifro_area_state_get_view - текущая граница отображения.
 *
 * Для пересчета координат точки из прямоугольной системы координат окна в значения "видимой" области и обратно используются
 * функции #gtk_cifro_area_state_point_to_value и #gtk_cifro_area_state_value_to_point.
 *
 * Для пересчета координат из прямоугольной системы видимой области в значения и обратно используются
 * функции #gtk_cifro_area_state_visible_point_to_value и #gtk_cifro_area_state_visible_value_to_point.
 *
 * Одной из распространенных задач является нанесения координатной сетки на изображение. Для
 * упрощения расчета координат и шага между линиями сетки предназначена функция #gtk_cifro_area_state_get_axis_step.
 *
 * Линии толщиной 1 пиксель библиотека cairo рисует без размытия если координата равна
 * ( целое число +- 0,5 ), функция #gtk_cifro_area_state_point_to_cairo выравнивает координату по этому правилу.
 *
 * Объект не поддерживает работу в многопоточном режиме.
 *
 */

#ifndef __GTK_CIFRO_AREA_STATE_H__
#define __GTK_CIFRO_AREA_STATE_H__

#include <glib-object.h>
#include <gtk-cifro-area-exports.h>

G_BEGIN_DECLS

/*! \brief Структура с размерами областей \link GtkCifroArea \endlink.  */
typedef struct
{
  gint                 width;                  /*!< Ширина области (в точках). */
  gint                 height;                 /*!< Высота области (в точках). */
} GtkCifroAreaSize;

/*! \brief Структура с размерами окантовки \link GtkCifroArea \endlink.  */
typedef struct
{
  gint                 left;                   /*!< Размер окантовки слева (в точках). */
  gint                 right;                  /*!< Размер окантовки справа (в точках). */
  gint                 top;                    /*!< Размер окантовки сверху (в точках). */
  gint                 bottom;                 /*!< Размер окантовки снизу (в точках). */
} GtkCifroAreaBorder;

/*! \brief Структура с параметрами зеркального отражения по осям \link GtkCifroArea \endlink. */
typedef struct
{
  gboolean             swap_x;                 /*!< Зеркальное отражение по оси x (TRUE если отражение установлено). */
  gboolean             swap_y;                 /*!< Зеркальное отражение по оси y (TRUE если отражение установлено). */
} GtkCifroAreaSwap;

/*! \brief Структура с границами значений \link GtkCifroArea \endlink.  */
typedef struct
{
  gdouble              x1;                     /*!< Меньшее значение оси x (логические координаты). */
  gdouble              x2;                     /*!< Большее значение оси x (логические координаты). */
  gdouble              y1;                     /*!< Меньшее значение оси y (логические координаты). */
  gdouble              y2;                     /*!< Большее значение оси y (логические координаты). */
} GtkCifroAreaView;

#define GTK_TYPE_CIFRO_AREA_STATE             (gtk_cifro_area_state_get_type ())
#define GTK_CIFRO_AREA_STATE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CIFRO_AREA_STATE, GtkCifroAreaState))
#define GTK_IS_CIFRO_AREA_STATE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CIFRO_AREA_STATE))
#define GTK_CIFRO_AREA_STATE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CIFRO_AREA_STATE, GtkCifroAreaStateClass))
#define GTK_IS_CIFRO_AREA_STATE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CIFRO_AREA_STATE))
#define GTK_CIFRO_AREA_STATE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CIFRO_AREA_STATE, GtkCifroAreaStateClass))

typedef struct _GtkCifroAreaState GtkCifroAreaState;
typedef struct _GtkCifroAreaStatePrivate GtkCifroAreaStatePrivate;
typedef struct _GtkCifroAreaStateClass GtkCifroAreaStateClass;

struct _GtkCifroAreaState
{
  GObject parent_instance;

  GtkCifroAreaStatePrivate *priv;
};

struct _GtkCifroAreaStateClass
{
  GObjectClass parent_class;
};

GTK_CIFROAREA_EXPORT
GType                  gtk_cifro_area_state_get_type                   (void);

/*!
 *
 * Функция создаёт новый объект \link GtkCifroAreaState \endlink. Объект должен быть
 * удалён функцией g_object_unref по окончанию использования.
 *
 * Пользователи \link GtkCifroArea \endlink не должны использовать
 * эту функцию (см. описание \link GtkCifroAreaState \endlink).
 *
 * \return Указатель на объект \link GtkCifroAreaState \endlink.
 *
 */
GTK_CIFROAREA_EXPORT
GtkCifroAreaState     *gtk_cifro_area_state_new                        (void);

/*!
 *
 * Функция задаёт новые значения размеров области вывода.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param width ширина области вывода;
 * \param height высота области вывода.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_set_area_size              (GtkCifroAreaState     *state,
                                                                        gint                   width,
                                                                        gint                   height);

/*!
 *
 * Функция возвращает текущие значения размеров области вывода.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param width ширина области вывода или NULL;
 * \param height высота области вывода или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_area_size              (GtkCifroAreaState     *state,
                                                                        gint                  *width,
                                                                        gint                  *height);

/*!
 *
 * Функция задаёт новые значения размеров видимой области.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param width ширина видимой области;
 * \param height высота видимой области.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_set_visible_size           (GtkCifroAreaState     *state,
                                                                        gint                   width,
                                                                        gint                   height);

/*!
 *
 * Функция возвращает текущие значения размеров видимой области.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param width ширина видимой области или NULL;
 * \param height высота видимой области или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_visible_size           (GtkCifroAreaState     *state,
                                                                        gint                  *width,
                                                                        gint                  *height);

/*!
 *
 * Функция задаёт новые значения размеров окантовки видимой области.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param left размер окантовки слева;
 * \param right размер окантовки справа;
 * \param top размер окантовки сверху;
 * \param bottom размер окантовки снизу.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_set_border                 (GtkCifroAreaState     *state,
                                                                        gint                   left,
                                                                        gint                   right,
                                                                        gint                   top,
                                                                        gint                   bottom);

/*!
 *
 * Функция возвращает текущие значения размеров окантовки видимой области.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param left размер окантовки слева или NULL;
 * \param right размер окантовки справа или NULL;
 * \param top размер окантовки сверху или NULL;
 * \param bottom размер окантовки снизу или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_border                 (GtkCifroAreaState     *state,
                                                                        gint                  *left,
                                                                        gint                  *right,
                                                                        gint                  *top,
                                                                        gint                  *bottom);

/*!
 *
 * Функция задаёт новые значения параметров зеркального отражения по осям.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param swap_x TRUE - если при выводе изображение зеркально отражается по оси x, иначе - FALSE;
 * \param swap_y TRUE - если при выводе изображение зеркально отражается по оси y, иначе - FALSE.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_set_swap                   (GtkCifroAreaState     *state,
                                                                        gboolean               swap_x,
                                                                        gboolean               swap_y);

/*!
 *
 * Функция возвращает текущие значения параметров зеркального отражения по осям.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param swap_x зеркальное отражение по оси x или NULL;
 * \param swap_y зеркальное отражение по оси y или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_swap                   (GtkCifroAreaState     *state,
                                                                        gboolean              *swap_x,
                                                                        gboolean              *swap_y);

/*!
 *
 * Функция задаёт новое значение угла поворота изображения видимой области.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param angle угол в радианах на который производится поворот.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_set_angle                  (GtkCifroAreaState     *state,
                                                                        gdouble                angle);

/*!
 *
 * Функция возвращает текущее значение угла поворота изображения видимой области.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink.
 *
 * \return Угол в радианах на который производится поворот.
 *
 */
GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_state_get_angle                  (GtkCifroAreaState     *state);

/*!
 *
 * Функция задаёт новые значения пределов отображения.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param min_x минимально возможное значение по оси x;
 * \param max_x максимально возможное значение по оси x;
 * \param min_y минимально возможное значение по оси y;
 * \param max_y максимально возможное значение по оси y.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_set_view_limits            (GtkCifroAreaState     *state,
                                                                        gdouble                min_x,
                                                                        gdouble                max_x,
                                                                        gdouble                min_y,
                                                                        gdouble                max_y);

/*!
 *
 * Функция возвращает значения пределов отображения.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param min_x минимально возможное значение по оси x или NULL;
 * \param max_x максимально возможное значение по оси x или NULL;
 * \param min_y минимально возможное значение по оси y или NULL;
 * \param max_y максимально возможное значение по оси y или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_view_limits            (GtkCifroAreaState     *state,
                                                                        gdouble               *min_x,
                                                                        gdouble               *max_x,
                                                                        gdouble               *min_y,
                                                                        gdouble               *max_y);

/*!
 *
 * Функция задаёт новые значения текущей границы отображения.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param from_x минимальная граница изображения по оси x;
 * \param to_x максимальная граница изображения по оси x;
 * \param from_y минимальная граница изображения по оси y;
 * \param to_y максимальная граница изображения по оси y.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_set_view                   (GtkCifroAreaState     *state,
                                                                        gdouble                from_x,
                                                                        gdouble                to_x,
                                                                        gdouble                from_y,
                                                                        gdouble                to_y);

/*!
 *
 * Функция возвращает значения текущей границы отображения.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param from_x минимальная граница изображения по оси x или NULL;
 * \param to_x максимальная граница изображения по оси x или NULL;
 * \param from_y минимальная граница изображения по оси y или NULL;
 * \param to_y максимальная граница изображения по оси y или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_view                   (GtkCifroAreaState     *state,
                                                                        gdouble               *from_x,
                                                                        gdouble               *to_x,
                                                                        gdouble               *from_y,
                                                                        gdouble               *to_y);

/*!
 *
 * Функция возвращает значения текущих масштабов отображения.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param scale_x текущий масштаб по оси x или NULL;
 * \param scale_y текущий масштаб по оси y или NULL;
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_scale                  (GtkCifroAreaState     *state,
                                                                        gdouble               *scale_x,
                                                                        gdouble               *scale_y);

/*!
 *
 * Функция преобразовает координаты из прямоугольной системы окна в логические координаты
 * отображаемые в объекте \link GtkCifroArea \endlink.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param x координата x в системе координат окна;
 * \param y координата y в системе координат окна;
 * \param x_val координата x в логической системе координат или NULL;
 * \param y_val координата y в логической системе координат или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_point_to_value             (GtkCifroAreaState     *state,
                                                                        gdouble                x,
                                                                        gdouble                y,
                                                                        gdouble               *x_val,
                                                                        gdouble               *y_val);

/*!
 *
 * Функция преобразовает координаты из логических, отображаемых в объекте
 * \link GtkCifroArea \endlink  в прямоугольную систему координат окна.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param x координата x в системе координат окна или NULL;
 * \param y координата y в системе координат окна или NULL;
 * \param x_val координата x в логической системе координат;
 * \param y_val координата y в логической системе координат.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_value_to_point             (GtkCifroAreaState     *state,
                                                                        gdouble               *x,
                                                                        gdouble               *y,
                                                                        gdouble                x_val,
                                                                        gdouble                y_val);

/*!
 *
 * Функция преобразовает координаты из прямоугольной системы видимой области
 * в логические координаты отображаемые в объекте \link GtkCifroArea \endlink.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param x координата x в системе координат видимой области;
 * \param y координата y в системе координат видимой области;
 * \param x_val координата x в логической системе координат или NULL;
 * \param y_val координата y в логической системе координат или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_visible_point_to_value     (GtkCifroAreaState     *state,
                                                                        gdouble                x,
                                                                        gdouble                y,
                                                                        gdouble               *x_val,
                                                                        gdouble               *y_val);

/*!
 *
 * Функция преобразовает координаты из логических, отображаемых в объекте
 * \link GtkCifroArea \endlink в прямоугольную систему координат видимой области.
 *
 * \param state указатель на объект \link GtkCifroAreaState \endlink;
 * \param x координата x в системе координат видимой области или NULL;
 * \param y координата y в системе координат видимой области или NULL;
 * \param x_val координата x в логической системе координат;
 * \param y_val координата y в логической системе координат.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_visible_value_to_point     (GtkCifroAreaState     *state,
                                                                        gdouble               *x,
                                                                        gdouble               *y,
                                                                        gdouble                x_val,
                                                                        gdouble                y_val);

/*!
 *
 * Функция расчитывает параметры прямоугольной координатной сетки.
 *
 * В функцию должны быть переданы масштаб, желаемое расстояние между осями и начало
 * области. Функция вернёт выровненную по шагу между линиями сетки координату первой линии.
 * При необходимости функция вернёт шаг между линиями сетки, цену деления сетки и степень цены деления сетки.
 *
 * \param scale масштаб - число пикселей в одной логической единице;
 * \param step_width желаемое расстояние между координатными осями;
 * \param from логическая координата первой линии сетки;
 * \param step логические шаг между линиями сетки или NULL;
 * \param range "цена" деления координатной сетки или NULL;
 * \param power степень "цены" деления координатной сетки или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_state_get_axis_step              (gdouble                scale,
                                                                        gdouble                step_width,
                                                                        gdouble               *from,
                                                                        gdouble               *step,
                                                                        gint                  *range,
                                                                        gint                  *power);

/*!
 *
 * Функция выравнивает координаты для использования в библиотеке cairo.
 *
 * \param point координата для выравнивания.
 *
 * \return Координата для использования в библиотеке cairo.
 *
 */
GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_state_point_to_cairo             (gdouble                point);

G_END_DECLS

#endif /* __GTK_CIFRO_AREA_STATE_H__ */
