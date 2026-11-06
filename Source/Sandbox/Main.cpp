#include <Windows.h>
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <wrl/client.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <GraphicsEngine/GraphicsEngine.h>
#include <GraphicsEngine/RHI/GraphicsCommandList.h>
#include <GraphicsEngine/PostProcessing/BloomEffect.h>
#include <GraphicsEngine/PostProcessing/SSAOEffect.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "DemoScene.h"
#include "FlyCamera.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
using Microsoft::WRL::ComPtr;

namespace
{
	unsigned pendingWidth = 0;
	unsigned pendingHeight = 0;
	bool minimized = false;

	LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui::GetCurrentContext())
		{
			ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam);
		}
		switch (message)
		{
		case WM_SIZE:
		{
			minimized = wParam == SIZE_MINIMIZED;
			pendingWidth = LOWORD(lParam);
			pendingHeight = HIWORD(lParam);
			return 0;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			return 0;
		}
		default:
		{
			return DefWindowProcW(window, message, wParam, lParam);
		}
		}
	}

	struct Window
	{
		HWND Handle = nullptr;

		~Window()
		{
			if (Handle)
			{
				DestroyWindow(Handle);
			}
		}
	};

	struct Gui
	{
		Gui(HWND window, GraphicsEngine& graphics)
		{
			ImGui::CreateContext();
			ImGui::StyleColorsDark();
			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(graphics.GetNativeDevice(), graphics.GetNativeDeviceContext());
		}

		~Gui()
		{
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
	};

	// GPU readback makes smoke captures show the actual rendered framebuffer.
	void Capture(GraphicsEngine& graphics, const std::filesystem::path& path)
	{
		ComPtr<ID3D11RenderTargetView> target;
		graphics.GetNativeDeviceContext()->OMGetRenderTargets(1, &target, nullptr);
		if (!target)
		{
			throw std::runtime_error("No capture render target");
		}
		ComPtr<ID3D11Resource> resource;
		target->GetResource(&resource);
		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(resource.As(&texture)))
		{
			throw std::runtime_error("Cannot read capture target");
		}
		D3D11_TEXTURE2D_DESC description;
		texture->GetDesc(&description);
		description.Usage = D3D11_USAGE_STAGING;
		description.BindFlags = 0;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		description.MiscFlags = 0;
		ComPtr<ID3D11Texture2D> staging;
		if (FAILED(graphics.GetNativeDevice()->CreateTexture2D(&description, nullptr, &staging)))
		{
			throw std::runtime_error("Cannot create capture staging texture");
		}
		auto* context = graphics.GetNativeDeviceContext();
		context->CopyResource(staging.Get(), texture.Get());
		D3D11_MAPPED_SUBRESOURCE mapped;
		if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
		{
			throw std::runtime_error("Cannot map capture texture");
		}
		std::vector<unsigned char> pixels(static_cast<size_t>(description.Width) * description.Height * 4);
		for (unsigned y = 0; y < description.Height; ++y)
		{
			const auto* source = static_cast<const unsigned char*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch;
			auto* destination = pixels.data() + static_cast<size_t>(y) * description.Width * 4;
			for (unsigned x = 0; x < description.Width; ++x)
			{
				destination[x * 4] = source[x * 4 + 2];
				destination[x * 4 + 1] = source[x * 4 + 1];
				destination[x * 4 + 2] = source[x * 4];
				destination[x * 4 + 3] = 255;
			}
		}
		context->Unmap(staging.Get(), 0);
		BITMAPFILEHEADER fileHeader{};
		fileHeader.bfType = 0x4D42;
		fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		fileHeader.bfSize = fileHeader.bfOffBits + static_cast<DWORD>(pixels.size());
		BITMAPINFOHEADER info{};
		info.biSize = sizeof(info);
		info.biWidth = static_cast<LONG>(description.Width);
		info.biHeight = -static_cast<LONG>(description.Height);
		info.biPlanes = 1;
		info.biBitCount = 32;
		info.biCompression = BI_RGB;
		std::ofstream output(path, std::ios::binary);
		output.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
		output.write(reinterpret_cast<const char*>(&info), sizeof(info));
		output.write(reinterpret_cast<const char*>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
		if (!output)
		{
			throw std::runtime_error("Cannot save capture");
		}
	}

	void CheckDevice(GraphicsEngine& graphics, const std::filesystem::path& reportPath)
	{
		std::ofstream report(reportPath);
		const HRESULT removed = graphics.GetNativeDevice()->GetDeviceRemovedReason();
		report << "Device status: " << removed << "\n";
		ComPtr<ID3D11InfoQueue> messages;
		unsigned errors = 0;
		if (SUCCEEDED(graphics.GetNativeDevice()->QueryInterface(IID_PPV_ARGS(&messages))))
		{
			report << "Debug layer: available\n";
			for (UINT64 index = 0; index < messages->GetNumStoredMessagesAllowedByRetrievalFilter(); ++index)
			{
				SIZE_T size = 0;
				messages->GetMessage(index, nullptr, &size);
				std::vector<unsigned char> storage(size);
				auto* message = reinterpret_cast<D3D11_MESSAGE*>(storage.data());
				messages->GetMessage(index, message, &size);
				if (message->Severity <= D3D11_MESSAGE_SEVERITY_WARNING)
				{
					report << message->Severity << ": " << message->pDescription << "\n";
				}
				if (message->Severity <= D3D11_MESSAGE_SEVERITY_ERROR)
				{
					++errors;
				}
			}
		}
		if (!messages)
		{
			report << "Debug layer: unavailable; message validation skipped\n";
		}
		report << "D3D errors: " << errors << "\n";
		if (FAILED(removed) || errors != 0)
		{
			throw std::runtime_error("Graphics validation failed; see smoke-report.txt");
		}
	}

	int Run(HINSTANCE instance, bool smoke, bool emissionCheck)
	{
		wchar_t executablePath[32768]{};
		const DWORD length = GetModuleFileNameW(nullptr, executablePath, 32768);
		if (length == 0 || length >= 32768)
		{
			throw std::runtime_error("Cannot locate sandbox executable");
		}
		const auto executableRoot = std::filesystem::path(executablePath).parent_path();
		const auto contentRoot = executableRoot.parent_path().parent_path() / "Content";
		const auto shaderRoot = executableRoot / "EngineShaders";
		const auto captureRoot = executableRoot / "Captures";
		if (smoke)
		{
			std::filesystem::create_directories(captureRoot);
		}

		WNDCLASSW windowClass{};
		windowClass.lpfnWndProc = WindowProcedure;
		windowClass.hInstance = instance;
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.lpszClassName = L"GPGraphicsSandbox";
		if (!RegisterClassW(&windowClass))
		{
			throw std::runtime_error("Cannot register sandbox window");
		}
		RECT bounds{0, 0, 1280, 720};
		AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
		Window window;
		window.Handle = CreateWindowW(windowClass.lpszClassName, L"GP Sandbox", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		                              bounds.right - bounds.left, bounds.bottom - bounds.top, nullptr, nullptr, instance, nullptr);
		if (!window.Handle)
		{
			throw std::runtime_error("Cannot create sandbox window");
		}

		auto& graphics = GraphicsEngine::Get();
		auto bloom = std::make_shared<BloomEffect>();
		auto ssao = std::make_shared<SSAOEffect>();
		graphics.AddPostProcessEffect(bloom);
		graphics.AddPostProcessEffect(ssao);
		if (!graphics.Initialize(window.Handle, shaderRoot))
		{
			throw std::runtime_error("Cannot initialize graphics engine");
		}
		GraphicsCommandList commands;
		if (!graphics.CreateCommandList("Sandbox frame", commands))
		{
			throw std::runtime_error("Cannot create frame commands");
		}
		Gui gui(window.Handle, graphics);
		DemoScene scene(contentRoot, shaderRoot);
		FlyCamera fly;
		fly.Camera.SetResolution(graphics.GetClientSize());
		GraphicsRenderSettings settings;
		if (emissionCheck)
		{
			settings.RenderPass = RenderPassType::Emission;
			bloom->SetActive(false);
		}
		ShowWindow(window.Handle, SW_SHOW);
		bool running = true;
		unsigned frame = 0;
		auto previous = std::chrono::steady_clock::now();

		// One visible frame loop: input, assignment update, render, UI, present.
		while (running)
		{
			MSG message;
			while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
			{
				if (message.message == WM_QUIT)
				{
					running = false;
				}
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			if (!running)
			{
				break;
			}
			if (minimized)
			{
				WaitMessage();
				continue;
			}
			if (pendingWidth > 0 && pendingHeight > 0)
			{
				commands.ResetCommandList();
				if (!graphics.Resize(pendingWidth, pendingHeight) || !fly.Camera.SetResolution({pendingWidth, pendingHeight}))
				{
					throw std::runtime_error("Cannot resize graphics viewport");
				}
				pendingWidth = pendingHeight = 0;
			}
			const auto now = std::chrono::steady_clock::now();
			const float deltaTime = std::min(std::chrono::duration<float>(now - previous).count(), 0.25f);
			previous = now;
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::SetNextWindowPos({15, 15}, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize({320, 340}, ImGuiCond_FirstUseEver);
			ImGui::Begin("Graphics lab");
			ImGui::TextUnformatted("Hard-coded scene / DirectX 11");
			ImGui::TextUnformatted("WASD: fly | Space/Ctrl: up/down");
			ImGui::TextUnformatted("RMB: look | Shift: faster | Esc: quit");
			ImGui::SliderFloat("Camera speed", &fly.Speed, 50, 2000);
			if (ImGui::Button("Reset camera"))
			{
				fly.Reset();
			}
			ImGui::Checkbox("Animate FBX", &scene.Animate);
			ImGui::Checkbox("Translucent objects", &scene.ShowTranslucency);
			ImGui::Checkbox("Shadows", &scene.Shadows);
			bool bloomEnabled = bloom->IsActive();
			bool ssaoEnabled = ssao->IsActive();
			if (ImGui::Checkbox("Bloom", &bloomEnabled))
			{
				bloom->SetActive(bloomEnabled);
			}
			if (ImGui::Checkbox("SSAO", &ssaoEnabled))
			{
				ssao->SetActive(ssaoEnabled);
			}
			int tonemap = static_cast<int>(settings.TonemapAlgorithm);
			if (ImGui::Combo("Tonemapper", &tonemap, "None\0ACES\0Unreal\0Lottes\0"))
			{
				settings.TonemapAlgorithm = static_cast<TonemapType>(tonemap);
			}
			int renderPass = static_cast<int>(settings.RenderPass);
			if (ImGui::SliderInt("Render pass", &renderPass, 0, static_cast<int>(RenderPassType::Count) - 1))
			{
				settings.RenderPass = static_cast<RenderPassType>(renderPass);
			}
			ImGui::TextUnformatted(GraphicsRenderSettings::GetRenderPassName(settings.RenderPass));
			ImGui::End();
			if (!smoke)
			{
				const auto& input = ImGui::GetIO();
				fly.Update(window.Handle, deltaTime, input.WantCaptureKeyboard, input.WantCaptureMouse);
			}
			scene.Update(smoke ? 1.0f / 60.0f : deltaTime);
			ImGui::Render();
			auto snapshot = scene.GetSnapshot(fly.Camera);
			if (emissionCheck && frame < 10)
			{
				// Isolate the two forward-rendered chests before checking the opaque one after resize.
				std::erase_if(snapshot.Meshes, [](const MeshInstance* mesh)
				{
					return !mesh->Materials.empty() && mesh->Materials[0] && mesh->Materials[0]->GetName() == "ChestMaterial";
				});
			}
			commands.ResetCommandList();
			graphics.Render(commands, snapshot, settings);
			if (!commands.FinishCommandList())
			{
				throw std::runtime_error("Cannot finish frame commands");
			}
			graphics.ExecuteCommandList(commands);
			graphics.SetRenderTarget(graphics.GetBackBuffer(), nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			if (smoke && (frame == 5 || frame == 15 || frame == 25 || frame == 35))
			{
				Capture(graphics, captureRoot / ("frame-" + std::to_string(frame) + ".bmp"));
			}
			graphics.Present();
			if (smoke)
			{
				if (frame == 9)
				{
					bloom->SetActive(false);
					ssao->SetActive(false);
					scene.ShowTranslucency = false;
					scene.Shadows = false;
					SetWindowPos(window.Handle, nullptr, 0, 0, 1100, 700, SWP_NOMOVE | SWP_NOZORDER);
				}
				if (frame == 19)
				{
					settings.RenderPass = emissionCheck ? RenderPassType::Light : RenderPassType::NormalWorld;
					scene.Animate = false;
					if (emissionCheck)
					{
						scene.ShowTranslucency = true;
						scene.Shadows = true;
						ssao->SetActive(true);
					}
				}
				if (frame == 29)
				{
					bloom->SetActive(true);
					if (!emissionCheck)
					{
						ssao->SetActive(true);
						scene.Shadows = true;
						scene.ShowTranslucency = true;
						scene.Animate = true;
						settings.RenderPass = RenderPassType::Light;
						settings.TonemapAlgorithm = TonemapType::ACES;
					}
				}
				if (++frame == 40)
				{
					CheckDevice(graphics, captureRoot / "smoke-report.txt");
					running = false;
				}
			}
		}
		commands.ResetCommandList();
		graphics.GetNativeDeviceContext()->ClearState();
		return 0;
	}
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR arguments, int)
{
	const bool emissionCheck = std::wstring(arguments).find(L"--emission-check") != std::wstring::npos;
	const bool smoke = emissionCheck || std::wstring(arguments).find(L"--smoke") != std::wstring::npos;
	try
	{
		return Run(instance, smoke, emissionCheck);
	}
	catch (const std::exception& error)
	{
		std::ofstream("sandbox-error.log") << error.what() << "\n";
		if (!smoke)
		{
			MessageBoxA(nullptr, error.what(), "GP Sandbox", MB_OK | MB_ICONERROR);
		}
		return 1;
	}
}
