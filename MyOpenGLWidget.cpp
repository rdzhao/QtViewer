#include "MyOpenGLWidget.h"

void OGLWidget::initializeGL()
{
	// general initialize
	leftPressed = false;
	faceSelection = false;
	edgeSelection = false;
	vertSelection = false;

	// opengl initialize
	initializeOpenGLFunctions();

	//connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

	printContextInformation();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void OGLWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < rModules.size(); ++i)
	{
		rModules[i]->setCamera(camera);
		rModules[i]->setLightDistance(5 * mesh->radius());
		rModules[i]->render();
	}
}

void OGLWidget::resizeGL(int width, int height)
{
	camera.setWinWidth(width);
	camera.setWinHeight(height);
	//cout << width << " " << height << endl;
}

void OGLWidget::mousePressEvent(QMouseEvent* event)
{
	cout << faceSelection << endl;
	if (faceSelection && event->button() == Qt::LeftButton)
	{
		cout << "At Face Selection..."<< endl;

		selectFace(event->x(), event->y());
	}
	else if (event->button() == Qt::LeftButton)
	{
		//cout << "@@@@@@@@@@@@@@@@@@@@@@@"<< endl;
		leftPressed = true;

		camera.setPWX(event->x());
		camera.setPWY(event->y());
		camera.updatePUnitCoord();
		camera.setPRotationIdentity();
	}
}
void OGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		leftPressed = false;
	}
}
void OGLWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (leftPressed)
	{
		//camera.moveUnitCoordToPre();

		camera.setWX(event->x());
		camera.setWY(event->y());
		camera.updateUnitCoord();

		camera.arcballRotate();
		update();
	}
}
void OGLWidget::wheelEvent(QWheelEvent* event)
{
	if (event->delta() != 0)
	{
		camera.setscroll(event->delta());
		camera.zoom();
		update();
	}
}

void OGLWidget::setMesh(Mesh* m)
{
	mesh = m;
}

void OGLWidget::setKDTree()
{
	int num_verts, num_tris;
	glm::vec3 *verts, *tris;
	int k;
	// import mesh data
	num_verts = mesh->size_of_vertices();
	verts = new glm::vec3[num_verts];
	for (Vertex_iterator vi = mesh->vertices_begin(); vi != mesh->vertices_end(); ++vi)
		verts[vi->idx()] = 
		glm::vec3(vi->point().x() - mesh->xcenter(),
			vi->point().y() - mesh->ycenter(),
			vi->point().z() - mesh->zcenter());
	num_tris = mesh->size_of_facets();
	tris = new glm::vec3[num_tris];
	for (Facet_iterator fi = mesh->facets_begin(); fi != mesh->facets_end(); ++fi)
		tris[fi->idx()] =
		glm::vec3(fi->halfedge()->vertex()->idx(),
			fi->halfedge()->next()->vertex()->idx(),
			fi->halfedge()->next()->next()->vertex()->idx());

	kdTree = new KDTreeCPU(num_tris, tris, num_verts, verts);
	
	cout << "Num Verts: " << num_verts << endl;
	cout << "Num Faces: " << num_tris << endl;
	cout << "KD tree construction complete ..." << endl;
}

void OGLWidget::setRenderContexts()
{
	meshModule = new MeshModule();
	rModules.push_back(meshModule);
	wireFrameModule = new WireFrameModule();
	rModules.push_back(wireFrameModule);

	setMeshModule(meshModule);
	setWireFrameModule(wireFrameModule);
}

