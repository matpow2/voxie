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

#include "include_gl.h"
#include "glm.h"
#include "editorcommon.h"
#include "draw.h"
#include "collision.h"
#include <QObject>
#include <QLabel>
#include <QInputDialog>
#include <QEvent>
#include <QApplication>
#include <QFileDialog>

QLabel * create_label(const QString & text)
{
    QLabel * label = new QLabel(text);
    label->setFixedWidth(50);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

std::string convert_str(const QString & str)
{
    return str.toUtf8().constData();
}

QString get_model_name(QWidget * parent, bool save)
{
    QString caption = QObject::tr("Model file dialog");
    QString filter = QObject::tr("Voxie model (*.vxi);;All Files (*)");
    QString text;
    if (save)
        text = QFileDialog::getSaveFileName(parent, caption, "", filter);
    else
        text = QFileDialog::getOpenFileName(parent, caption, "", filter);
    return text;
}

void set_window_file_path(QWidget * w, const QString & name)
{
    w->setWindowFilePath(name);
    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(w, &e);
}

#define CONE_HEIGHT 75.0f
#define CONE_RADIUS 10.0f
#define LINE_LEN 100.0f
#define CONE_START (LINE_LEN + CONE_HEIGHT / 2.0f)

inline btTransform get_pos_trans(const vec3 & p)
{
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(convert_vec(p));
    return trans;
}

struct ArrowResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
    int index;

    ArrowResultCallback(const btVector3 & a, const btVector3 & b)
    : btCollisionWorld::ClosestRayResultCallback(a, b), index(NONE_CONE)
    {
    }

    btScalar addSingleResult(btCollisionWorld::LocalRayResult & r, bool n)
    {
        index = r.m_localShapeInfo->m_triangleIndex;
        return btCollisionWorld::ClosestRayResultCallback::addSingleResult(
            r, n);
    }
};

// PositionArrows

PositionArrows::PositionArrows(float scale)
: pan(NONE_CONE), shape(NULL)
{
    set_scale(scale);
}

PositionArrows::~PositionArrows()
{

}

void PositionArrows::set_scale(float value)
{
    scale = value;
    if (shape != NULL) {
        for (int i = 0; i < shape->getNumChildShapes(); i++) {
            delete shape->getChildShape(i);
        }
        delete shape;
    }

    btConeShapeX * x_cone = new btConeShapeX(CONE_RADIUS, CONE_HEIGHT);
    btConeShape * y_cone = new btConeShape(CONE_RADIUS, CONE_HEIGHT);
    btConeShapeZ * z_cone = new btConeShapeZ(CONE_RADIUS, CONE_HEIGHT);
    btCompoundShape * shape = new btCompoundShape();
    shape->addChildShape(get_pos_trans(vec3(CONE_START, 0.0f, 0.0f)), x_cone);
    shape->addChildShape(get_pos_trans(vec3(0.0f, CONE_START, 0.0f)), y_cone);
    shape->addChildShape(get_pos_trans(vec3(0.0f, 0.0f, CONE_START)), z_cone);
    shape->setLocalScaling(btVector3(scale, scale, scale));
    this->shape = shape;
}

void PositionArrows::update(const vec3 & ray_pos, const vec3 & ray_dir)
{
    test_ray_plane(ray_pos, ray_dir, pos, normal, last);
}

void PositionArrows::on_mouse_press(const vec3 & ray_pos, const vec3 & ray_dir)
{
    pan = ray_test(ray_pos, ray_dir);
    switch (pan) {
        case X_CONE:
            normal = vec3(0.0f, -ray_dir.y, -ray_dir.z);
            break;
        case Y_CONE:
            normal = vec3(-ray_dir.x, 0.0f, -ray_dir.z);
            break;
        case Z_CONE:
            normal = vec3(-ray_dir.x, -ray_dir.y, 0.0f);
            break;
    }
    if (pan != NONE_CONE) {
        update(ray_pos, ray_dir);
        return;
    }
}

