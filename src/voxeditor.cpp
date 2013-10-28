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

#include "voxel.h"
#include "voxeditor.h"
#include "mainwindow.h"
#include "draw.h"
#include "modelproperties.h"
#include "collision.h"
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btGeometryUtil.h>

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QRubberBand>
#include <QApplication>
#include <QClipboard>

#define CAMERA_ROTATION_SPEED 0.25f
#define CAMERA_MOVE_SPEED 1.0f

#define GREY_R 75
#define GREY_G 75
#define GREY_B 75

#define DEFAULT_X_SIZE 16
#define DEFAULT_Y_SIZE 16
#define DEFAULT_Z_SIZE 16
#define COORD_LINE_SIZE 200.0f

// static variable

SelectedVoxels VoxelEditor::copied_list;

VoxelEditor::VoxelEditor(MainWindow * parent)
: QGLWidget(parent->gl_format, parent, parent->shared_gl), scale(10.0f), 
  rotate_x(-58.0f), rotate_z(-143.0f), window(parent), pos_arrows(0.05f)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    voxel = new VoxelFile();
    rubberband = new QRubberBand(QRubberBand::Rectangle);
    rubberband->setWindowOpacity((qreal)0.5);
    rubberband->setWindowFlags(Qt::ToolTip);
}

void VoxelEditor::load(const QString & filename)
{
    current_model = filename;
    if (!voxel->load(filename))
        reset();
    set_window_file_path(this, current_model);
}

void VoxelEditor::reset()
{
    voxel->reset(16, 16, 16);
    voxel->set_offset(-8, -8, 0);
    setWindowModified(true);
}

void VoxelEditor::clone(VoxelFile * other)
{
    voxel->clone(*other);
    setWindowModified(true);
}

void VoxelEditor::on_changed()
{
    update();
    setWindowModified(true);
}

VoxelEditor::~VoxelEditor()
{
    delete voxel;
}

void VoxelEditor::initializeGL()
{
    setup_opengl();

    model = voxel->get_model();
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    pos = vec3(0.0f, 0.0f, 0.0f);
}

void VoxelEditor::paintEvent(QPaintEvent * e)
{
    QGLWidget::paintEvent(e);
}

