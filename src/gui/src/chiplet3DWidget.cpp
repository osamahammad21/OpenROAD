// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025, The OpenROAD Authors

#include "chiplet3DWidget.h"

#include <vtkCellArray.h>
#include <vtkCubeSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLine.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnsignedCharArray.h>

#include <array>
#include <cmath>

#include "odb/db.h"
#include "odb/dbTransform.h"
#include "odb/geom.h"
#include "odb/unfoldedModel.h"
#include "utl/Logger.h"

namespace {
constexpr double INITIAL_DISTANCE_FACTOR = 3.0;
constexpr double LAYER_GAP_FACTOR = 2.0;
constexpr double MIN_DISTANCE_CHECK = 100.0;
constexpr double DEFAULT_DISTANCE = 1000.0;
constexpr double GRID_SIZE_FACTOR = 1.5;
constexpr int GRID_LINE_COUNT = 5;
constexpr double GRID_STEPS = 5.0;
constexpr double GRID_Z_OFFSET_FACTOR = 0.05;

static const std::array<std::array<double, 3>, 7> COLOR_PALETTE
    = {{{0.0, 1.0, 0.0},
        {1.0, 1.0, 0.0},
        {0.0, 1.0, 1.0},
        {1.0, 0.0, 1.0},
        {1.0, 0.5, 0.0},
        {0.5, 0.5, 1.0},
        {1.0, 0.0, 0.0}}};
}  // namespace

