#include "pch.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Math/Matrix.h>
#include <Kore/System.h>
#include <limits>

using namespace Kore;

namespace {
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::Shader* geometryShader;
	Graphics4::Shader* tessEvalShader;
	Graphics4::Shader* tessControlShader;
	Graphics4::PipelineState* pipeline;
	Graphics4::VertexBuffer* vertices;
	Graphics4::IndexBuffer* indices;

	Graphics4::ConstantLocation tessLevelInnerLocation;
	Graphics4::ConstantLocation tessLevelOuterLocation;
	Graphics4::ConstantLocation lightPositionLocation;
	Graphics4::ConstantLocation projectionLocation;
	Graphics4::ConstantLocation modelviewLocation;
	Graphics4::ConstantLocation normalMatrixLocation;
	Graphics4::ConstantLocation ambientMaterialLocation;
	Graphics4::ConstantLocation diffuseMaterialLocation;

	float tessLevelInner = 3;
	float tessLevelOuter = 2;
	float width = 1024;
	float height = 768;

	void update() {
		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag);

		Graphics4::setPipeline(pipeline);
		Graphics4::setVertexBuffer(*vertices);
		Graphics4::setIndexBuffer(*indices);
		
		Graphics4::setFloat(tessLevelInnerLocation, tessLevelInner);
		Graphics4::setFloat(tessLevelOuterLocation, tessLevelOuter);
		Graphics4::setFloat3(lightPositionLocation, 0.25f, 0.25f, 1.0f);
        Graphics4::setMatrix(projectionLocation, mat4::Perspective(Kore::pi / 3, width / height, 5, 150));
		
		mat4 rotation = mat4::RotationX((float)System::time());
		vec3 eyePosition(0.0f, 0.0f, -8.0f);
		vec3 targetPosition(0.0f, 0.0f, 0.0f);
		vec3 upVector(0.0f, 1.0f, 0.0f);
		mat4 lookAt = mat4::lookAt(eyePosition, targetPosition, upVector);
		mat4 modelviewMatrix = lookAt * rotation;
		mat3 normalMatrix(modelviewMatrix.Transpose3x3());
		Graphics4::setMatrix(modelviewLocation, modelviewMatrix);
		Graphics4::setMatrix(normalMatrixLocation, normalMatrix);
		
		Graphics4::setFloat3(ambientMaterialLocation, 0.04f, 0.04f, 0.04f);
		Graphics4::setFloat3(diffuseMaterialLocation, 0.0f, 0.75f, 0.75f);

		Graphics4::drawIndexedVertices();

		Graphics4::end();
		Graphics4::swapBuffers();
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
	vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	geometryShader = new Graphics4::Shader(gs.readAll(), gs.size(), Graphics4::GeometryShader);
	tessEvalShader = new Graphics4::Shader(tese.readAll(), tese.size(), Graphics4::TessellationEvaluationShader);
	tessControlShader = new Graphics4::Shader(tesc.readAll(), tesc.size(), Graphics4::TessellationControlShader);
	Graphics4::VertexStructure structure;
	structure.add("Position", Graphics4::Float3VertexData);
	pipeline = new Graphics4::PipelineState();
	pipeline->vertexShader = vertexShader;
	pipeline->fragmentShader = fragmentShader;
	pipeline->geometryShader = geometryShader;
	pipeline->tessellationEvaluationShader = tessEvalShader;
	pipeline->tessellationControlShader = tessControlShader;
    pipeline->inputLayout[0] = &structure;
    pipeline->inputLayout[1] = nullptr;
	pipeline->compile();

	tessLevelInnerLocation = pipeline->getConstantLocation("TessLevelInner");
	tessLevelOuterLocation = pipeline->getConstantLocation("TessLevelOuter");
	lightPositionLocation = pipeline->getConstantLocation("LightPosition");
	projectionLocation = pipeline->getConstantLocation("Projection");
	modelviewLocation = pipeline->getConstantLocation("Modelview");
	normalMatrixLocation = pipeline->getConstantLocation("NormalMatrix");
	ambientMaterialLocation = pipeline->getConstantLocation("AmbientMaterial");
	diffuseMaterialLocation = pipeline->getConstantLocation("DiffuseMaterial");

	{
		vertices = new Graphics4::VertexBuffer(12, structure);
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
		indices = new Graphics4::IndexBuffer(20 * 3);
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
