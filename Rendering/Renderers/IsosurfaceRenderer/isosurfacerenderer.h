#ifndef ISOSURFACERENDERER_H
#define ISOSURFACERENDERER_H
#include "Rendering/Renderer.h"
#include <GL/glew.h>
#include "Rendering/Renderers/IsosurfaceRenderer/isosurfacerendererconfig.h"

class IsosurfaceRenderer:public Renderer
{
	public:
        IsosurfaceRenderer();
        virtual ~IsosurfaceRenderer();
		virtual void glewIsReadyRenderer();
		void draw(RModel*);
        BaseRendererConfig* getRendererConfigWidget();
		bool hasRendererConfigWidget();
		void applyConfigChanges(RModel * = (RModel*)0);
		bool rmodelChanged(RModel* rmodel);
	private:
        GLuint renderProgram;
        GLuint generateProgram;
        GLuint transformFeedback;
        //std::vector<GLuint> isosurfaceBuffers;
        GLuint isosurfacesBuffer;
        GLuint triTableTex;
        IsosurfaceRendererConfig* config;
        Model* lastModel;
        std::vector<float> lastScalarLevels;
        void generateIsosurface(RModel* rmodel, std::vector<float> values);
        bool buildIsosurfaceRenderProgram();
        bool buildIsosurfaceGenerationProgram();
};

#endif // ISOSURFACERENDERER_H