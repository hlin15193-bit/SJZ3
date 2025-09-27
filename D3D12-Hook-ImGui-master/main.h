#include <windows.h>
#include <psapi.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>
#pragma comment(lib, "d3d12.lib")

#include "MinHook\Include\MinHook.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/imgui_impl_win32.h"

#include "SDK/SDK/Engine_classes.hpp"
#include "SDK/SDK/CoreUObject_structs.hpp"
#include "SDK/SDK/GPGameplay_classes.hpp"
#include "SDK/SDK/DFMGameplay_classes.hpp"

#include <fstream>
using namespace std;



inline SDK::UWorld* GWorld = nullptr;
inline SDK::AIntCharacter* GLocalCharacter = nullptr;
inline SDK::ULocalPlayer* GLocalPlayer = nullptr;
inline SDK::APlayerController* GPlayerController = nullptr;
inline SDK::FVector GLocalPlayerPos;
inline SDK::AIntCharacter* _player = nullptr;


inline int selected_key = 2;

inline bool 方框开关 = false; 
inline bool 骨骼开关 = true;
inline bool 人机开关 = true;

inline bool 名字开关 = true;
inline bool 干员开关 = true;
inline bool 手持开关 = false;

inline bool 头甲开关 = true;
inline bool 职业开关 = false;
inline bool 队标开关 = false;

inline bool 距离开关 = true;
inline bool 血条开关 = true;
inline bool 周围预警 = false;//加了没写功能

inline bool 战斗模式 = false;
//人机类
inline bool 人机血条 = false;
inline bool 人机盒子 = false;
inline bool 人物盒子 = true;
//容器类
inline bool 盒子透视 = false;
inline bool 绘制容器 = false;
inline bool 绘制密码 = false;

inline bool 高级容器 = false;
inline bool 中级容器 = false;
inline bool 低级容器 = false;

inline bool 过滤打开 = false;
inline bool 物品开关 = true;
inline bool 战场模式 = false;
//自瞄类
inline bool 漏打开关 = false;
inline bool 平滑自瞄 = false;
inline bool 追踪开关 = false;
//距离类

inline float 设置平滑 = 5.f;
inline float 玩家距离 = 100.0f;
inline float 人机距离 = 50.0f;
inline float 容器距离 = 50.0f;
inline float 骨灰距离 = 50.0f;
inline float 物品距离 = 300.0f;  // 新增物品距离
inline float 瞄准范围 = 50.0f;
inline float 预警距离 = 50.0f;
inline float 价值过滤 = 10000.0f;



inline ImColor 玩家不可见颜色 = ImColor(255, 240, 150, 255);
inline ImColor 玩家可见颜色 = ImColor(255, 0, 0, 255);
inline ImColor 人机可见颜色 = ImColor(255, 246, 143, 255);
inline ImColor 人机不可见颜色 = ImColor(240, 240, 240, 255);
inline ImColor 玩家盒子颜色 = ImColor(255, 0, 0, 255);
inline ImColor 人机盒子颜色 = ImColor(255, 255, 255, 255);
inline ImColor BOSS颜色 = ImColor(255, 192, 203);
inline ImColor 距离颜色 = ImColor(255, 255, 255, 255);
inline ImColor 玩家名字颜色 = ImColor(0, 255, 0, 255);

inline int selected_part = 0;

inline std::wstring GetBoneNameW(int index)
{
	const std::wstring bone_names[] = {
		L"head",      // 0 - 头
		L"neck",      // 1 - 脖子
		L"spine",     // 2 - 胸
		L"hips",      // 3 - 盆骨
		L"leftfoot",  // 4 - 左脚
		L"rightfoot", // 5 - 右脚
		L"leftleg",   // 6 - 左腿
		L"rightleg"   // 7 - 右腿
	};

	if (index >= 0 && index < (int)(sizeof(bone_names) / sizeof(bone_names[0])))
		return bone_names[index];
	else
		return L""; // 错误索引返回空字符串
}


inline int GetKeyCodeFromSelection(int selected_key)
{
	switch (selected_key)
	{
	case 0: return VK_LBUTTON;   // 鼠标左键
	case 1: return VK_RBUTTON;   // 鼠标右键
	case 2: return VK_SHIFT;     // Shift
	case 3: return VK_MENU;      // Alt
	case 4: return VK_CONTROL;   // Ctrl
	default: return 0;           // 无效
	}
}



