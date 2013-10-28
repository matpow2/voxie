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

#include "glm.h"
#include "editorcommon.h"

#include <QGLWidget>

class VoxelFile;
class VoxelModel;
class MainWindow;
class btCollisionObject;
class QPaintEvent;
class QRubberBand;

class SelectedVoxel
{
public:
    int x, y, z;
    unsigned char v;

    SelectedVoxel(int x = 0, int y = 0, int z = 0, unsigned char v = 0)
    : x(x), y(y), z(z), v(v)
    {
    }
};

typedef std::vector<SelectedVoxel> SelectedVoxels;

class VoxelEditor : public QGLWidget
{
    Q_OBJECT

public:
    QString model_name;

    MainWindow * window;

    VoxelFile * voxel;
    VoxelModel * model;

    mat4 projection_matrix, view_matrix, mvp, inverse_mvp;
    vec4 viewport;

    QString current_model;

    vec3 pos;
    float scale;
    float rotate_x, rotate_z;

    bool has_hit, hit_floor;
    ivec3 hit_next, hit_block;

    QRubberBand * rubberband;
    QPoint start_drag;
    SelectedVoxels selected_list;
    static SelectedVoxels copied_list;
    PositionArrows pos_arrows;

    QPoint last_pos;

    VoxelEditor(MainWindow * parent);
    void load(const QString & name);
    void reset();
    void clone(VoxelFile * other);
    void on_changed();
    void update_hit();
    ~VoxelEditor();

protected:
    void initializeGL();
    void paintGL();
    void paintEvent(QPaintEvent * e);
    void resizeGL(int w, int h);
    void keyPressEvent(QKeyEvent *e);
    btCollisionObject * get_collision_object();
    ivec3 get_pos_vec(const vec3 & v);
    void update_drag();
    void offset_selected(int dx, int dy, int dz);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void pick_color();
    void use_tool_primary(bool click);
    void use_tool_secondary(bool click);
    void wheelEvent(QWheelEvent * e);
    void closeEvent(QCloseEvent *event);
    void deselect();
    void copy_selected();
    void delete_selected();
    void paste();
    void flood_fill(int x, int y, int z);

public slots:
    void save();
    void save_as();
};