void VoxelEditor::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

    view_matrix = mat4();
    view_matrix = glm::translate(view_matrix, 
        vec3(width() / 2, height() / 2, 0.0f));
    view_matrix = glm::translate(view_matrix, pos);
    view_matrix = glm::scale(view_matrix, vec3(scale, scale, scale));
    view_matrix = glm::rotate(view_matrix, rotate_x, vec3(1.0f, 0.0f, 0.0f));
    view_matrix = glm::rotate(view_matrix, rotate_z, vec3(0.0f, 0.0f, 1.0f));
    mvp = projection_matrix * view_matrix;
    inverse_mvp = glm::inverse(mvp);

    multiply_matrix(view_matrix);

    glEnable(GL_LIGHTING);
    setup_lighting();
    model->draw_immediate();

    SelectedVoxels::const_iterator it;
    for (it = selected_list.begin(); it != selected_list.end(); it++) {
        const SelectedVoxel & v = *it;
        RGBColor & c = global_palette[v.v];
        draw_cube(v.x, v.y, v.z, v.x + 1.0f, v.y + 1.0f, v.z + 1.0f,
            c.r, c.g, c.b, 255);
        glDisable(GL_LIGHTING);
        draw_wireframe_cube(v.x, v.y, v.z, v.x + 1.0f, v.y + 1.0f, v.z + 1.0f,
            255, 255, 255, 255);
        glEnable(GL_LIGHTING);
    }

    glDisable(GL_LIGHTING);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    vec3 min = voxel->get_min();
    vec3 max = voxel->get_max();
    float x1 = min.x;
    float x2 = max.x;
    float y1 = min.y;
    float y2 = max.y;
    float z1 = min.z;
    float z2 = max.z;
    draw_wireframe_cube(x1, y1, z1, x2, y2, z2, 255, 255, 255, 255);

    // checkerboard
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    for (int x = 0; x < voxel->x_size; x++)
    for (int y = 0; y < voxel->y_size; y++) {
        float gl_x1 = x1 + float(x);
        float gl_x2 = gl_x1 + 1.0f;
        float gl_y1 = y1 + float(y);
        float gl_y2 = gl_y1 + 1.0f;
        unsigned char r, g, b;
        if ((x + y) % 2 == 0)
            r = g = b = 100;
        else
            r = g = b = 180;
        glColor4ub(r, g, b, 255);
        glVertex3f(gl_x1, gl_y1, z1 - 0.01f);
        glVertex3f(gl_x2, gl_y1, z1 - 0.01f);
        glVertex3f(gl_x2, gl_y2, z1 - 0.01f);
        glVertex3f(gl_x1, gl_y2, z1 - 0.01f);
    }
    glEnd();

    // draw axis lines
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glColor4ub(GREY_R, GREY_G, GREY_B, 255);
    glVertex3f(-COORD_LINE_SIZE, 0.0f, 0.0f);
    glVertex3f(0, 0.0f, 0.0f);
    glVertex3f(0.0f, -COORD_LINE_SIZE, 0.0f);
    glVertex3f(0.0f, 0, 0.0f);

    glColor4ub(RED_R, RED_G, RED_B, 255);
    glVertex3f(0, 0.0f, 0.0f);
    glVertex3f(COORD_LINE_SIZE, 0.0f, 0.0f);

    glColor4ub(BLUE_R, BLUE_G, BLUE_B, 255);
    glVertex3f(0.0f, 0, 0.0f);
    glVertex3f(0.0f, COORD_LINE_SIZE, 0.0f);

    glColor4ub(GREEN_R, GREEN_G, GREEN_B, 255);
    glVertex3f(0.0f, 0.0f, 0);
    glVertex3f(0.0f, 0.0f, COORD_LINE_SIZE);
    glEnd();

    glDisable(GL_DEPTH_TEST);

    if (selected_list.size() > 0)
        pos_arrows.draw();

    if (window->test_current_window(this)) {
        ReferencePoint * ref = window->model_properties->get_point();
        if (ref != NULL) {
            float x, y, z;
            x = float(ref->x);
            y = float(ref->y);
            z = float(ref->z);
            draw_cube(x, y, z, 0.5f, 127, 127, 127, 127);
            draw_cube(x, y, z, 0.2f, 0, 0, 255, 255);
        }
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void VoxelEditor::resizeGL(int w, int h)
{
    projection_matrix = glm::ortho(0.0f, float(w), 0.0f, float(h),
        -3000.0f, 3000.0f);
    viewport = vec4(0, 0, w, h);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    load_matrix(projection_matrix);
    glMatrixMode(GL_MODELVIEW);
}

void VoxelEditor::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    int tool = window->get_tool();
    switch (key) {
        case Qt::Key_C:
            if (e->modifiers() & Qt::ControlModifier)
                copy_selected();
            else
                pick_color();
            break;
        case Qt::Key_V:
            if (e->modifiers() & Qt::ControlModifier)
                paste();
            break;
        case Qt::Key_Insert:
            use_tool_primary(true);
            break;
        case Qt::Key_Delete:
            if (tool == POINTER_EDIT_TOOL)
                delete_selected();
            else
                use_tool_secondary(true);
            break;
        case Qt::Key_Left:
            window->set_animation_frame(false);
            break;
        case Qt::Key_Right:
            window->set_animation_frame(true);
            break;
    }
}

btCollisionObject * VoxelEditor::get_collision_object()
{
    static btCollisionObject * obj = new btCollisionObject();
    obj->setCollisionShape(voxel->get_shape());
    return obj;
}

ivec3 VoxelEditor::get_pos_vec(const vec3 & v)
{
    int x = int(v.x - voxel->x_offset);
    int y = int(v.y - voxel->y_offset);
    int z = int(v.z - voxel->z_offset);
    return ivec3(x, y, z);
}