inline std::string WStrToUtf8(const wchar_t* wsz)
{
	if (!wsz) return {};

	int len = WideCharToMultiByte(CP_UTF8, 0, wsz, -1, nullptr, 0, nullptr, nullptr);
	if (len <= 0) return {};

	std::string result(len - 1, 0); 
	WideCharToMultiByte(CP_UTF8, 0, wsz, -1, result.data(), len, nullptr, nullptr);
	return result;
}

#define U8(wstr_literal) WStrToUtf8(L##wstr_literal)

 inline bool Isvalidptr(void* ptr) {

	if (IsBadReadPtr(ptr, 8))return false;
	if ((DWORD64)ptr < 0x40000 || (DWORD64)ptr > 0x7FFFFFFF0000)return false;
	if (ptr == nullptr) return false;
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(ptr, &mbi, sizeof(mbi))) {
		return (mbi.State == MEM_COMMIT) &&
			(mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_READONLY | PAGE_EXECUTE_READ));
	}
	return false;
}

 inline SDK::FVector GetBonePos(SDK::USkeletalMeshComponent* Mesh, std::wstring BoneName)
 {
	 if (!Isvalidptr(Mesh)) return SDK::FVector{ 0.f, 0.f, 0.f };


	 SDK::FVector OutPos;
	 SDK::FRotator OutRot;

	 Mesh->TransformFromBoneSpace(SDK::UKismetStringLibrary::Conv_StringToName(SDK::FString(BoneName.c_str())), SDK::FVector(0.f, 0.f, 0.f), SDK::FRotator(0.f, 0.f, 0.f), &OutPos, &OutRot);

	 return OutPos;
 }

 inline SDK::FRotator GetAimRotation(SDK::FVector AimPos)
 {
	 SDK::FVector EnemyPos = AimPos;
	 SDK::FVector CameraPos = GetBonePos(GLocalCharacter->Mesh, L"head");

	 SDK::FVector diff{ EnemyPos.X - CameraPos.X, EnemyPos.Y - CameraPos.Y,EnemyPos.Z - CameraPos.Z, };

	 SDK::FRotator AimRotation;
	 AimRotation.Yaw = atan2f(diff.Y, diff.X) * 180 / (float)3.1415926;
	 AimRotation.Pitch = atan2f(diff.Z, sqrt(diff.X * diff.X + diff.Y * diff.Y)) * 180 / (float)3.1415926;

	 return AimRotation;
 }


 inline char dlldir[320];
 inline char* GetDirectoryFile(char* filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

 inline void Log(const char* fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirectoryFile((PCHAR)"log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

//=========================================================================================================================//

 inline WNDCLASSEX WindowClass;
 inline HWND WindowHwnd;

 inline bool InitWindow() {

	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = DefWindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.hIcon = NULL;
	WindowClass.hCursor = NULL;
	WindowClass.hbrBackground = NULL;
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = "MJ";
	WindowClass.hIconSm = NULL;
	RegisterClassEx(&WindowClass);
	WindowHwnd = CreateWindow(WindowClass.lpszClassName, "DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, WindowClass.hInstance, NULL);
	if (WindowHwnd == NULL) {
		return false;
	}
	return true;
}

 inline bool DeleteWindow() {
	DestroyWindow(WindowHwnd);
	UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
	if (WindowHwnd != NULL) {
		return false;
	}
	return true;
}

#if defined _M_X64
typedef uint64_t uintx_t;
#elif defined _M_IX86
typedef uint32_t uintx_t;
#endif

static uintx_t* MethodsTable = NULL;

//=========================================================================================================================//


namespace DirectX12 {
	inline bool Init() {

		if (InitWindow() == false) {
			return false;
		}

		HMODULE D3D12Module = GetModuleHandle("d3d12.dll");
		HMODULE DXGIModule = GetModuleHandle("dxgi.dll");
		if (D3D12Module == NULL || DXGIModule == NULL) {
			DeleteWindow();
			return false;
		}

		void* CreateDXGIFactory = GetProcAddress(DXGIModule, "CreateDXGIFactory");
		if (CreateDXGIFactory == NULL) {
			DeleteWindow();
			return false;
		}

		IDXGIFactory* Factory;
		if (((long(__stdcall*)(const IID&, void**))(CreateDXGIFactory))(__uuidof(IDXGIFactory), (void**)&Factory) < 0) {
			DeleteWindow();
			return false;
		}

		IDXGIAdapter* Adapter;
		if (Factory->EnumAdapters(0, &Adapter) == DXGI_ERROR_NOT_FOUND) {
			DeleteWindow();
			return false;
		}

		void* D3D12CreateDevice = GetProcAddress(D3D12Module, "D3D12CreateDevice");
		if (D3D12CreateDevice == NULL) {
			DeleteWindow();
			return false;
		}

		ID3D12Device* Device;
		if (((long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**))(D3D12CreateDevice))(Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&Device) < 0) {
			DeleteWindow();
			return false;
		}

		D3D12_COMMAND_QUEUE_DESC QueueDesc;
		QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		QueueDesc.Priority = 0;
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.NodeMask = 0;

		ID3D12CommandQueue* CommandQueue;
		if (Device->CreateCommandQueue(&QueueDesc, __uuidof(ID3D12CommandQueue), (void**)&CommandQueue) < 0) {
			DeleteWindow();
			return false;
		}

		ID3D12CommandAllocator* CommandAllocator;
		if (Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&CommandAllocator) < 0) {
			DeleteWindow();
			return false;
		}

		ID3D12GraphicsCommandList* CommandList;
		if (Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&CommandList) < 0) {
			DeleteWindow();
			return false;
		}

		DXGI_RATIONAL RefreshRate;
		RefreshRate.Numerator = 60;
		RefreshRate.Denominator = 1;

		DXGI_MODE_DESC BufferDesc;
		BufferDesc.Width = 100;
		BufferDesc.Height = 100;
		BufferDesc.RefreshRate = RefreshRate;
		BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SAMPLE_DESC SampleDesc;
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;

		DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
		SwapChainDesc.BufferDesc = BufferDesc;
		SwapChainDesc.SampleDesc = SampleDesc;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 2;
		SwapChainDesc.OutputWindow = WindowHwnd;
		SwapChainDesc.Windowed = 1;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* SwapChain;
		if (Factory->CreateSwapChain(CommandQueue, &SwapChainDesc, &SwapChain) < 0) {
			DeleteWindow();
			return false;
		}

		MethodsTable = (uintx_t*)::calloc(150, sizeof(uintx_t));
		memcpy(MethodsTable, *(uintx_t**)Device, 44 * sizeof(uintx_t));
		memcpy(MethodsTable + 44, *(uintx_t**)CommandQueue, 19 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19, *(uintx_t**)CommandAllocator, 9 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9, *(uintx_t**)CommandList, 60 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9 + 60, *(uintx_t**)SwapChain, 18 * sizeof(uintx_t));

		MH_Initialize();
		Device->Release();
		Device = NULL;
		CommandQueue->Release();
		CommandQueue = NULL;
		CommandAllocator->Release();
		CommandAllocator = NULL;
		CommandList->Release();
		CommandList = NULL;
		SwapChain->Release();
		SwapChain = NULL;
		DeleteWindow();
		return true;
	}
}

//=========================================================================================================================//

inline bool CreateHook(uint16_t Index, void** Original, void* Function) {
	assert(Index >= 0 && Original != nullptr && Function != nullptr);

	void* target = (void*)MethodsTable[Index];

	if (Index == 140) { 
		BYTE* p = (BYTE*)target;

		if (p[0] == 0xE9) {
			int32_t rel1 = *(int32_t*)(p + 1);
			BYTE* addr1 = p + 5 + rel1;

			if (addr1[0] == 0xE9) {
				int32_t rel2 = *(int32_t*)(addr1 + 1);
				BYTE* addr2 = addr1 + 5 + rel2;

				target = (void*)addr2; 
			}
		}
	}

	if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK) {
		return false;
	}
	return true;
}



//inline bool CreateHook(uint16_t Index, void** Original, void* Function) {
//	assert(_index >= 0 && _original != NULL && _function != NULL);
//	void* target = (void*)MethodsTable[Index];
//	if (Index == 140)
//	{
//		BYTE* p = (BYTE*)target;
//		if (p[0] == 0xE9) {
//
//			HMODULE hSteamOverlay = GetModuleHandleA("gameoverlayrenderer64.dll");
//			target = (void*)((uintptr_t)hSteamOverlay + 0x8F130);
//		}
//	}
//	if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK) {
//		return false;
//	}
//	return true;
//}

inline void DisableHook(uint16_t Index) {
	assert(Index >= 0);
	MH_DisableHook((void*)MethodsTable[Index]);
}

inline void DisableAll() {
	MH_DisableHook(MH_ALL_HOOKS);
	free(MethodsTable);
	MethodsTable = NULL;
}