void PositionArrows::on_mouse_move(const vec3 & ray_pos, const vec3 & ray_dir)
{
    if (pan == NONE_CONE)
        return;
    vec3 old_pan = last;
    update(ray_pos, ray_dir);
    vec3 new_add = last - old_pan;
    switch (pan) {
        case X_CONE:
            new_add = vec3(new_add.x, 0.0f, 0.0f);
            break;
        case Y_CONE:
            new_add = vec3(0.0f, new_add.y, 0.0f);
            break;
        case Z_CONE:
            new_add = vec3(0.0f, 0.0f, new_add.z);
            break;
    }
    add += new_add;
}

void PositionArrows::on_mouse_release()
{
    pan = NONE_CONE;
}

void PositionArrows::set_pos(const vec3 & p)
{
    pos = p;
}

vec3 PositionArrows::get(float grid)
{
    vec3 p;
    if (grid == 0.0f)
        p = add;
    else
        p = glm::round(add / grid) * grid;
    if (p != vec3(0.0f)) {
        add = vec3(0.0f);
        pos += p;
    }
    return p;
}

int PositionArrows::ray_test(const vec3 & ray_pos, const vec3 & ray_dir)
{
    static btCollisionObject * obj = new btCollisionObject();
    obj->setCollisionShape(shape);

    btTransform obj_trans = get_pos_trans(pos);

    btTransform from_trans;
    btVector3 from_vec = convert_vec(ray_pos);
    from_trans.setIdentity();
    from_trans.setOrigin(from_vec);

    btTransform to_trans;
    btVector3 to_vec = convert_vec(ray_pos + ray_dir * 5000.0f);
    to_trans.setIdentity();
    to_trans.setOrigin(to_vec);

    ArrowResultCallback callback(from_vec, to_vec);

    btCollisionWorld::rayTestSingle(from_trans, to_trans, obj, shape, obj_trans,
        callback);

    return callback.index;
}

void PositionArrows::draw()
{
    const static int cone_n = 6;

    int x_r = RED_R;
    int x_g = RED_G;
    int x_b = RED_B;

    int y_r = BLUE_R;
    int y_g = BLUE_G;
    int y_b = BLUE_B;

    int z_r = GREEN_R;
    int z_g = GREEN_G;
    int z_b = GREEN_B;

    switch (pan) {
        case X_CONE:
            x_r = x_g = x_b = 255;
            break;
        case Y_CONE:
            y_r = y_g = y_b = 255;
            break;
        case Z_CONE:
            z_r = z_g = z_b = 255;
            break;
    }

    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glScalef(scale, scale, scale);

    glLineWidth(1.0f);
    glBegin(GL_LINES);

    glColor4ub(x_r, x_g, x_b, 255);
    glVertex3f(0, 0.0f, 0.0f);
    glVertex3f(LINE_LEN, 0.0f, 0.0f);

    glColor4ub(y_r, y_g, y_b, 255);
    glVertex3f(0.0f, 0, 0.0f);
    glVertex3f(0.0f, LINE_LEN, 0.0f);

    glColor4ub(z_r, z_g, z_b, 255);
    glVertex3f(0.0f, 0.0f, 0);
    glVertex3f(0.0f, 0.0f, LINE_LEN);
    glEnd();

    glColor4ub(x_r, x_g, x_b, 255);
    vec3 x_cone(LINE_LEN, 0.0f, 0.0f);
    draw_cone(x_cone, x_cone + vec3(CONE_HEIGHT, 0.0f, 0.0f), CONE_RADIUS,
              cone_n);

    glColor4ub(y_r, y_g, y_b, 255);
    vec3 y_cone(0.0f, LINE_LEN, 0.0f);
    draw_cone(y_cone, y_cone + vec3(0.0f, CONE_HEIGHT, 0.0f), CONE_RADIUS,
              cone_n);

    glColor4ub(z_r, z_g, z_b, 255);
    vec3 z_cone(0.0f, 0.0f, LINE_LEN);
    draw_cone(z_cone, z_cone + vec3(0.0f, 0.0f, CONE_HEIGHT), CONE_RADIUS,
              cone_n);

    glPopMatrix();
}