void VoxelEditor::update_hit()
{
    has_hit = hit_floor = false;
    vec3 dir, pos;
    vec2 win_pos(last_pos.x(), height() - last_pos.y());
    get_window_ray(win_pos, inverse_mvp, viewport, pos, dir);
    btCollisionObject * obj = get_collision_object();

    btTransform from_trans;
    btVector3 from_vec = convert_vec(pos);
    from_trans.setIdentity();
    from_trans.setOrigin(from_vec);

    btTransform to_trans;
    btVector3 to_vec = convert_vec(pos + dir * float(1e12));
    to_trans.setIdentity();
    to_trans.setOrigin(to_vec);

    btTransform obj_trans;
    obj_trans.setIdentity();

    btCollisionWorld::ClosestRayResultCallback callback(from_vec, to_vec);

    btCollisionWorld::rayTestSingle(from_trans, to_trans, obj, 
        obj->getCollisionShape(), obj_trans, callback);
    if (!callback.hasHit()) {
        vec3 min = voxel->get_min();
        vec3 max = voxel->get_max();
        btStaticPlaneShape plane(btVector3(0.0f, 0.0f, 1.0f), min.z);
        obj->setCollisionShape(&plane);
        btCollisionWorld::rayTestSingle(from_trans, to_trans, obj, 
            obj->getCollisionShape(), obj_trans, callback);
        if (!callback.hasHit())
            return;
        float x = callback.m_hitPointWorld.x();
        float y = callback.m_hitPointWorld.y();
        if (x >= max.x || x < min.x ||
            y >= max.y || y < min.y)
            return;
        hit_floor = true;
    }
    vec3 hit_normal = convert_vec(callback.m_hitNormalWorld);
    vec3 hit_pos = convert_vec(callback.m_hitPointWorld);
    hit_next = get_pos_vec(hit_pos + hit_normal * 0.1f);
    hit_block = get_pos_vec(hit_pos - hit_normal * 0.1f);
    has_hit = true;
}

void VoxelEditor::copy_selected()
{
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->clear();
    copied_list = selected_list;
    window->set_status("Copied voxels");
}

void VoxelEditor::delete_selected()
{
    selected_list.clear();
    window->set_status("Deleted voxels");
    on_changed();
}

void VoxelEditor::paste()
{
    deselect();

    // test for image in clipboard
    QClipboard * clipboard = QApplication::clipboard();
    QImage img = clipboard->image();
    if (img.isNull()) {
        // paste whatever we have in the internal copied list
        selected_list = copied_list;
    } else {
        // paste image from clipboard
        selected_list.clear();
        SelectedVoxel voxel;
        for (int x = 0; x < img.width(); x++)
        for (int y = 0; y < img.height(); y++) {
            QColor pixel = QColor(img.pixel(x, y));
            if (pixel.alpha() <= 0)
                continue;
            RGBColor c(pixel.red(), pixel.green(), pixel.blue());
            unsigned char cc = VoxelFile::get_closest_index(c);
            voxel.x = x;
            voxel.y = 0;
            voxel.z = img.height() - y;
            voxel.v = cc;
            selected_list.push_back(voxel);
        }
    }
    window->set_status("Pasted voxels");
    on_changed();
}

void VoxelEditor::deselect()
{
    SelectedVoxels::const_iterator it;
    for (it = selected_list.begin(); it != selected_list.end(); it++) {
        const SelectedVoxel & v = *it;
        voxel->set(v.x - voxel->x_offset,
                   v.y - voxel->y_offset, 
                   v.z - voxel->z_offset, 
                   v.v);
    }
    selected_list.clear();
    voxel->reset_shape();
    update();
}

void add_frustum_vertices(int x, int y, const mat4 & inverse_mvp,
                         const vec4 & viewport, 
                         btAlignedObjectArray<btVector3> & vertices)
{
    vec2 win(x, y);
    vec3 p;
    p = unproject(vec3(win, 0.0f), inverse_mvp, viewport);
    vertices.push_back(convert_vec(p));
    p = unproject(vec3(win, 1.0f), inverse_mvp, viewport);
    vertices.push_back(convert_vec(p));
}

