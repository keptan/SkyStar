#include "systems.h"
#include "entities.h"
#include "rtree.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

void lalaGameSetup (WorldSystems& world, GameState& state, QTree&, Graphics&)
{
	player(world);
}


auto main (void) -> int
{
	Graphics window(640,480);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(window.window.Get(), window.rendr.Get());
	ImGui_ImplSDLRenderer2_Init(window.rendr.Get());
	auto io = ImGui::GetIO();

	bool done = false;
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); // A nice background color
	while (!done)
	{
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		window.rendr.SetDrawColor(20, 0, 0, 255);
		window.rendr.Clear();
		static float f = 0.0f;
		static int counter = 0;

		ImGui::ShowDemoWindow(&show_demo_window);
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				if (event.type == SDL_QUIT)
					done = true;
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.window.Get()))
					done = true;
			}



			ImGui::Begin("Hello, world!"); // Create a window
			ImGui::Text("This is the SDL_Renderer backend."); // Display text

			ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bool
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats

			if (ImGui::Button("Button")) // Buttons return true when clicked
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), window.rendr.Get());
		window.rendr.Present();

	}
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	/*
	Game game;
	//game.sceneBuild()
	auto sweep = game.addSystem(sweeper);
	game.addSystem(playerMove);
	game.addSystem(pathSystem);
	game.addSystem(animationSystem);
	game.addSystem(outOfBounds);
	game.addSystem(renderSystem)->before(sweep);
	game.addSystem(spawnFireBolts);
	game.addSystem(velocitySystem);
	game.addSystem(spaceSystem);
	game.addSystem(collision)->after(sweep);

	game.addSetup(lalaGameSetup);

	game.setup();

	while (true)
	{
		int code = game.gameLoop();
		if (code!= 0) break;
	}*/
}