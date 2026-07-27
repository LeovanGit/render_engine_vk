#include "render_engine.h"

#include <SDL.h>

int main(int argc, char* argv[])
{
	RenderEngine::Init();
	RenderEngine& renderEngine = RenderEngine::Get();

	renderEngine.Run();

	RenderEngine::Destroy();

	return 0;
}