void VoxelEditor::update_drag()
{
    deselect();

    int x1, y1, x2, y2;
    x1 = start_drag.x();
    y1 = height() - start_drag.y();
    x2 = last_pos.x();
    y2 = height() - last_pos.y();

    btAlignedObjectArray<btVector3> vertices;

    add_frustum_vertices(x1, y1, inverse_mvp, viewport, vertices);
    add_frustum_vertices(x2, y1, inverse_mvp, viewport, vertices);
    add_frustum_vertices(x2, y2, inverse_mvp, viewport, vertices);
    add_frustum_vertices(x1, y2, inverse_mvp, viewport, vertices);

    btAlignedObjectArray<btVector3> bt_planes;

    btGeometryUtil::getPlaneEquationsFromVertices(vertices, bt_planes);

    if (bt_planes.size() != 6)
        return;

    vec4 planes[6];

    for (int i = 0; i < 6; i++)
        planes[i] = vec4(bt_planes[i].x(),
                         bt_planes[i].y(),
                         bt_planes[i].z(),
                         bt_planes[i].w());
    
    vec3 global_min, global_max;
    bool global_set = false;

    for (int x = 0; x < voxel->x_size; x++)
    for (int y = 0; y < voxel->y_size; y++)
    for (int z = 0; z < voxel->z_size; z++) {
        unsigned char & v = voxel->get(x, y, z);
        if (v == VOXEL_AIR)
            continue;
        int x2, y2, z2;
        x2 = x + voxel->x_offset;
        y2 = y + voxel->y_offset;
        z2 = z + voxel->z_offset;
        vec3 min(x2, y2, z2);
        vec3 max = min + vec3(1.0f);
        if (!test_aabb_frustum(min, max, planes))
            continue;
        selected_list.push_back(SelectedVoxel(x2, y2, z2, v));
        v = VOXEL_AIR;
        if (!global_set) {
            global_min = min;
            global_max = max;
            global_set = true;
        } else {
            global_min = glm::min(global_min, min);
            global_max = glm::max(global_max, max);
        }
    }

    pos_arrows.set_pos((global_min + global_max) * 0.5f);

    update();
}

void VoxelEditor::mousePressEvent(QMouseEvent *event)
{
    last_pos = event->pos();
    if (event->modifiers())
        return;

    if (event->button() == Qt::LeftButton)
        use_tool_primary(true);
    else if (event->button() == Qt::RightButton)
        use_tool_secondary(true);
}

void VoxelEditor::offset_selected(int dx, int dy, int dz)
{
    if (dx == 0 && dy == 0 && dz == 0)
        return;
    SelectedVoxels::iterator it;
    for (it = selected_list.begin(); it != selected_list.end(); it++) {
        SelectedVoxel & v = *it;
        v.x += dx;
        v.y += dy;
        v.z += dz;
    }
    on_changed();
}

struct FillNode
{
    int x, y, z;

    FillNode(int x, int y, int z) 
    : x(x), y(y), z(z)
    {
    }
};

inline void add_node(int x, int y, int z, VoxelFile * voxel,
                     unsigned char col, std::vector<FillNode> & nodes)
{
    if (x < 0 || x >= voxel->x_size ||
        y < 0 || y >= voxel->y_size ||
        z < 0 || z >= voxel->z_size)
        return;
    if (voxel->get(x, y, z) != col)
        return;
    nodes.push_back(FillNode(x, y, z));
}

void VoxelEditor::flood_fill(int x, int y, int z)
{
    unsigned char col = voxel->get(x, y, z);
    if (col == VOXEL_AIR)
        return;
    unsigned char new_col = window->get_palette_index();
    if (col == new_col)
        return;
    std::vector<FillNode> nodes;

    add_node(x, y, z, voxel, col, nodes);
    
    while (!nodes.empty()) {
        FillNode & node = nodes.back();
        x = node.x;
        y = node.y;
        z = node.z;
        nodes.pop_back();
        unsigned char & c = voxel->get(x, y, z);
        if (c == new_col)
            continue;
        c = new_col;

        add_node(x, y, z - 1, voxel, col, nodes);
        add_node(x, y - 1, z, voxel, col, nodes);
        add_node(x, y + 1, z, voxel, col, nodes);
        add_node(x - 1, y, z, voxel, col, nodes);
        add_node(x + 1, y, z, voxel, col, nodes);
        add_node(x, y, z + 1, voxel, col, nodes);
    }
}

