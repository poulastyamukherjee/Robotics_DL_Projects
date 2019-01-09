#include <iostream>
#include <QWidget>
#include <Inventor/SoDB.h>
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <rl/sg/so/Scene.h>

int
main(int argc, char** argv)
{
	SoDB::init();
	QWidget* widget = SoQt::init(argc, argv, argv[0]);
	widget->resize(800, 600);
	rl::sg::so::Scene scene;
	scene.load("C:\\RoboWrapSVN4_build\\VC14_32\\dependencies\\rl-0.7.0\\share\\rl-0.7.0\\examples\\rlsg\\mitsubishi_rv_2f_boxes.xml");
	SoQtExaminerViewer viewer(widget, NULL, true, SoQtFullViewer::BUILD_POPUP);
	viewer.setSceneGraph(scene.root);
	viewer.setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
	viewer.show();
	SoQt::show(widget);
	SoQt::mainLoop();
	return 0;
}