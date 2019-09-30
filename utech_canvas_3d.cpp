#include <QFile>
#include <QString>
#include <QDebug>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkRendererCollection.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkDataSetMapper.h>
#include <vtkPointPicker.h>
#include <vtkPropPicker.h>
#include <vtkAreaPicker.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkObjectFactory.h>
#include <vtkPlanes.h>
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkTextProperty.h"
#include <vtkTextActor.h>
#include <vtkLineSource.h>
#include <vtkPointSource.h>
#include <QTextCodec>
#include "utech_canvas_3d.h"
#include "UI.h"
#include "PCLIO.h"



class PointPickerInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static PointPickerInteractorStyle* New();
    vtkTypeMacro(PointPickerInteractorStyle, vtkInteractorStyleTrackballCamera);

    virtual void OnLeftButtonDown()
    {
         if(secondActor != NULL)
         {
             this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(firstActor);
             this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(secondActor);
             firstSphereSource->Delete();
             firstSphereSource = NULL;
             firstActor->Delete();
             firstActor = NULL;
             secondSphereSource->Delete();
             secondSphereSource = NULL;
             secondActor->Delete();
             secondActor = NULL;
         }
//        std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
         this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
            this->Interactor->GetEventPosition()[1],
            0,  // always zero.
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

      //  double picked[3];
         this->Interactor->GetPicker()->GetPickPosition(secondPicked);
       // std::cout << "Picked value: " << secondPicked[0] << " " << secondPicked[1] << " " << secondPicked[2] << std::endl;
        //this->Interactor->GetPicker()->Pick3DPoint(secondPicked,this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
         std::cout << "Picked value: " << secondPicked[0] << " " << secondPicked[1] << " " << secondPicked[2] << std::endl;
        float capture[3] = {secondPicked[0],secondPicked[1],secondPicked[2]};
        std::vector<float> outData;

        int ret = ScreenCapturePoint(capture, modelData,modelLength, outData);

//        if(secondPicked[0]>modelRangeX[0] && secondPicked[0]<modelRangeX[1] && secondPicked[1]>modelRangeY[0] && secondPicked[1]<modelRangeY[1]
//                && secondPicked[2]>modelRangeZ[0] && secondPicked[2]<modelRangeZ[1])
        if(ret == 0)
        {
//            secondPicked[0] = outData.at(0);
//            secondPicked[1] = outData.at(1);
//            secondPicked[2] = outData.at(2);

            firstSphereSource = vtkSphereSource::New();
            firstSphereSource->SetRadius(1.0);
            firstSphereSource->Update();
            firstMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            firstMapper->SetInputConnection(firstSphereSource->GetOutputPort());
            firstActor = vtkActor::New();
            firstActor->SetMapper(firstMapper);
            firstActor->SetPosition(firstPicked);
            firstActor->SetScale(0.05);
            firstActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

            secondSphereSource = vtkSphereSource::New();
            secondSphereSource->SetRadius(1.0);
            secondSphereSource->Update();
            secondMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            secondMapper->SetInputConnection(secondSphereSource->GetOutputPort());
            secondActor = vtkActor::New();
            secondActor->SetMapper(secondMapper);
            secondActor->SetPosition(secondPicked);
            secondActor->SetScale(0.05);
            secondActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

            if(View3d->select_points_ !=NULL)
            {
                delete[] View3d->select_points_;
                View3d->select_points_ = NULL;
            }
            View3d->select_points_     =  new float[6];
            View3d->select_points_[0]  =  firstPicked[0];
            View3d->select_points_[1]  =  firstPicked[1];
            View3d->select_points_[2]  =  firstPicked[2];
            View3d->select_points_[3]  =  secondPicked[0];
            View3d->select_points_[4]  =  secondPicked[1];
            View3d->select_points_[5]  =  secondPicked[2];


            this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(firstActor);
            this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(secondActor);
            this->GetInteractor()->GetRenderWindow()->Render();
            firstPicked[0] = secondPicked[0];
            firstPicked[1] = secondPicked[1];
            firstPicked[2] = secondPicked[2];
            View3d->is_display_line_ = true;
        }

        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }

    void SetView(UTechCanvas3d* view)
    {
        this->View3d = view;
    }

    void setModelData(float* data,int length)
    {
        modelData = data;
        modelLength = length;
    }

public:
    double firstPicked[3]={0,0,0};
    double secondPicked[3]={0,0,0};


private:
    UTechCanvas3d* View3d;
    vtkSphereSource* firstSphereSource;
    vtkSphereSource* secondSphereSource;
    vtkActor* firstActor  = NULL;
    vtkActor* secondActor = NULL;
    vtkSmartPointer<vtkPolyDataMapper> firstMapper;
    vtkSmartPointer<vtkPolyDataMapper> secondMapper;
    float*  modelData;
    int    modelLength;
};

vtkStandardNewMacro(PointPickerInteractorStyle);


#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

// Define interaction style
class HighlightInteractorStyle : public vtkInteractorStyleRubberBandPick
{
  public:
    static HighlightInteractorStyle* New();
    vtkTypeMacro(HighlightInteractorStyle,vtkInteractorStyleRubberBandPick);