void OGLWidget::setMeshModule(RenderModule* rm)
{
	std::vector<float> v;
	std::vector<int> idx;
	std::vector<float> n;
	std::vector<float> c;

	for (Vertex_iterator vi = mesh->vertices_begin(); vi != mesh->vertices_end(); ++vi)
	{
		//cout << vi->point() << endl;
		v.push_back(vi->point().x() - mesh->xcenter());
		v.push_back(vi->point().y() - mesh->ycenter());
		v.push_back(vi->point().z() - mesh->zcenter());

		n.push_back(vi->normal().x());
		n.push_back(vi->normal().y());
		n.push_back(vi->normal().z());

		c.push_back(0.3);
		c.push_back(0.5);
		c.push_back(0.7);
	}

	for (Facet_iterator fi = mesh->facets_begin(); fi != mesh->facets_end(); ++fi)
	{
		Halfedge_around_facet_circulator he = fi->facet_begin();
		Halfedge_around_facet_circulator end = he;

		//cout << "^^^^^^^^^^^^^^^^^^^^"<< endl;
		CGAL_For_all(he, end)
		{
			//cout << he->vertex()->idx() << endl;
			idx.push_back(he->vertex()->idx());
		}
	}

	rm->setData(v, idx, n, c);
}
void OGLWidget::setWireFrameModule(RenderModule* rm)
{
	std::vector<float> v;
	std::vector<int> idx;
	std::vector<float> n;
	std::vector<float> c;

	for (Vertex_iterator vi = mesh->vertices_begin(); vi != mesh->vertices_end(); ++vi)
	{
		//cout << vi->point() << endl;
		v.push_back(vi->point().x() - mesh->xcenter());
		v.push_back(vi->point().y() - mesh->ycenter());
		v.push_back(vi->point().z() - mesh->zcenter());

		n.push_back(vi->normal().x());
		n.push_back(vi->normal().y());
		n.push_back(vi->normal().z());

		c.push_back(1);
		c.push_back(0);
		c.push_back(0);
	}

	for (Edge_iterator ei = mesh->edges_begin(); ei != mesh->edges_end(); ++ei)
	{
		idx.push_back(ei->vertex()->idx());
		idx.push_back(ei->opposite()->vertex()->idx());
	}

	rm->setData(v, idx, n, c);
}

void OGLWidget::setCamera()
{

	camera.setView(QVector3D(0, 0, 5*mesh->radius()), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

	camera.setProject(45.0, 4.0 / 3.0, 0.01, 10000);
	
	camera.init();
}

void OGLWidget::printContextInformation()
{
	QString glType;
	QString glVersion;
	QString glProfile;

	// Get Version Information
	glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
	glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

	// Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
	switch (format().profile())
	{
		CASE(NoProfile);
		CASE(CoreProfile);
		CASE(CompatibilityProfile);
	}
#undef CASE

	// qPrintable() will print our QString w/o quotes around it.
	qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}

void OGLWidget::setFaceSelection(bool b)
{
	faceSelection = b;
}

void OGLWidget::setEdgeSelection(bool b)
{
	edgeSelection = b;
}

void OGLWidget::setVertSelection(bool b)
{
	edgeSelection = b;
}

void OGLWidget::selectFace(int wx, int wy)
{
	QVector3D nearP, farP, d;
	camera.getFarNearPointWorld(wx, wy, nearP, farP);
	d = (farP - nearP).normalized();

	//cout << nearP.x()<<" " << nearP.y() << " " << nearP.z() << endl;
	//cout << farP.x() << " " << farP.y() << " " << farP.z() << endl;

	glm::vec3 rayO, rayD, hitP, normal;
	float t;
	int idx;
	rayO = glm::vec3(nearP.x(), nearP.y(), nearP.z());
	rayD = glm::vec3(d.x(), d.y(), d.z());
	bool intersected = kdTree->intersectNew(rayO, rayD, t, hitP, normal, idx);

	//cout << "Intersected: " << intersected << endl;
	//if (intersected)
	//{
	//	cout << "Triangle Idx: " << idx << endl;
	//	cout << "Hit Point: " << hitP.x << " " << hitP.y << " " << hitP.z << endl;
	//	cout << "Normal: " << normal.x << " " << normal.y << " " << normal.z << endl;
	//}
	//cout << endl;

	if (intersected)
	{
		meshModule->highlightFace(idx);
		update();
	}
}