void VoxelEditor::use_tool_primary(bool click)
{
    int tool = window->get_tool();

    if ((tool == BLOCK_EDIT_TOOL || tool == BUCKET_EDIT_TOOL) && !click)
        return;

    if (tool == POINTER_EDIT_TOOL) {
        vec2 mouse(last_pos.x(), height() - last_pos.y());
        vec3 pos, dir;
        get_window_ray(mouse, inverse_mvp, viewport, pos, dir);

        if (selected_list.size() > 0) {
            if (click)
                pos_arrows.on_mouse_press(pos, dir);
            else {
                pos_arrows.on_mouse_move(pos, dir);
                vec3 add = pos_arrows.get(1.0f);
                int dx = int(add.x);
                int dy = int(add.y);
                int dz = int(add.z);
                offset_selected(dx, dy, dz);
            }
            if (pos_arrows.pan != NONE_CONE) {
                update();
                return;
            }
        }

        if (click) {
            start_drag = last_pos;
            rubberband->move(mapToGlobal(start_drag));
            rubberband->resize(0, 0);
            rubberband->show();
        } else {
            rubberband->setGeometry(
                QRect(mapToGlobal(start_drag), 
                mapToGlobal(last_pos)).normalized());
        }
        update_drag();
        return;
    }

    if (!has_hit)
        return;

    int hit_x, hit_y, hit_z;
    if (tool == BLOCK_EDIT_TOOL) {
        hit_x = hit_next.x;
        hit_y = hit_next.y;
        hit_z = hit_next.z;
    } else if (tool == PENCIL_EDIT_TOOL || tool == BUCKET_EDIT_TOOL) {
        if (hit_floor)
            return;
        hit_x = hit_block.x;
        hit_y = hit_block.y;
        hit_z = hit_block.z;
    } else
        return;

    if (tool == BUCKET_EDIT_TOOL)
        flood_fill(hit_x, hit_y, hit_z);
    else
        voxel->set(hit_x, hit_y, hit_z, window->get_palette_index());
    voxel->reset_shape();

    update_hit();
    on_changed();
}

void VoxelEditor::pick_color()
{
    int i = voxel->get(hit_block.x, hit_block.y, hit_block.z);
    window->set_palette_index(i);
}

void VoxelEditor::use_tool_secondary(bool click)
{
    if (!has_hit)
        return;
    if (!click)
        return;

    int tool = window->get_tool();

    if (tool == BLOCK_EDIT_TOOL) {
        voxel->set(hit_block.x, hit_block.y, hit_block.z, VOXEL_AIR);
        voxel->reset_shape();
        update_hit();
        on_changed();
    } else if (tool == PENCIL_EDIT_TOOL) {
        pick_color();
    }
}

void VoxelEditor::mouseMoveEvent(QMouseEvent * e)
{
    QPoint dpos = e->pos() - last_pos;
    last_pos = e->pos();

    update_hit();

    float dx = dpos.x();
    float dy = dpos.y();

    bool left = e->buttons() & Qt::LeftButton;
    bool right = e->buttons() & Qt::RightButton;

    if (e->modifiers() == Qt::AltModifier) {
        if (left) { 
            dx *= CAMERA_ROTATION_SPEED;
            dy *= CAMERA_ROTATION_SPEED;
            rotate_x = std::min<float>(0.0f, std::max<float>(-180.0f,
                rotate_x + float(dy)));
            rotate_z += float(dx);
        } else if (right) {
            dx *= CAMERA_MOVE_SPEED;
            dy *= CAMERA_MOVE_SPEED;
            pos.x += float(dx);
            pos.y -= float(dy);
        }
        update();
    } else {
        if (left) {
            use_tool_primary(false);
        } else {
            use_tool_secondary(false);
        }
    }
}

void VoxelEditor::mouseReleaseEvent(QMouseEvent * e)
{
    rubberband->hide();
    if (pos_arrows.pan != NONE_CONE) {
        pos_arrows.on_mouse_release();
        update();
    }
}

void VoxelEditor::wheelEvent(QWheelEvent * e)
{
    QPoint pixels = e->pixelDelta();
    QPoint degrees = e->angleDelta();
    float scroll = 0.0f;

    if (!pixels.isNull()) {
        scroll = pixels.y();
    } else if (!degrees.isNull()) {
        QPoint steps = degrees / 8 / 15;
        scroll = steps.y();
    }

    scale = std::max<float>(0.1f, scale + float(scroll) * 0.25f);

    e->accept();

    update();
}

void VoxelEditor::closeEvent(QCloseEvent *event)
{
    if (!isWindowModified()) {
        event->accept();
        return;
    }
    QMessageBox * msg = new QMessageBox(this);
    msg->setText("Do you want to save your changes?");
    msg->setStandardButtons(
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msg->setDefaultButton(QMessageBox::Save);
    int ret = msg->exec();
    if (ret == QMessageBox::Save) {
        save();
        event->accept();
    } else if (ret == QMessageBox::Discard) {
        event->accept();
    } else
        event->ignore();
}

void VoxelEditor::save()
{
    if (current_model.isEmpty()) {
        save_as();
        return;
    }
    voxel->save(current_model);
    setWindowModified(false);
}

void VoxelEditor::save_as()
{
    current_model = get_model_name(this, true);
    set_window_file_path(this, current_model);
    if (current_model.isEmpty())
        // ok, user really doesn't want to save the model
        return;
    save();
}