namespace gui {

Chiplet3DWidget::Chiplet3DWidget(QWidget* parent)
    : QVTKOpenGLNativeWidget(parent)
{
  setupRenderer();
}

void Chiplet3DWidget::setupRenderer()
{
  // Create render window and renderer
  vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
  renderer_ = vtkSmartPointer<vtkRenderer>::New();

  renderWindow->AddRenderer(renderer_);
  setRenderWindow(renderWindow);

  // Set background color (dark gray)
  renderer_->SetBackground(0.1, 0.1, 0.1);

  // Setup interactor style for trackball camera rotation
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  renderWindow->GetInteractor()->SetInteractorStyle(style);
}

void Chiplet3DWidget::setChip(odb::dbChip* chip)
{
  chip_ = chip;
  buildGeometries();
  renderWindow()->Render();
}

void Chiplet3DWidget::setLogger(utl::Logger* logger)
{
  logger_ = logger;
}

vtkSmartPointer<vtkActor> Chiplet3DWidget::createChipletActor(double xMin,
                                                              double yMin,
                                                              double zMin,
                                                              double xMax,
                                                              double yMax,
                                                              double zMax,
                                                              double r,
                                                              double g,
                                                              double b)
{
  // Calculate center and dimensions
  double centerX = (xMin + xMax) / 2.0;
  double centerY = (yMin + yMax) / 2.0;
  double centerZ = (zMin + zMax) / 2.0;
  double lengthX = xMax - xMin;
  double lengthY = yMax - yMin;
  double lengthZ = zMax - zMin;

  // Create cube source for solid cuboid
  vtkNew<vtkCubeSource> cubeSource;
  cubeSource->SetCenter(centerX, centerY, centerZ);
  cubeSource->SetXLength(lengthX);
  cubeSource->SetYLength(lengthY);
  cubeSource->SetZLength(lengthZ);
  cubeSource->Update();

  // Create mapper
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(cubeSource->GetOutputPort());

  // Create actor
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(r, g, b);
  actor->GetProperty()->SetOpacity(0.9);  // Slight transparency to see overlaps
  actor->GetProperty()->SetEdgeVisibility(
      true);  // Show edges for better definition
  actor->GetProperty()->SetEdgeColor(0.2, 0.2, 0.2);  // Dark edges

  return actor;
}

void Chiplet3DWidget::addGridActor()
{
  double grid_size = bounding_radius_ * GRID_SIZE_FACTOR;
  double step = grid_size / GRID_STEPS;
  double grid_z = 0;

  vtkNew<vtkPoints> points;
  vtkNew<vtkCellArray> lines;

  int pointId = 0;
  for (int i = -GRID_LINE_COUNT; i <= GRID_LINE_COUNT; ++i) {
    double pos = i * step;

    // Vertical line
    points->InsertNextPoint(pos, -grid_size, grid_z);
    points->InsertNextPoint(pos, grid_size, grid_z);
    vtkNew<vtkLine> vLine;
    vLine->GetPointIds()->SetId(0, pointId++);
    vLine->GetPointIds()->SetId(1, pointId++);
    lines->InsertNextCell(vLine);

    // Horizontal line
    points->InsertNextPoint(-grid_size, pos, grid_z);
    points->InsertNextPoint(grid_size, pos, grid_z);
    vtkNew<vtkLine> hLine;
    hLine->GetPointIds()->SetId(0, pointId++);
    hLine->GetPointIds()->SetId(1, pointId++);
    lines->InsertNextCell(hLine);
  }

  vtkNew<vtkPolyData> polyData;
  polyData->SetPoints(points);
  polyData->SetLines(lines);

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(polyData);

  grid_actor_ = vtkSmartPointer<vtkActor>::New();
  grid_actor_->SetMapper(mapper);
  grid_actor_->GetProperty()->SetColor(0.3, 0.3, 0.3);
  grid_actor_->GetProperty()->SetLineWidth(1.0);

  renderer_->AddActor(grid_actor_);
}

void Chiplet3DWidget::buildGeometries()
{
  if (!chip_) {
    return;
  }

  // Clear existing actors
  for (auto& actor : chiplet_actors_) {
    renderer_->RemoveActor(actor);
  }
  chiplet_actors_.clear();

  if (grid_actor_) {
    renderer_->RemoveActor(grid_actor_);
    grid_actor_ = nullptr;
  }

  odb::Cuboid global_cuboid = chip_->getCuboid();
  odb::UnfoldedModel model(logger_, chip_);
  odb::dbTransform center_transform = odb::dbTransform(
      odb::Point3D(-global_cuboid.xCenter(), -global_cuboid.yCenter(), 0));

  // Calculate bounding sphere radius
  double dx = global_cuboid.dx();
  double dy = global_cuboid.dy();
  double dz = global_cuboid.dz() * LAYER_GAP_FACTOR;
  bounding_radius_ = std::sqrt(dx * dx + dy * dy + dz * dz) / 2.0;

  double distance = bounding_radius_ * INITIAL_DISTANCE_FACTOR;
  if (distance < MIN_DISTANCE_CHECK) {
    distance = DEFAULT_DISTANCE;
  }

  // Add grid
  addGridActor();

  // Build chiplet actors
  int index = 0;
  for (const auto& chip : model.getChips()) {
    odb::Cuboid draw_cuboid = chip.cuboid;
    center_transform.apply(draw_cuboid);

    const auto& color = COLOR_PALETTE[index++ % COLOR_PALETTE.size()];

    auto actor = createChipletActor(draw_cuboid.xMin(),
                                    draw_cuboid.yMin(),
                                    draw_cuboid.zMin(),
                                    draw_cuboid.xMax(),
                                    draw_cuboid.yMax(),
                                    draw_cuboid.zMax(),
                                    color[0],
                                    color[1],
                                    color[2]);

    chiplet_actors_.push_back(actor);
    renderer_->AddActor(actor);
  }

  // Setup camera
  vtkCamera* camera = renderer_->GetActiveCamera();
  camera->SetPosition(0, 0, distance);
  camera->SetFocalPoint(0, 0, 0);
  camera->SetViewUp(0, 1, 0);

  // Set clipping range based on bounding radius
  double zNear = std::max(10.0, distance - bounding_radius_ * 2.0);
  double zFar = distance + bounding_radius_ * 2.0;
  camera->SetClippingRange(zNear, zFar);

  renderer_->ResetCameraClippingRange();
}

}  // namespace gui
