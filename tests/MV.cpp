#include <CSRMatrix.h>
#include <Mesh.h>
#include <iostream>
#include <cholesky.h>

#include <QApplication>
#include "MatrixVisualizer.h"

using namespace FEMLib;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 可选：强制使用更高版本的 OpenGL (3.3或以上)
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 5);
    QSurfaceFormat::setDefaultFormat(format);

    // 创建可视化窗口
    MatrixVisualizer vis;
    vis.resize(800, 800);
    vis.show();
    
    Mesh mesh(10, MeshType::CUBE);

    CSRMatrix M(mesh);

    // 传入稀疏矩阵信息
    vis.setMatrix(M.rows, M.cols, M.row_offset.to_stdVector(), M.elm_idx.to_stdVector());

    // 如果想测试保存图像
    vis.saveImage("matrix.png");

    return app.exec();
}