    HighlightInteractorStyle() : vtkInteractorStyleRubberBandPick()
    {
      if(SelectedActor != NULL)
      {
      SelectedActor->Delete();
      SelectedActor = NULL;
      }
      this->SelectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
      this->SelectedActor = vtkSmartPointer<vtkActor>::New();
      this->SelectedActor->SetMapper(SelectedMapper);
    }

    virtual void OnLeftButtonUp()
    {
      // Forward events
      vtkInteractorStyleRubberBandPick::OnLeftButtonUp();

      if(this->CurrentMode == VTKISRBP_SELECT)
      {
        vtkPlanes* frustum = static_cast<vtkAreaPicker*>(this->GetInteractor()->GetPicker())->GetFrustum();
        vtkSmartPointer<vtkExtractPolyDataGeometry> extractPolyDataGeometry = vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
#if VTK_MAJOR_VERSION <= 5
        extractPolyDataGeometry->SetInput(this->PolyData);
#else
        extractPolyDataGeometry->SetInputData(this->PolyData);
#endif
        extractPolyDataGeometry->SetImplicitFunction(frustum);
        extractPolyDataGeometry->Update();
        this->SelectedPoints = extractPolyDataGeometry->GetOutput()->GetPoints();
        if(View3d->select_points_ !=NULL)
        {
            delete[] View3d->select_points_;
            View3d->select_points_ = NULL;
        }
        View3d->select_points_length_ =  SelectedPoints->GetNumberOfPoints();
        View3d->select_points_ = new float[SelectedPoints->GetNumberOfPoints()*3];
        for(int i = 0; i< SelectedPoints->GetNumberOfPoints();++i)
        {
            auto point = SelectedPoints->GetPoint(i);
            View3d->select_points_[i*3]    =  point[0];
            View3d->select_points_[i*3+1]  =  point[1];
            View3d->select_points_[i*3+2]  =  point[2];
        //    std::cout << "point[" << i << "] " << point[0] << " " << point[1] << " " << point[2] << std::endl;
        }

      //  std::cout << "Extracted " << extractPolyDataGeometry->GetOutput()->GetNumberOfCells() << " cells." << std::endl;

#if VTK_MAJOR_VERSION <= 5
        this->SelectedMapper->SetInputConnection(
          extractPolyDataGeometry->GetOutputPort());
#else
        this->SelectedMapper->SetInputData(extractPolyDataGeometry->GetOutput());
#endif
        this->SelectedMapper->ScalarVisibilityOff();

//        vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(selected->GetPointData()->GetArray("OriginalIds"));

        this->SelectedActor->GetProperty()->SetColor(1.0, 0.0, 0.0); //(R,G,B)
        this->SelectedActor->GetProperty()->SetPointSize(5);
        this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(SelectedActor);
        this->GetInteractor()->GetRenderWindow()->Render();
        this->HighlightProp(NULL);
      }
    }

    virtual void OnChar()
    {
        // Forward events
        vtkInteractorStyleRubberBandPick::OnChar();
        switch (this->GetInteractor()->GetKeyCode()) {
        case 'd':   //delete
//            for(int i = 0; i<3000;i++)
//            {
//                PolyData->BuildLinks();
//                PolyData->DeletePoint(i);
//                PolyData->DeleteLinks();
//                PolyData->Modified();
//            }
            break;
        default:
            break;
        }
    }

    void SetPolyData(UTechCanvas3d* view,vtkSmartPointer<vtkPolyData> polyData)
    {
        this->View3d = view;
        this->PolyData = polyData;
    }

    private:
        vtkSmartPointer<vtkPolyData> PolyData;
        vtkSmartPointer<vtkActor> SelectedActor = NULL;
        vtkSmartPointer<vtkDataSetMapper> SelectedMapper;
    public:
        vtkSmartPointer<vtkPoints> SelectedPoints;
        UTechCanvas3d* View3d;

};

vtkStandardNewMacro(HighlightInteractorStyle);


UTechCanvas3d::UTechCanvas3d(int width,int height)
{
    canvas_3d_width_   = width;
    canvas_3d_height_  = height;
    init(width,height);
}

UTechCanvas3d::~UTechCanvas3d()
{

}

