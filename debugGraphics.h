#pragma once
#include "engine.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"


	inline void graphicsSetup (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForSDLRenderer(graphics.window.Get(), graphics.rendr.Get());
		ImGui_ImplSDLRenderer2_Init(graphics.rendr.Get());
		ImGui::GetIO();
	}

	inline void graphicsRun (WorldSystems& world, GameState& state, QTree& space, Graphics& graphics)
	{
		bool toggleDebug = true;
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow(&toggleDebug);

		ImGui::Render();
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), graphics.rendr.Get());
	}

	inline void debugGraphicsClose ()
	{
		ImGui_ImplSDLRenderer2_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}
