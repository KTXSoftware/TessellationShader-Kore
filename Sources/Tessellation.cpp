#include "pch.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Shader.h>
#include <Kore/Math/Matrix.h>
#include <Kore/System.h>
#include <limits>

using namespace Kore;

namespace {
	Shader* vertexShader;
	Shader* fragmentShader;
	Shader* geometryShader;
	Shader* tessEvalShader;
	Shader* tessControlShader;
	Program* program;
	VertexBuffer* vertices;
	IndexBuffer* indices;

	ConstantLocation tessLevelInnerLocation;
	ConstantLocation tessLevelOuterLocation;
	ConstantLocation lightPositionLocation;
	ConstantLocation projectionLocation;
	ConstantLocation modelviewLocation;
	ConstantLocation normalMatrixLocation;
	ConstantLocation ambientMaterialLocation;
	ConstantLocation diffuseMaterialLocation;

	float tessLevelInner = 3;
	float tessLevelOuter = 2;
	float width = 1024;
	float height = 768;

	void update() {
		Graphics::begin();
		Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag);
		Graphics::setRenderState(Kore::DepthTest, true);

		program->set();
		Graphics::setVertexBuffer(*vertices);
		Graphics::setIndexBuffer(*indices);
		
		Graphics::setFloat(tessLevelInnerLocation, tessLevelInner);
		Graphics::setFloat(tessLevelOuterLocation, tessLevelOuter);
		Graphics::setFloat3(lightPositionLocation, 0.25f, 0.25f, 1.0f);
        Graphics::setMatrix(projectionLocation, mat4::Perspective(Kore::pi / 3, width / height, 5, 150));
		
		mat4 rotation = mat4::RotationX((float)System::time());
		vec3 eyePosition(0.0f, 0.0f, -8.0f);
		vec3 targetPosition(0.0f, 0.0f, 0.0f);
		vec3 upVector(0.0f, 1.0f, 0.0f);
		mat4 lookAt = mat4::lookAt(eyePosition, targetPosition, upVector);
		mat4 modelviewMatrix = lookAt * rotation;
		mat3 normalMatrix(modelviewMatrix.Transpose3x3());
		Graphics::setMatrix(modelviewLocation, modelviewMatrix);
		Graphics::setMatrix(normalMatrixLocation, normalMatrix);
		
		Graphics::setFloat3(ambientMaterialLocation, 0.04f, 0.04f, 0.04f);
		Graphics::setFloat3(diffuseMaterialLocation, 0.0f, 0.75f, 0.75f);

		Graphics::drawIndexedVertices();

		Graphics::end();
		Graphics::swapBuffers();
	}
}

int kore(int argc, char** argv) {
	Kore::System::setName("Tessellation");
	Kore::System::setup();
	Kore::WindowOptions options;
	options.title = "Tessellation";
	options.width = 1024;
	options.height = 768;
	options.x = 100;
	options.y = 100;
	options.targetDisplay = -1;
	options.mode = WindowModeWindow;
	options.rendererOptions.depthBufferBits = 16;
	options.rendererOptions.stencilBufferBits = 8;
	options.rendererOptions.textureFormat = 0;
	options.rendererOptions.antialiasing = 0;
	Kore::System::initWindow(options);
	Kore::System::setCallback(update);

	FileReader vs("test.vert");
	FileReader fs("test.frag");
	FileReader gs("test.geom");
	FileReader tese("test.tese");
	FileReader tesc("test.tesc");
	vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
	fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
	geometryShader = new Shader(gs.readAll(), gs.size(), GeometryShader);
	tessEvalShader = new Shader(tese.readAll(), tese.size(), TesselationEvaluationShader);
	tessControlShader = new Shader(tesc.readAll(), tesc.size(), TesselationControlShader);
	VertexStructure structure;
	structure.add("Position", Float3VertexData);
	program = new Program;
	program->setVertexShader(vertexShader);
	program->setFragmentShader(fragmentShader);
	program->setGeometryShader(geometryShader);
	program->setTesselationEvaluationShader(tessEvalShader);
	program->setTesselationControlShader(tessControlShader);
	program->link(structure);

	tessLevelInnerLocation = program->getConstantLocation("TessLevelInner");
	tessLevelOuterLocation = program->getConstantLocation("TessLevelOuter");
	lightPositionLocation = program->getConstantLocation("LightPosition");
	projectionLocation = program->getConstantLocation("Projection");
	modelviewLocation = program->getConstantLocation("Modelview");
	normalMatrixLocation = program->getConstantLocation("NormalMatrix");
	ambientMaterialLocation = program->getConstantLocation("AmbientMaterial");
	diffuseMaterialLocation = program->getConstantLocation("DiffuseMaterial");

	{
		vertices = new VertexBuffer(12, structure);
		float* data = vertices->lock();
		int i = 0;

		data[i++] = 0.000f; data[i++] = 0.000f; data[i++] = 1.000f;
		data[i++] = 0.894f; data[i++] = 0.000f; data[i++] = 0.447f;
		data[i++] = 0.276f; data[i++] = 0.851f; data[i++] = 0.447f;
		data[i++] = -0.724f; data[i++] = 0.526f; data[i++] = 0.447f;
		data[i++] = -0.724f; data[i++] = -0.526f; data[i++] = 0.447f;
		data[i++] = 0.276f; data[i++] = -0.851f; data[i++] = 0.447f;
		data[i++] = 0.724f; data[i++] = 0.526f; data[i++] = -0.447f;
		data[i++] = -0.276f; data[i++] = 0.851f; data[i++] = -0.447f;
		data[i++] = -0.894f; data[i++] = 0.000f; data[i++] = -0.447f;
		data[i++] = -0.276f; data[i++] = -0.851f; data[i++] = -0.447f;
		data[i++] = 0.724f; data[i++] = -0.526f; data[i++] = -0.447f;
		data[i++] = 0.000f; data[i++] = 0.000f; data[i++] = -1.000f;

		vertices->unlock();
	}

	{
		indices = new IndexBuffer(20 * 3);
		int i = 0;
		int* data = indices->lock();

		data[i++] = 2; data[i++] = 1; data[i++] = 0;
		data[i++] = 3; data[i++] = 2; data[i++] = 0;
		data[i++] = 4; data[i++] = 3; data[i++] = 0;
		data[i++] = 5; data[i++] = 4; data[i++] = 0;
		data[i++] = 1; data[i++] = 5; data[i++] = 0;
		data[i++] = 11; data[i++] = 6; data[i++] = 7;
		data[i++] = 11; data[i++] = 7; data[i++] = 8;
		data[i++] = 11; data[i++] = 8; data[i++] = 9;
		data[i++] = 11; data[i++] = 9; data[i++] = 10;
		data[i++] = 11; data[i++] = 10; data[i++] = 6;
		data[i++] = 1; data[i++] = 2; data[i++] = 6;
		data[i++] = 2; data[i++] = 3; data[i++] = 7;
		data[i++] = 3; data[i++] = 4; data[i++] = 8;
		data[i++] = 4; data[i++] = 5; data[i++] = 9;
		data[i++] = 5; data[i++] = 1; data[i++] = 10;
		data[i++] = 2; data[i++] = 7; data[i++] = 6;
		data[i++] = 3; data[i++] = 8; data[i++] = 7;
		data[i++] = 4; data[i++] = 9; data[i++] = 8;
		data[i++] = 5; data[i++] = 10; data[i++] = 9;
		data[i++] = 1; data[i++] = 6; data[i++] = 10;

		indices->unlock();
	}

	Kore::System::start();

	return 0;
}