int UTechCanvas3d::doDisplayAscFile(QString path,int colorType)
{
    if(path == " ")
    {
        return  -1;
    }

    if(point_cloud_actor_ != NULL)
    {
       point_cloud_actor_->Delete();
       point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = pointsReader(path,colorType);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayAscFileTriangulation(QString path, int colorType, int dataType)
{
    if(path == " ")
    {
        return -1;
    }
    if(point_cloud_actor_ != NULL)
    {
        point_cloud_actor_->Delete();
        point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = pointsReaderTriangulation(path,colorType,dataType);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayPlyFile(QString path, int colorType)
{
    if(path == "")
    {
        return -1;
    }
    QTextCodec *code = QTextCodec::codecForName("gb2312");
    std::string name = code->fromUnicode(path).data();
    int dataLength = 0;
    PlyFileReader(name.c_str(),current_points_,dataLength);
    current_points_length_ = dataLength/3;
//    std::vector<float> outData;
//    ReadPLY(name.c_str(), outData);
//    if(current_points_ != NULL)
//    {
//        delete[] current_points_;
//        current_points_ = NULL;
//    }
//    current_points_ = new float[outData.size()];
//    for(int i=0;i<outData.size()/3;i++)
//    {
//       current_points_[i*3+0] = outData.at(i*3+0);
//       current_points_[i*3+1] = outData.at(i*3+1);
//       current_points_[i*3+2] = outData.at(i*3+2);
//    }
//    current_points_length_ = outData.size()/3;
    if(point_cloud_actor_ != NULL)
    {
       point_cloud_actor_->Delete();
       point_cloud_actor_ = NULL;
    }

    point_cloud_actor_ = pointsReader(colorType);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayPcdFile(QString path, int colorType)
{
    if(path == "")
    {
        return -1;
    }
    QTextCodec *code = QTextCodec::codecForName("gb2312");
    std::string name = code->fromUnicode(path).data();
    int dataLength = 0;
    PcdFileReader(name.c_str(),current_points_,dataLength);
    current_points_length_ = dataLength/3;

    if(point_cloud_actor_ != NULL)
    {
       point_cloud_actor_->Delete();
       point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = pointsReader(colorType);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayObjFile(QString path, int colorType)
{
    if(path == "")
    {
        return -1;
    }
    QTextCodec *code = QTextCodec::codecForName("gb2312");
    std::string name = code->fromUnicode(path).data();
    int dataLength = 0;
    qDebug() << "name:" << path << endl;
    ObjFileReader(name.c_str(),current_points_,dataLength);
    current_points_length_ = dataLength/3;
    qDebug() << "length" << current_points_length_ << endl;
    if(point_cloud_actor_ != NULL)
    {
       point_cloud_actor_->Delete();
       point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = pointsReader(colorType);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayStlFile(QString path)
{
    if(path == " ")
    {
        return -1;
    }
    if(stl_actor_ != NULL)
    {
        stl_actor_->Delete();
        stl_actor_ = NULL;
    }
    stl_actor_ = stlFileReader(path,1,175,38,238);
    renderer_->AddActor(stl_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayPointCloud(int colorType)
{
    if(point_cloud_actor_ != NULL)
    {
       point_cloud_actor_->Delete();
       point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = vtkActor::New();
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    int pointCount = current_points_length_;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    int gradientColor;
    for(int index=2; index<pointCount*3;index=index+3)
    {
        if(index == 2)
        {
           z_min_ =  z_max_ = current_points_[index];
        }
        if(z_min_> current_points_[index])
        {
            z_min_ = current_points_[index];
        }
        if(z_max_ < current_points_[index])
        {
            z_max_ = current_points_[index];
        }
    }

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        pointX = current_points_[cpt * 3 + 0];
        pointY = current_points_[cpt * 3 + 1];
        pointZ = current_points_[cpt * 3 + 2];
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        if(colorType == 0)  // blue
        {
           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
        }
        else if(colorType == 1)  //yellow
        {
           colors->InsertNextTuple3(255,255,0);
        }
        else if(colorType == 2)    //purple
        {
           if(cpt*175.0/pointCount <= 175.0/3)
           {
               gradientColor = 175.0/3;
           }
           else
           {
               gradientColor = cpt*175.0/pointCount;
           }
           colors->InsertNextTuple3(gradientColor,38.0,238.0);
        }
        else if(colorType == 3)    //rainbow
        {
            colors->InsertNextTuple3(255-255*(pointZ-z_min_)/(z_max_-z_min_),0,255.0*(pointZ-z_min_)/(z_max_-z_min_));
        }
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();

    if(point_cloud_data_!=NULL)
    {
       point_cloud_data_->Delete();
       point_cloud_data_ = NULL;
    }
    point_cloud_data_ = polyData;

    ptsMapper->SetInputData(polyData);
    point_cloud_actor_->SetMapper(ptsMapper);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayPointCloud(float *pCloud, int sizeNum,int colorType)
{
    if(point_cloud_actor_ != NULL)
    {
       point_cloud_actor_->Delete();
       point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = vtkActor::New();
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    int pointCount = sizeNum;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    int gradientColor;

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        pointX = pCloud[cpt * 3 + 0];
        pointY = pCloud[cpt * 3 + 1];
        pointZ = pCloud[cpt * 3 + 2];
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        if(colorType == 0)  // blue
        {
           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
        }
        else if(colorType == 1)  //yellow
        {
           colors->InsertNextTuple3(255,255,0);
        }
        else if(colorType == 2)    //purple
        {
           if(cpt*175.0/pointCount <= 175.0/3)
           {
               gradientColor = 175.0/3;
           }
           else
           {
               gradientColor = cpt*175.0/pointCount;
           }
           colors->InsertNextTuple3(gradientColor,38.0,238.0);
        }
        else if(colorType == 3)    //rainbow
        {
            colors->InsertNextTuple3(255-cpt*255.0/pointCount,0,cpt*255.0/pointCount);
        }
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    ptsMapper->SetInputData(polyData);
    point_cloud_actor_->SetMapper(ptsMapper);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayPointCloudTriangulation(float *pCloud, int sizeNum)
{
    if(point_cloud_actor_ != NULL)
    {
       point_cloud_actor_->Delete();
       point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = vtkActor::New();
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkDelaunay2D> ptsDelaunay = vtkSmartPointer<vtkDelaunay2D>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    int pointCount = sizeNum;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        pointX = pCloud[cpt * 3 + 0];
        pointY = pCloud[cpt * 3 + 1];
        pointZ = pCloud[cpt * 3 + 2];
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
    }
    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    //ptsMapper->SetInputData(polyData);
    ptsDelaunay->SetInputData(polyData);
    ptsDelaunay->Update();
    ptsMapper->SetInputData(ptsDelaunay->GetOutput());
    point_cloud_actor_->SetMapper(ptsMapper);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayPointCloudTexture(float *pCloud, unsigned char *pTexture, int sizeNum)
{
    if(point_cloud_actor_ != NULL)
    {
           point_cloud_actor_->Delete();
           point_cloud_actor_ = NULL;
    }
    point_cloud_actor_ = vtkActor::New();
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    //vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
     vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    int pointCount = sizeNum;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        double texture = 0.0;

        pointX = pCloud[cpt * 3 + 0];
        pointY = pCloud[cpt * 3 + 1];
        pointZ = pCloud[cpt * 3 + 2];
        texture = pTexture[cpt];

        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        colors->InsertNextTuple3(texture,texture,texture);
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    ptsMapper->SetInputData(polyData);
    point_cloud_actor_->SetMapper(ptsMapper);
    renderer_->AddActor(point_cloud_actor_);
    return 0;
}

int UTechCanvas3d::doDisplayAxes(int xOffest, int yOffest, int zOffest, double xAngle, double yAngle, double zAngle, int type)
{
    if(transform_ != NULL)
    {
        transform_->Delete();
        transform_ = NULL;
    }
    transform_ = vtkTransform::New();

    if(axes_ != NULL)
    {
        axes_->Delete();
        axes_ = NULL;
    }
    axes_ = vtkAxesActor::New();

    transform_->Translate(xOffest, yOffest, zOffest);
    if(type == 0)
    {
        transform_->RotateX(xAngle);
        transform_->RotateY(yAngle);
        transform_->RotateZ(zAngle);
    }
    else if(type == 1)
    {
        transform_->RotateY(yAngle);
        transform_->RotateX(xAngle);
        transform_->RotateZ(zAngle);
    }
    else if(type == 2)
    {
        transform_->RotateZ(zAngle);
        transform_->RotateY(yAngle);
        transform_->RotateX(xAngle);
    }

    axes_->SetAxisLabels(0);
    axes_->SetUserTransform(transform_);
    renderer_->AddActor(axes_);
    return 0;
}

int UTechCanvas3d::doDisplayText(int positionX, int positionY, int fontSize,const char *text)
{
    // Setup the text and add it to the renderer
   // vtkTextActor* textActor =  vtkTextActor::New();
    vtkSmartPointer<vtkTextActor> textActor =  vtkSmartPointer<vtkTextActor>::New();
    textActor->SetInput(text);
    textActor->SetPosition(positionX,positionY);
    textActor->GetTextProperty()->SetFontSize(fontSize);
    textActor->GetTextProperty()->SetColor(1.0,0.0,0.0);
    renderer_->AddActor2D (textActor);
    return 0;
}

int UTechCanvas3d::addDisplaySphere(double centerX, double centerY, double centerZ, double radius)
{
    vtkActor* actor = vtkActor::New();
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(centerX,centerY,centerZ);
    sphereSource->SetRadius(radius);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());
    actor->SetMapper(mapper);
    actor_set_.push_back(actor);
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::addDisplayPlane(double center[], double normal[])
{
    vtkActor* actor = vtkActor::New();
    vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
//  planeSource->SetOrigin(10,10,10);
    planeSource->SetCenter(center[0],center[1],center[2]);
    planeSource->SetNormal(normal[0],normal[1],normal[2]);
//  planeSource->SetXResolution(100);
//  planeSource->SetYResolution(100);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(planeSource->GetOutputPort());
    actor->SetMapper(mapper);
    actor_set_.push_back(actor);
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::addDisplayLine(float point1[], float point2[])
{
    vtkActor* actor = vtkActor::New();
    vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(point1[0],point1[1],point1[2]);
    lineSource->SetPoint2(point2[0],point2[1],point2[2]);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(lineSource->GetOutputPort());
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1,0,0);
    actor_set_.push_back(actor);
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::addDipslayPoint(float point[])
{
    vtkActor* actor = vtkActor::New();
    vtkSmartPointer<vtkPointSource> pointSource = vtkSmartPointer<vtkPointSource>::New();
    pointSource->SetCenter(point[0],point[1],point[2]);
    pointSource->SetRadius(0);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(pointSource->GetOutputPort());
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1,0,0);
    actor_set_.push_back(actor);
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::addDisplayPointCloud(float *pCloud, int sizeNum, int colorType)
{
    vtkActor* actor = vtkActor::New();
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    int pointCount = sizeNum;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    int gradientColor;

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        pointX = pCloud[cpt * 3 + 0];
        pointY = pCloud[cpt * 3 + 1];
        pointZ = pCloud[cpt * 3 + 2];
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        if(colorType == 0)  // blue
        {
           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
        }
        else if(colorType == 1)  //yellow
        {
           colors->InsertNextTuple3(255,255,0);
        }
        else if(colorType == 2)    //purple
        {
           if(cpt*175.0/pointCount <= 175.0/3)
           {
               gradientColor = 175.0/3;
           }
           else
           {
               gradientColor = cpt*175.0/pointCount;
           }
           colors->InsertNextTuple3(gradientColor,38.0,238.0);
        }
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    ptsMapper->SetInputData(polyData);
    actor->SetMapper(ptsMapper);
    actor_set_.push_back(actor);
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::addDisplayPointCloud(std::vector<float> &pCloud, int colorType)
{
    vtkActor* actor = vtkActor::New();
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    int pointCount = pCloud.size()/3;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    int gradientColor;

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        pointX = pCloud.at(cpt*3);
        pointY = pCloud.at(cpt*3+1);
        pointZ = pCloud.at(cpt*3+2);
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        if(colorType == 0)  // blue
        {
           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
        }
        else if(colorType == 1)  //yellow
        {
           colors->InsertNextTuple3(255,255,0);
        }
        else if(colorType == 2)    //purple
        {
           if(cpt*175.0/pointCount <= 175.0/3)
           {
               gradientColor = 175.0/3;
           }
           else
           {
               gradientColor = cpt*175.0/pointCount;
           }
           colors->InsertNextTuple3(gradientColor,38.0,238.0);
        }
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    ptsMapper->SetInputData(polyData);
    actor->SetMapper(ptsMapper);
    actor_set_.push_back(actor);
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::addDisplayPointCloudTexture(std::vector<float> &pCloud)
{
    vtkActor* actor = vtkActor::New();
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    //vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
     vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    int pointCount = pCloud.size()/4;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        double textureR = 0.0;
        double textureG = 0.0;
        double textureB = 0.0;

        pointX = pCloud.at(cpt * 4 + 0);
        pointY = pCloud.at(cpt * 4 + 1);
        pointZ = pCloud.at(cpt * 4 + 2);
        textureR = pCloud.at(cpt * 4 + 3);
        textureG = pCloud.at(cpt * 4 + 3);
        textureB = pCloud.at(cpt * 4 + 3);

        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        colors->InsertNextTuple3(textureR,textureG,textureB);
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    ptsMapper->SetInputData(polyData);
    actor->SetMapper(ptsMapper);
    actor_set_.push_back(actor);
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::addDisplayAscFile(QString path,int colorType)
{
    if(path == "")
    {
        return -1;
    }
    actor_set_.push_back(pointsReader(path,colorType));

    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));

    return 0;
}

int UTechCanvas3d::addDisplayAscFileTriangulation(QString path, int colorType, int dataType)
{
    if(path == "")
    {
        return -1;
    }
    actor_set_.push_back(pointsReaderTriangulation(path,colorType,dataType));

    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));

    return 0;
}

int UTechCanvas3d::addDisplayPlyFile(QString path, int colorType)
{
    if(path == "")
    {
        return -1;
    }
    if(colorType == 0)   // blue
    {
        actor_set_.push_back(plyFileReader(path,1,0,0,255));
    }
    else if(colorType == 1)  //yellow
    {
        actor_set_.push_back(plyFileReader(path,1,255,255,0));
    }
    else if(colorType == 2)   //purple
    {
        actor_set_.push_back(plyFileReader(path,1,175,38,238));
    }
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));

    return 0;
}

int UTechCanvas3d::addDisplayStlFile(QString path)
{
    if(path == "")
    {
        return -1;
    }

    actor_set_.push_back(stlFileReader(path,1,175,38,238));
    renderer_->AddActor(actor_set_.at(actor_set_.length()-1));
    return 0;
}

int UTechCanvas3d::setPickStyle(int enabled, float *data, int length)
{
    if(render_window_interactor_ != NULL)
    {
        render_window_interactor_->Delete();
        render_window_interactor_ = NULL;
    }
    if(enabled == 0)
    {
        vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
        render_window_interactor_ = vtkRenderWindowInteractor::New();
        render_window_interactor_->SetPicker(pointPicker);
        render_window_interactor_->SetRenderWindow(render_window_);
        vtkSmartPointer<PointPickerInteractorStyle> style =
                vtkSmartPointer<PointPickerInteractorStyle>::New();
        style->SetView(this);
        style->setModelData(data,length);
        render_window_interactor_->SetInteractorStyle(style);
        render_window_->Render();
        render_window_interactor_->Start();
    }
    else if(enabled == 1)
    {
        vtkSmartPointer<vtkAreaPicker> areaPicker = vtkSmartPointer<vtkAreaPicker>::New();
        render_window_interactor_ = vtkRenderWindowInteractor::New();
        render_window_interactor_->SetPicker(areaPicker);
        render_window_interactor_->SetRenderWindow(render_window_);
        vtkSmartPointer<HighlightInteractorStyle> style =
                vtkSmartPointer<HighlightInteractorStyle>::New();
       style->SetPolyData(this,point_cloud_data_);
       render_window_interactor_->SetInteractorStyle(style);
       render_window_->Render();
       render_window_interactor_->Start();
    }
    return 0;
}

int UTechCanvas3d::getPickX(double &x)
{
  //  x = picked[0];
    return 0;
}

int UTechCanvas3d::getPickY(double &y)
{
  //  y = picked[1];
    return 0;
}

int UTechCanvas3d::getPickZ(double &z)
{
 //   z = picked[2];
    return 0;
}

int UTechCanvas3d::clearAll()
{
    renderer_->RemoveAllViewProps();
    this->update();
    return 0;
}

int UTechCanvas3d::affichage()
{
    renderer_->ResetCamera();
    render_window_->Render();
    return 0;
}

int UTechCanvas3d::removeAllActor()
{
    if(actor_set_.length()>0)
    {
        for(int i=0; i<actor_set_.length(); i++)
        {
            renderer_->RemoveActor(actor_set_.at(i));
            actor_set_.at(i)->Delete();
         //   actor_set_.at(i) = NULL;
        }
        actor_set_.erase(actor_set_.begin(),actor_set_.end());
        //actor_set_.clear();
    }
    return 0;
}


void UTechCanvas3d::init(int width, int height)
{
    this->setFixedSize(width,height);
    renderer_ =  vtkSmartPointer<vtkRenderer>::New();
    renderer_->SetBackground(40.0/255,40.0/255,40.0/255);
    render_window_ = vtkGenericOpenGLRenderWindow::New();
    this->SetRenderWindow(render_window_);
    render_window_->AddRenderer(renderer_);
    point_cloud_actor_ = NULL;
    stl_actor_ = NULL;
    transform_ = NULL;
    axes_      = NULL;
    select_points_ = NULL;
    select_points_length_  = 0;
    current_points_ =NULL;
    current_points_length_ = 0;
    point_cloud_data_ = NULL;
    render_window_interactor_ = NULL;
    is_display_line_ = false;
}

vtkActor* UTechCanvas3d::stlFileReader(QString path, int mode,int r, int g, int b)
{
    vtkSmartPointer<vtkSTLReader> stlReader = vtkSmartPointer<vtkSTLReader>::New();
    vtkSmartPointer<vtkPolyDataMapper> stlMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkPolyData> stlPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkActor* stlActor = vtkActor::New();
    stlReader->SetFileName(path.toLocal8Bit().data());
    stlReader->Update();
    stlPolyData = stlReader->GetOutput();
    stlMapper->SetInputConnection(stlReader->GetOutputPort());
    stlActor->SetMapper(stlMapper);
    stlActor->GetProperty()->SetOpacity(0.6);
    stlActor->GetProperty()->SetColor(r*1.0/255,g*1.0/255,b*1.0/255);
    if(mode == 0)
    {
        stlActor->GetProperty()->SetRepresentationToWireframe();
    }
    else if(mode == 1)
    {
        stlActor->GetProperty()->SetRepresentationToSurface();
    }
    return stlActor;
}

vtkActor* UTechCanvas3d::plyFileReader(QString path, int mode, int r, int g, int b)
{
    vtkSmartPointer<vtkPLYReader> plyReader = vtkSmartPointer<vtkPLYReader>::New();
    vtkSmartPointer<vtkPolyDataMapper> plyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkPolyData> plyPolyData  = vtkSmartPointer<vtkPolyData>::New();
    vtkActor* plyActor = vtkActor::New();
    plyReader->SetFileName(path.toLocal8Bit().data());
    plyReader->Update();
    plyPolyData = plyReader->GetOutput();
    plyMapper->SetInputConnection(plyReader->GetOutputPort());
    plyActor->SetMapper(plyMapper);
    plyActor->GetProperty()->SetOpacity(0.6);
    plyActor->GetProperty()->SetColor(r*1.0/255,g*1.0/255,b*1.0/255);
    if(mode == 0)
    {
        plyActor->GetProperty()->SetRepresentationToWireframe();
    }
    else if(mode == 1)
    {
        plyActor->GetProperty()->SetRepresentationToSurface();
    }
    else if(mode == 2)
    {
        plyActor->GetProperty()->SetRepresentationToPoints();
    }
    return plyActor;
}

//vtkActor* UTechCanvas3d::pointsReader(QString path,int colorType,int dataType)
//{
//    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
//    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
//    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
//    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
//    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    vtkActor* ptsActor = vtkActor::New();
//    QVector<UTechPoint*>* vesselPoints = doReadPointCloudFile(path,dataType);
//    int pointCount = vesselPoints->size();

//    colors->SetNumberOfComponents(3);
//    colors->SetName("Colors");
//    int gradientColor;
//    for(int cpt = 0;cpt<vesselPoints->size();cpt++)
//    {
//        vtkIdType pid[1];
//        double pointX = 0.0;
//        double pointY = 0.0;
//        double pointZ = 0.0;
//        vesselPoints->at(cpt)->getX(pointX);
//        vesselPoints->at(cpt)->getY(pointY);
//        vesselPoints->at(cpt)->getZ(pointZ);
//        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
//        vertices->InsertNextCell(1,pid);
//        if(colorType == 0)  // blue
//        {
//           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
//        }
//        else if(colorType == 1)  //yellow
//        {
//           colors->InsertNextTuple3(255,255,0);
//        }
//        else if(colorType == 2)    //purple
//        {
//           if(cpt*175.0/pointCount <= 175.0/3)
//           {
//               gradientColor = 175.0/3;
//           }
//           else
//           {
//               gradientColor = cpt*175.0/pointCount;
//           }
//           colors->InsertNextTuple3(gradientColor,38.0,238.0);
//        }
//    }

//    polyData->SetPoints(pts);
//    polyData->SetVerts(vertices);
//    polyData->GetPointData()->SetScalars(colors);
//    polyData->Modified();
//    point_cloud_data_ = polyData;
//    ptsMapper->SetInputData(polyData);
//    ptsActor->SetMapper(ptsMapper);

//    for(int i = 0;i<vesselPoints->size();i++)
//    {
//        UTechPoint* point = vesselPoints->at(i);
//        delete point;
//        point = NULL;
//    }
//    delete vesselPoints;
//    vesselPoints  = NULL;
//    return ptsActor;
//}


vtkActor* UTechCanvas3d::pointsReader(QString path,int colorType)
{
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkActor* ptsActor = vtkActor::New();
    QTextCodec *code = QTextCodec::codecForName("gb2312");
    std::string name = code->fromUnicode(path).data();
    AscFileReader(name.c_str(),current_points_,current_points_length_,z_min_,z_max_);
    qDebug() << "current_points_length_:" << current_points_length_ << endl;
    //readPointCloudFile(name.c_str());
    //readPointCloudFile(path.toStdString().c_str());
    //readPointCloudFile(path,dataType);
    int pointCount = current_points_length_;

    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    int gradientColor;
    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = current_points_[cpt*3];
        double pointY = current_points_[cpt*3+1];
        double pointZ = current_points_[cpt*3+2];
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        if(colorType == 0)  // blue
        {
           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
        }
        else if(colorType == 1)  //yellow
        {
           colors->InsertNextTuple3(255,255,0);
        }
        else if(colorType == 2)    //purple
        {
           if(cpt*175.0/pointCount <= 175.0/3)
           {
               gradientColor = 175.0/3;
           }
           else
           {
               gradientColor = cpt*175.0/pointCount;
           }
           colors->InsertNextTuple3(gradientColor,38.0,238.0);
        }
        else if(colorType == 3)    //rainbow
        {
           colors->InsertNextTuple3(255-255*(pointZ-z_min_)/(z_max_-z_min_),0,255.0*(pointZ-z_min_)/(z_max_-z_min_));
        }
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    if(point_cloud_data_!=NULL)
    {
       point_cloud_data_->Delete();
       point_cloud_data_ = NULL;
    }

    point_cloud_data_ = polyData;
    ptsMapper->SetInputData(polyData);
    ptsActor->SetMapper(ptsMapper);
    return ptsActor;
}

vtkActor *UTechCanvas3d::pointsReader(int colorType)
{
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkActor* ptsActor = vtkActor::New();

    int pointCount = current_points_length_;

    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    int gradientColor;
    for(int index=2; index<pointCount*3;index=index+3)
    {
        if(index == 2)
        {
           z_min_ =  z_max_ = current_points_[index];
        }
        if(z_min_> current_points_[index])
        {
            z_min_ = current_points_[index];
        }
        if(z_max_ < current_points_[index])
        {
            z_max_ = current_points_[index];
        }
    }

    for(int cpt = 0;cpt<pointCount;cpt++)
    {
        vtkIdType pid[1];
        double pointX = current_points_[cpt*3];
        double pointY = current_points_[cpt*3+1];
        double pointZ = current_points_[cpt*3+2];
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        if(colorType == 0)  // blue
        {
           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
        }
        else if(colorType == 1)  //yellow
        {
           colors->InsertNextTuple3(255,255,0);
        }
        else if(colorType == 2)    //purple
        {
           if(cpt*175.0/pointCount <= 175.0/3)
           {
               gradientColor = 175.0/3;
           }
           else
           {
               gradientColor = cpt*175.0/pointCount;
           }
           colors->InsertNextTuple3(gradientColor,38.0,238.0);
        }
        else if(colorType == 3)    //rainbow
        {
           colors->InsertNextTuple3(255-255*(pointZ-z_min_)/(z_max_-z_min_),0,255.0*(pointZ-z_min_)/(z_max_-z_min_));
        }
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    if(point_cloud_data_!=NULL)
    {
       point_cloud_data_->Delete();
       point_cloud_data_ = NULL;
    }

    point_cloud_data_ = polyData;
    ptsMapper->SetInputData(polyData);
    ptsActor->SetMapper(ptsMapper);
    return ptsActor;
}


vtkActor *UTechCanvas3d::pointsReaderTriangulation(QString path, int colorType, int dataType)
{
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> ptsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkDelaunay2D> ptsDelaunay = vtkSmartPointer<vtkDelaunay2D>::New();
    vtkActor* ptsActor = vtkActor::New();
    QVector<UTechPoint*>* vesselPoints = doReadPointCloudFile(path,dataType);

    int pointCount = vesselPoints->size();

    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    int gradientColor;
    for(int cpt = 0;cpt<vesselPoints->size();cpt++)
    {
        vtkIdType pid[1];
        double pointX = 0.0;
        double pointY = 0.0;
        double pointZ = 0.0;
        pointX =vesselPoints->at(cpt)->x;
        pointY =vesselPoints->at(cpt)->y;
        pointZ =vesselPoints->at(cpt)->z;
        pid[0] = pts->InsertNextPoint(pointX,pointY,pointZ);
        vertices->InsertNextCell(1,pid);
        if(colorType == 0)  // blue
        {
           colors->InsertNextTuple3(0,cpt*255.0/pointCount,255);
        }
        else if(colorType == 1)  //yellow
        {
           colors->InsertNextTuple3(255,255,0);
        }
        else if(colorType == 2)
        {
           if(cpt*175.0/pointCount <= 175.0/3)
           {
               gradientColor = 175.0/3;
           }
           else
           {
               gradientColor = cpt*175.0/pointCount;
           }
           colors->InsertNextTuple3(gradientColor,38.0,238.0);
        }
    }

    polyData->SetPoints(pts);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(colors);
    polyData->Modified();
    ptsDelaunay->SetInputData(polyData);
    ptsDelaunay->Update();
    ptsMapper->SetInputData(ptsDelaunay->GetOutput());
 //   ptsMapper->SetInputData(polyData);
    ptsActor->SetMapper(ptsMapper);

    for(int i = 0;i<vesselPoints->size();i++)
    {
        UTechPoint* point = vesselPoints->at(i);
        delete point;
        point = NULL;
    }
    delete vesselPoints;
    vesselPoints  = NULL;
    return ptsActor;
}

QVector<UTechPoint*>* UTechCanvas3d::doReadPointCloudFile(QString path,int dataType)
{
    QVector<UTechPoint*>* pointCloudVessel = new QVector<UTechPoint*>();
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug() << "sorry,can't open the file!" << endl;
    }
    //int cpt = 0;
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();

        //if(cpt > 10)
        //{
            QString str(line);

            QString ptsString = str.split("\n")[0];
            QStringList parameter;
            if(dataType == 0)  // big data
            {
               parameter = ptsString.split("\t");
            }
            else if(dataType == 1)  // small data
            {
               parameter = ptsString.split(" ");
            }

            UTechPoint* point = new UTechPoint();
            point->x = parameter[0].toDouble();
            point->y = parameter[1].toDouble();
            point->z = parameter[2].toDouble();

            pointCloudVessel->append(point);
        //}
        //cpt ++;
    }
    return pointCloudVessel;
}

//void UTechCanvas3d::readPointCloudFile(QString path, int dataType)
//{
//    std::vector<double> points;
//    QFile file(path);
//    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
//    {
//        qDebug() << "sorry,can't open the file!" << endl;
//    }
//    current_points_length_ = 0;
//    while(!file.atEnd())
//    {
//        QByteArray line = file.readLine();
//        QString str(line);
//        QString ptsString = str.split("\n")[0];
//        QStringList parameter;
//        if(dataType == 0)  // big data
//        {
//           parameter = ptsString.split("\t");
//        }
//        else if(dataType == 1)  // small data
//        {
//           parameter = ptsString.split(" ");
//        }
//        points.push_back(parameter[0].toDouble(0));
//        points.push_back(parameter[1].toDouble(0));
//        points.push_back(parameter[2].toDouble(0));
//        current_points_length_ ++;
//    }
//    if(current_points_ != NULL)
//    {
//        delete[] current_points_;
//        current_points_ = NULL;
//    }
//    current_points_ = new float[current_points_length_*3];
//    for(int i=0;i<current_points_length_;i++)
//    {
//       current_points_[i*3]   = points.at(i*3);
//       current_points_[i*3+1] = points.at(i*3+1);
//       current_points_[i*3+2] = points.at(i*3+2);
//    }
//}


void UTechCanvas3d::readPointCloudFile(QString path, int dataType)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
    }
    current_points_length_ = 1008420;
    if(current_points_ != NULL)
    {
        delete[] current_points_;
        current_points_ = NULL;
    }
    current_points_ = new float[current_points_length_*3];
    int i =0;

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        QString ptsString = str.split("\n")[0];
        QStringList parameter;
        if(dataType == 0)  // big data
        {
           parameter = ptsString.split("\t");
        }
        else if(dataType == 1)  // small data
        {
           parameter = ptsString.split(" ");
        }
        current_points_[i*3]   = parameter[0].toDouble(0);
        current_points_[i*3+1] = parameter[1].toDouble(0);
        current_points_[i*3+2] = parameter[2].toDouble(0);
        i++;
    }
    current_points_length_ = i;
}

void UTechCanvas3d::readPointCloudFile(const char *path)
{
    std::ifstream inFile;
    inFile.open(path);
    if(!inFile.is_open())
    {

    }
    else
    {
        current_points_length_ = 1008420;
        if(current_points_ != NULL)
        {
            delete[] current_points_;
            current_points_ = NULL;
        }
        current_points_ = new float[current_points_length_*3];
        float value =0;
        int i = 0;
        int leng = 3;
        while(inFile.good() && !inFile.eof())
        {
            inFile >> value;
            current_points_[i] = value;
            if(i == 2)
            {
                z_min_ = z_max_ = value;
            }
            if(i%leng == 2)
            {
                if(z_min_ > value)
                {
                    z_min_ = value;
                }
                if(z_max_ < value)
                {
                    z_max_ = value;
                }
            }
            i++;
        }
        qDebug() <<"zMin:" << z_min_ << endl;
        qDebug() <<"zMax:" << z_max_ << endl;
        current_points_length_ = (i+1)/3;
    }
    inFile.close();
}
