#ifndef UTECH_CANVAS_3D_H
#define UTECH_CANVAS_3D_H

#include <QVector>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkSTLReader.h>
#include <vtkOBJReader.h>
#include <QVTKOpenGLWidget.h>
#include <QVTKWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkPLYReader.h>
#include <vtkAxesActor.h>
#include <vtkTransform.h>
#include <vtkDelaunay2D.h>
#include <vtkRenderWindowInteractor.h>
#include "utech_point.h"
#include "pointpicker_interactor_style.h"


class UTechCanvas3d : public QVTKOpenGLWidget
{
    Q_OBJECT

public:
    UTechCanvas3d(int width,int height);
    ~UTechCanvas3d();
    int doDisplayAscFile(QString path,int colorType);
    int doDisplayAscFileTriangulation(QString path,int colorType,int dataType);
    int doDisplayPlyFile(QString path,int colorType);
    int doDisplayPcdFile(QString path,int colorType);
    int doDisplayObjFile(QString path,int colorType);
    int doDisplayStlFile(QString path);
    int doDisplayPointCloud(int colorType);
    int doDisplayPointCloud(float* pCloud,int sizeNum,int colorType);
    int doDisplayPointCloudTriangulation(float* pCloud,int sizeNum);
    int doDisplayPointCloudTexture(float* pCloud,unsigned char* pTexture,int sizeNum);
    int doDisplayAxes(int xOffest,int yOffest,int zOffest,double xAngle,double yAngle,double zAngle,int type);
    int doDisplayText(int positionX,int positionY,int fontSize,const char* text);
    int addDisplaySphere(double centerX,double centerY,double centerZ,double radius);
    int addDisplayPlane(double center[3],double normal[3]);
    int addDisplayLine(float point1[3],float point2[3]);
    int addDipslayPoint(float point[3]);
    int addDisplayPointCloud(float* pCloud,int sizeNum,int colorType);
    int addDisplayPointCloud(std::vector<float>& pCloud,int colorType);
    int addDisplayPointCloudTexture(std::vector<float>& pCloud);
    int addDisplayAscFile(QString path,int colorTypes);
    int addDisplayAscFileTriangulation(QString path,int colorType,int dataType);
    int addDisplayPlyFile(QString path,int colorType);
    int addDisplayStlFile(QString path);
    int setPickStyle(int enabled,float* data,int length);
    int getPickX(double& x);
    int getPickY(double& y);
    int getPickZ(double& z);
    int clearAll();
    int affichage();
    int removeAllActor();

private:
    void init(int width,int height);
    vtkActor* stlFileReader(QString path,int mode,int r,int g,int b);
    vtkActor* plyFileReader(QString path,int mode,int r,int g,int b);
    vtkActor* pointsReader(QString path,int colorType);
    vtkActor* pointsReader(int colorType);
    vtkActor* pointsReaderTriangulation(QString path,int colorType,int dataType);
    QVector<UTechPoint*>* doReadPointCloudFile(QString path,int dataType);
    void readPointCloudFile(QString path,int dataType);
    void readPointCloudFile(const char* path);
public:
    vtkPolyData* point_cloud_data_;
    float* select_points_;
    int select_points_length_;
    float* current_points_;
    int current_points_length_;
    bool is_display_line_;

private:
    int canvas_3d_width_;
    int canvas_3d_height_;
    float z_min_;
    float z_max_;
    vtkSmartPointer<vtkRenderer> renderer_;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> render_window_;
    vtkSmartPointer<vtkCamera> camera_;
    vtkActor* point_cloud_actor_;
    QVector<vtkActor*> actor_set_;
    vtkActor* stl_actor_;
    vtkSmartPointer<vtkTransform> transform_;
    vtkAxesActor* axes_;
    vtkRenderWindowInteractor* render_window_interactor_;

    //PointPickerInteractorStyle pick_style_;
};

#endif // UTECH_CANVAS_3D_H
