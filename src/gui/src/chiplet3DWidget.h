// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025, The OpenROAD Authors

#pragma once

#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <vector>

namespace odb {
class dbChip;
}

namespace utl {
class Logger;
}

namespace gui {

class Chiplet3DWidget : public QVTKOpenGLNativeWidget
{
  Q_OBJECT

 public:
  explicit Chiplet3DWidget(QWidget* parent = nullptr);
  ~Chiplet3DWidget() override = default;

  void setChip(odb::dbChip* chip);
  void setLogger(utl::Logger* logger);

 private:
  void buildGeometries();
  void setupRenderer();
  void addGridActor();
  vtkSmartPointer<vtkActor> createChipletActor(double xMin,
                                               double yMin,
                                               double zMin,
                                               double xMax,
                                               double yMax,
                                               double zMax,
                                               double r,
                                               double g,
                                               double b);

  odb::dbChip* chip_ = nullptr;
  utl::Logger* logger_ = nullptr;

  vtkSmartPointer<vtkRenderer> renderer_;
  std::vector<vtkSmartPointer<vtkActor>> chiplet_actors_;
  vtkSmartPointer<vtkActor> grid_actor_;

  double bounding_radius_ = 10.0;
};

}  // namespace gui
