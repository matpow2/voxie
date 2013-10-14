/*
Copyright (c) 2013 Mathias Kaerlev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "palette.h"
#include "voxel.h"
#include "mainwindow.h"
#include "editorcommon.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QKeyEvent>
#include <QPainter>
#include <QLabel>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

#define POINTER_SIZE 10
#define POINTER_SIZE2 (POINTER_SIZE - 2)

void draw_pointer(int x, int y, QPainter & painter)
{
    painter.setPen(Qt::black);
    painter.drawArc(x - POINTER_SIZE / 2, y - POINTER_SIZE / 2, 
                    POINTER_SIZE, POINTER_SIZE, 0, 16 * 360);
    painter.setPen(Qt::white);
    painter.drawArc(x - POINTER_SIZE2 / 2, y - POINTER_SIZE2 / 2, 
                    POINTER_SIZE2, POINTER_SIZE2, 0, 16 * 360);
}

// ColorSpace

ColorSpace::ColorSpace(QWidget * parent)
: QWidget(parent), pix(NULL), hue(-1.0f)
{
    set_hsv(0.5f, 0.5f, 0.5f);
}

void ColorSpace::set_hue(float h)
{
    set_hsv(h, sat, val);
}

void ColorSpace::set_hsv(float h, float s, float v)
{
    h = std::max(0.0f, std::min(1.0f, h));
    s = std::max(0.0f, std::min(1.0f, s));
    v = std::max(0.0f, std::min(1.0f, v));
    if (hue != h) {
        hue = h;
        delete pix;
        pix = NULL;
    }
    sat = s;
    val = v;
    update();
}

void ColorSpace::set_mouse_pos(const QPoint & p)
{
    float s = float(p.x()) / float(width());
    float v = 1.0f - float(p.y()) / float(height());
    set_hsv(hue, s, v);
    ((PaletteEditor*)parentWidget())->set_palette();
}

void ColorSpace::mousePressEvent(QMouseEvent * event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    set_mouse_pos(event->pos());
}

void ColorSpace::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    set_mouse_pos(event->pos());
}

void ColorSpace::paintEvent(QPaintEvent * event)
{
    int w = width();
    int h = height();

    if (!pix || pix->height() != h || pix->width() != w) {
        delete pix;
        QImage img(w, h, QImage::Format_RGB32);
        unsigned int * pixel = (unsigned int*)img.scanLine(0);
        for (int y = 0; y < h; y++) {
            const unsigned int * end = pixel + w;
            int x = 0;
            while (pixel < end) {
                QColor c;
                float sat = float(x) / float(w);
                float val = 1.0f - float(y) / float(h);
                c.setHsvF(hue, sat, val);
                *pixel = c.rgb();
                pixel++;
                x++;
            }
        }
        pix = new QPixmap(QPixmap::fromImage(img));
    }

    QPainter p(this);
    p.drawPixmap(0, 0, *pix);

    int p_x = int(sat * w);
    int p_y = int((1.0f - val) * h);
    draw_pointer(p_x, p_y, p);
}

// ColorSlider

ColorSlider::ColorSlider(QWidget * parent)
: QWidget(parent), pix(NULL)
{
    setFixedHeight(10);
}

void ColorSlider::set(float v)
{
    value = std::max(0.0f, std::min(1.0f, v));
    ((PaletteEditor*)parentWidget())->color_space->set_hue(value);
    update();
}

void ColorSlider::set_mouse_pos(const QPoint & p)
{
    set(float(p.x()) / float(width()));
    ((PaletteEditor*)parentWidget())->set_palette();
}

void ColorSlider::mousePressEvent(QMouseEvent * event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    set_mouse_pos(event->pos());
}

void ColorSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    set_mouse_pos(event->pos());
}

void ColorSlider::paintEvent(QPaintEvent * event)
{
    int w = width();
    int h = height();

    if (!pix || pix->width() != w) {
        delete pix;
        QImage img(w, 1, QImage::Format_RGB32);
        unsigned int * pixel = (unsigned int*)img.scanLine(0);
        const unsigned int * end = pixel + w;
        int x = 0;
        while (pixel < end) {
            QColor c;
            float hue = float(x) / float(w);
            c.setHsvF(hue, 1.0f, 1.0f);
            *pixel = c.rgb();
            pixel++;
            x++;
        }
        pix = new QPixmap(QPixmap::fromImage(img));
    }

    QPainter p(this);
    p.drawPixmap(0, 0, w, h, *pix);

    int p_x = int(value * w);
    draw_pointer(p_x, h / 2, p);
}

// PaletteGrid

PaletteGrid::PaletteGrid(MainWindow * parent)
: QWidget(parent), window(parent), palette_index(0)
{
    setFixedSize(192, 192);
    setAcceptDrops(true);
}

void PaletteGrid::paintEvent(QPaintEvent * event)
{
    VoxelFile * voxel = window->get_voxel();

    QPainter p(this);

    int x_size = width() / 8;
    int y_size = height() / 8;

    for (int i = 0; i <= VOXEL_AIR; i++) {
        int x = i % 8;
        int y = i / 8;
        int x1 = x * x_size;
        float y1 = y * y_size;
        unsigned char r, g, b;
        if (i == VOXEL_AIR || voxel == NULL)
            r = g = b = 20;
        else {
            RGBColor & color = global_palette[i];
            r = color.r;
            g = color.g;
            b = color.b;
        }

        const int outline = 1;
        if (palette_index == i) {
            p.fillRect(x1, y1, x_size, y_size, Qt::white);
            p.fillRect(x1 + outline, y1 + outline, 
                       x_size - outline * 2, y_size - outline * 2,
                       Qt::black);
            p.fillRect(x1 + outline * 2, y1 + outline * 2, 
                       x_size - outline * 4, y_size - outline * 4, 
                       QColor(r, g, b));
        } else {
            p.fillRect(x1, y1, x_size, y_size, QColor(r, g, b));
        }
    }
}

int PaletteGrid::get_index(const QPoint & p)
{
    int x_size = width() / 8;
    int y_size = height() / 8;
    int p_x = p.x() / x_size;
    int p_y = p.y() / y_size;
    return p_x + p_y * 8;
}

void PaletteGrid::mousePressEvent(QMouseEvent * event)
{
    if (event->button() != Qt::LeftButton)
        return;
    const QPoint & p = event->pos();
    drag_start = p;
    palette_index = get_index(p);
    ((PaletteEditor*)parentWidget())->set_current();
    update();
}

void PaletteGrid::mouseMoveEvent(QMouseEvent * event)
{
    VoxelFile * voxel = window->get_voxel();
    if (!voxel)
        return;
    if (!(event->buttons() & Qt::LeftButton))
        return;
     if ((event->pos() - drag_start).manhattanLength()
          < QApplication::startDragDistance())
         return;
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    RGBColor & col = global_palette[palette_index];
    QColor col2(col.r, col.g, col.b);
    mimeData->setColorData(col2);
    drag->setMimeData(mimeData);
    Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
}

void PaletteGrid::dragEnterEvent(QDragEnterEvent * event)
{
    if (!event->mimeData()->hasColor())
        return;
    event->acceptProposedAction();
}

void PaletteGrid::dropEvent(QDropEvent * event)
{
    VoxelFile * voxel = window->get_voxel();
    if (!voxel)
        return;
    int r, g, b;
    QColor color = event->mimeData()->colorData().value<QColor>();
    color.getRgb(&r, &g, &b);
    int i = get_index(event->pos());
    global_palette[i] = RGBColor(r, g, b);
    event->acceptProposedAction();
    update();
    window->model_changed();
}

// PaletteEditor

QSpinBox * PaletteEditor::create_color_spinbox()
{
    QSpinBox * box = new QSpinBox(this);
    box->setMinimum(0);
    box->setMaximum(255);
    connect(box, SIGNAL(valueChanged(int)), this, SLOT(rgb_changed()));
    return box;
}

PaletteEditor::PaletteEditor(MainWindow * parent)
: QWidget(parent), window(parent), ignore_rgb(false)
{
    QVBoxLayout * layout = new QVBoxLayout(this);

    color_space = new ColorSpace(this);
    color_slider = new ColorSlider(this);
    grid = new PaletteGrid(window);

    layout->addWidget(grid, 0, Qt::AlignHCenter);
    layout->addWidget(color_space);
    layout->addWidget(color_slider);

    QHBoxLayout * rgb_layout = new QHBoxLayout;
    rgb_layout->addWidget(create_label("RGB"));
    r_edit = create_color_spinbox();
    rgb_layout->addWidget(r_edit);
    g_edit = create_color_spinbox();
    rgb_layout->addWidget(g_edit);
    b_edit = create_color_spinbox();
    rgb_layout->addWidget(b_edit);

    layout->addLayout(rgb_layout);

    setLayout(layout);

    set_current();
}

RGBColor & PaletteEditor::get_palette_color()
{
    return global_palette[grid->palette_index];
}

void PaletteEditor::set_current()
{
    if (!window->get_voxel())
        return;
    RGBColor & col = get_palette_color();
    QColor color(col.r, col.g, col.b);
    qreal h, s, v;
    color.getHsvF(&h, &s, &v);
    color_space->set_hsv(h, s, v);
    color_slider->set(h);
    if (!ignore_rgb) {
        ignore_rgb = true;
        r_edit->setValue(col.r);
        g_edit->setValue(col.g);
        b_edit->setValue(col.b);
        ignore_rgb = false;
    }
    grid->update();
}

void PaletteEditor::set_palette()
{
    QColor col = QColor::fromHsvF(color_space->hue, 
                                  color_space->sat, 
                                  color_space->val);
    RGBColor & pal = get_palette_color();
    pal.r = col.red();
    pal.g = col.green();
    pal.b = col.blue();
    ignore_rgb = true;
    r_edit->setValue(pal.r);
    g_edit->setValue(pal.g);
    b_edit->setValue(pal.b);
    ignore_rgb = false;
    grid->update();
    window->model_changed();
}

int PaletteEditor::get_palette_index()
{
    return grid->palette_index;
}

void PaletteEditor::reset_palette()
{
}

void PaletteEditor::rgb_changed()
{
    if (ignore_rgb)
        return;
    RGBColor & pal = get_palette_color();
    pal.r = r_edit->value();
    pal.g = g_edit->value();
    pal.b = b_edit->value();
    ignore_rgb = true;
    set_current();
    ignore_rgb = false;
    window->model_changed();
}
