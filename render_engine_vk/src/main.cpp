#include "render_engine.h"

#include <SDL.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void CreateDebugConsole()
{
	AllocConsole();

	FILE* dummy = nullptr;
	freopen_s(&dummy, "CONOUT$", "w", stdout);
	freopen_s(&dummy, "CONOUT$", "w", stderr);
}

int main(int argc, char* argv[])
{
    #ifdef _DEBUG
	    CreateDebugConsole();
    #endif

	RenderEngine::Init();
	RenderEngine& renderEngine = RenderEngine::Get();

	renderEngine.Run();

	RenderEngine::Destroy();

	return 0;
}
