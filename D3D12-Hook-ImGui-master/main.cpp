#include "main.h"
#include "hook22.h"
#include "byte_array.h"
#include <map>



int countnum = -1;
bool nopants_enabled = true;

//=========================================================================================================================//

typedef HRESULT(APIENTRY* Present12) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
Present12 oPresent = NULL;

typedef void(APIENTRY* DrawInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
DrawInstanced oDrawInstanced = NULL;

typedef void(APIENTRY* DrawIndexedInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
DrawIndexedInstanced oDrawIndexedInstanced = NULL;

typedef void(APIENTRY* ExecuteCommandLists)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);
ExecuteCommandLists oExecuteCommandLists = NULL;

//=========================================================================================================================//
inline bool ShowMenu = false;
bool ImGui_Initialised = false;

namespace Process {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	HMODULE Module;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}
namespace DirectX12Interface {
	ID3D12Device* Device = nullptr;
	ID3D12DescriptorHeap* DescriptorHeapBackBuffers;
	ID3D12DescriptorHeap* DescriptorHeapImGuiRender;
	ID3D12GraphicsCommandList* CommandList;
	ID3D12CommandQueue* CommandQueue;

	struct _FrameContext {
		ID3D12CommandAllocator* CommandAllocator;
		ID3D12Resource* Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
	};

	uintx_t BuffersCounts = -1;
	_FrameContext* FrameContext;
}

//=========================================================================================================================//

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ShowMenu) {
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
		//return true;
	}
	return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
}


ImVec2 screenCenter;
class Vec2
{
public:
	float x, y;
public:
	Vec2() :x(0.f), y(0.f) {}
	Vec2(float x_, float y_) :x(x_), y(y_) {}
	Vec2(ImVec2 ImVec2_) :x(ImVec2_.x), y(ImVec2_.y) {}
	Vec2 operator=(ImVec2 ImVec2_)
	{
		x = ImVec2_.x;
		y = ImVec2_.y;
		return *this;
	}
	Vec2 operator+(Vec2 Vec2_)
	{
		return { x + Vec2_.x,y + Vec2_.y };
	}
	Vec2 operator-(Vec2 Vec2_)
	{
		return { x - Vec2_.x,y - Vec2_.y };
	}
	Vec2 operator*(Vec2 Vec2_)
	{
		return { x * Vec2_.x,y * Vec2_.y };
	}
	Vec2 operator/(Vec2 Vec2_)
	{
		return { x / Vec2_.x,y / Vec2_.y };
	}
	Vec2 operator*(float n)
	{
		return { x / n,y / n };
	}
	Vec2 operator/(float n)
	{
		return { x / n,y / n };
	}
	bool operator==(Vec2 Vec2_)
	{
		return x == Vec2_.x && y == Vec2_.y;
	}
	bool operator!=(Vec2 Vec2_)
	{
		return x != Vec2_.x || y != Vec2_.y;
	}
	ImVec2 ToImVec2() const
	{
		return ImVec2(x, y);
	}
	float Length()
	{
		return sqrtf(powf(x, 2) + powf(y, 2));
	}
	float DistanceTo(const Vec2& Pos)
	{
		return sqrtf(powf(Pos.x - x, 2) + powf(Pos.y - y, 2));
	}
};
class Vec3
{
public:
	float x, y, z;

public:
	Vec3() :x(0.f), y(0.f), z(0.f) {}
	Vec3(float x_, float y_, float z_) :x(x_), y(y_), z(z_) {}
	Vec3 operator+(Vec3 Vec3_)
	{
		return { x + Vec3_.x,y + Vec3_.y,z + Vec3_.z };
	}
	Vec3 operator-(Vec3 Vec3_)
	{
		return { x - Vec3_.x,y - Vec3_.y,z - Vec3_.z };
	}
	Vec3 operator*(Vec3 Vec3_)
	{
		return { x * Vec3_.x,y * Vec3_.y,z * Vec3_.z };
	}
	Vec3 operator/(Vec3 Vec3_)
	{
		return { x / Vec3_.x,y / Vec3_.y,z / Vec3_.z };
	}
	Vec3 operator*(float n)
	{
		return { x * n,y * n,z * n };
	}
	Vec3 operator/(float n)
	{
		return { x / n,y / n,z / n };
	}
	bool operator==(Vec3 Vec3_)
	{
		return x == Vec3_.x && y == Vec3_.y && z == Vec3_.z;
	}
	bool operator!=(Vec3 Vec3_)
	{
		return x != Vec3_.x || y != Vec3_.y || z != Vec3_.z;
	}
	float Length()
	{
		return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
	}
	float DistanceTo(const Vec3& Pos)
	{
		return sqrtf(powf(Pos.x - x, 2) + powf(Pos.y - y, 2) + powf(Pos.z - z, 2));
	}
};
class Vec4 {
public:
	float x, y, z, w;

	bool IsValid() const
	{
		return !isinf(x) && !isinf(y) && !isinf(z) && !isinf(w);
	}


};

void Text_(std::string Text, Vec2 Pos, ImColor Color, float FontSize, bool KeepCenter)
{
	if (!KeepCenter)
	{
		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), FontSize, Pos.ToImVec2(), Color, Text.c_str());
	}
	else
	{
		float TextWidth = ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text.c_str()).x;
		ImVec2 Pos_ = { Pos.x - TextWidth / 2,Pos.y };
		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), FontSize, Pos_, Color, Text.c_str());
	}
}
void StrokeText(std::string Text, Vec2 Pos, ImColor Color, float FontSize, bool KeepCenter)
{
	Text_(Text, Vec2(Pos.x - 1, Pos.y + 1), ImColor(0, 0, 0), FontSize, KeepCenter);
	Text_(Text, Vec2(Pos.x - 1, Pos.y - 1), ImColor(0, 0, 0), FontSize, KeepCenter);
	Text_(Text, Vec2(Pos.x + 1, Pos.y + 1), ImColor(0, 0, 0), FontSize, KeepCenter);
	Text_(Text, Vec2(Pos.x + 1, Pos.y - 1), ImColor(0, 0, 0), FontSize, KeepCenter);
	Text_(Text, Pos, Color, FontSize, KeepCenter);
}


void AimBot(SDK::FVector2D Enemy_pos) {
	float TargetX = 0.0f;
	float TargetY = 0.0f;
	float Frevise = 0.5f;

	RECT rect;
	GetClientRect(FindWindow("UnrealWindow", NULL), &rect);
	
	if (Enemy_pos.X != 0) {
		if (Enemy_pos.X > screenCenter.x) {
			TargetX = -(screenCenter.x - Enemy_pos.X);
			TargetX /= 设置平滑;
			if (TargetX + screenCenter.x + 1 > rect.right) {
				TargetX = 0;
			}
			if (TargetX < 1 && TargetX > Frevise) {
				TargetX = 1;
			}
		}
		else if (Enemy_pos.X < screenCenter.x) {
			TargetX = Enemy_pos.X - screenCenter.x;
			TargetX /= 设置平滑;
			if (TargetX + screenCenter.x - 1 < 0) {
				TargetX = 0;
			}
			if (TargetX > -1 && TargetX < Frevise) {
				TargetX = -1;
			}
		}
	}

	if (Enemy_pos.Y != 0) {
		if (Enemy_pos.Y > screenCenter.y) {
			TargetY = -(screenCenter.y - Enemy_pos.Y);
			TargetY /= 设置平滑;
			if (TargetY + screenCenter.y + 1 > rect.bottom) {
				TargetY = 0;
			}
			if (TargetY < 1 && TargetY > Frevise) {
				TargetY = 1;
			}
		}
		else if (Enemy_pos.Y < screenCenter.y) {
			TargetY = Enemy_pos.Y - screenCenter.y;
			TargetY /= 设置平滑;
			if (TargetY + screenCenter.y - 1 < 0) {
				TargetY = 0;
			}
			if (TargetY > -1 && TargetY < Frevise) {
				TargetY = -1;
			}
		}
	}

	// 使用mouse_event移动鼠标
	mouse_event(MOUSEEVENTF_MOVE, static_cast<int>(TargetX), static_cast<int>(TargetY), 0, 0);
}

enum class DrawType {
	Text,
	StrokeText,
	Line,
	Line_,
	FilledRect,
	FillCircle,
	Rect,
	Circle  
};


struct DrawCommand {
	DrawType type;
	std::string text;
	Vec2 pos1;
	Vec2 pos2;
	ImColor color;
	float fontSize;
	bool center;
	float thickness;
};

std::vector<DrawCommand> g_DrawCommands;

void QueueText(const std::string& text, Vec2 pos, ImColor color, float size = 20.f, bool center = true) {
	
	g_DrawCommands.push_back({ DrawType::Text, text, pos, {}, color, size, center, 0.f });
}

void QueueStrokeText(const std::string& text, Vec2 pos, ImColor color, float size = 20.f, bool center = true) {
	
	g_DrawCommands.push_back({ DrawType::StrokeText, text, pos, {}, color, size, center, 0.f });
}

void QueueLine(Vec2 from, Vec2 to, ImColor color, float thickness = 1.f) {
	
	g_DrawCommands.push_back({ DrawType::Line, "", from, to, color, 0.f, false, thickness });
}

void QueueLine_(Vec2 from, Vec2 to, ImColor color, float thickness = 1.f) {
	
	g_DrawCommands.push_back({ DrawType::Line_, "", from, to, color, 0.f, false, thickness });
}

void QueueFilledRect(Vec2 pos, Vec2 size, ImColor color) {
	
	g_DrawCommands.push_back({ DrawType::FilledRect, "", pos, size, color, 0.f, false, 0.f });
}

void QueueRect(Vec2 pos, Vec2 size, ImColor color, float thickness = 1.f) {
	
	g_DrawCommands.push_back({ DrawType::Rect, "", pos, size, color, 0.f, false, thickness });
}

void QueueFillCircle(Vec2 center, float radius, ImColor color, int segments = 16) {
	
	g_DrawCommands.push_back({ DrawType::FillCircle, "", center, Vec2(radius, (float)segments), color, 0.f, false, 0.f });
}

void QueueCircle(Vec2 center, float radius, ImColor color, float thickness = 1.f, int segments = 32) {
	
	g_DrawCommands.push_back({ DrawType::Circle, "", center, Vec2(radius, (float)segments), color, 0.f, false, thickness });
}



ImColor GetColorForTeam(int TeamID) {
	// 使用取模运算确保TeamID在0-9范围内循环
	switch (TeamID % 10) {
	case 0: return ImColor(0x34, 0x98, 0xdb, 0xFF);  // 明亮蓝
	case 1: return ImColor(0xe7, 0x4c, 0x3c, 0xFF);  // 鲜艳红
	case 2: return ImColor(0x2e, 0xcc, 0x71, 0xFF);  // 翡翠绿
	case 3: return ImColor(0x9b, 0x59, 0xb6, 0xFF);  // 深紫色
	case 4: return ImColor(0x34, 0x49, 0x5e, 0xFF);  // 深蓝灰
	case 5: return ImColor(0xf1, 0xc4, 0x0f, 0xFF);  // 亮黄色
	case 6: return ImColor(0xe6, 0x7e, 0x22, 0xFF);  // 橙黄色
	case 7: return ImColor(0x1a, 0xbc, 0x9c, 0xFF);  // 绿松石色
	case 8: return ImColor(0x8e, 0x44, 0xad, 0xFF);  // 深紫红色
	case 9: return ImColor(0xec, 0xf0, 0xf1, 0xFF);  // 浅灰白色
	default: return ImColor(0x27, 0xae, 0x60, 0xFF); // 默认青绿色
	}
}
std::string WCharToString(const std::wstring& wstr) {
	if (wstr.empty()) {
		return "";
	}

	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string str(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], sizeNeeded, NULL, NULL);

	return str;
}





bool WorldToScreen(SDK::FVector pos, SDK::FVector2D& pm)
{
	return GPlayerController->ProjectWorldLocationToScreen(pos, &pm, false);
}

bool LineTraceSingle(SDK::TArray<SDK::AActor*> EmptyActors,SDK::FVector Start, SDK::FVector End)
{
	if (!Isvalidptr(GWorld))return false;
	SDK::FHitResult OutHit;
	return !SDK::UKismetSystemLibrary::LineTraceSingle(GWorld, Start, End, 0, false, EmptyActors, 0, &OutHit, true, { 0,0,0,0 }, { 0,0,0,0 }, 0.1f);
}


void DrawSkeleton(SDK::ACHARACTER* characterBase, ImColor Color)
{
	if (!Isvalidptr(characterBase) || !Isvalidptr(characterBase->Mesh))
		return;

	// 所有需要连接的骨骼对（名字都要用小写、你自己的 BoneName）
	std::vector<std::pair<std::wstring, std::wstring>> boneConnections = {
		{L"head", L"neck"},
		{L"neck", L"spine"},
		{L"spine", L"hips"},

		{L"neck", L"leftshoulder"},
		{L"leftshoulder", L"leftarm"},
		{L"leftarm", L"leftforearm"},
		{L"leftforearm", L"lefthand"},

		{L"neck", L"rightshoulder"},
		{L"rightshoulder", L"rightarm"},
		{L"rightarm", L"rightforearm"},
		{L"rightforearm", L"righthand"},

		{L"hips", L"leftupleg"},
		{L"leftupleg", L"leftleg"},
		{L"leftleg", L"leftfoot"},

		{L"hips", L"rightupleg"},
		{L"rightupleg", L"rightleg"},
		{L"rightleg", L"rightfoot"}
	};

	for (const auto& [parentName, childName] : boneConnections)
	{
		SDK::FVector parentWorld = GetBonePos(characterBase->Mesh, parentName);
		SDK::FVector childWorld = GetBonePos(characterBase->Mesh, childName);

		SDK::FVector2D parentScreen, childScreen;
		if (WorldToScreen(parentWorld, parentScreen) && WorldToScreen(childWorld, childScreen))
		{
			Vec2 from = { parentScreen.X, parentScreen.Y };
			Vec2 to = { childScreen.X, childScreen.Y };
			QueueLine_(from, to, Color, 2.0f);
		}
	}
}

std::vector<Vec2> 容器叠加数组;
std::vector<Vec2> 物品叠加数组;

bool 不在叠加数组中(const Vec2& 坐标, const std::vector<Vec2>& 数组) {
	for (const auto& e : 数组) {
		if (坐标.x == e.x && 坐标.y == e.y)
			return false;
	}
	return true;
}

bool 叠加坐标(Vec2& 坐标, const std::vector<Vec2>& 数组) {
	const int 最小间距 = 15;
	const int 最大尝试次数 = 10;
	int 尝试次数 = 0;

	float 原始Y = 坐标.y;

	while (尝试次数 < 最大尝试次数) {
		bool 被挤压 = false;

		for (const auto& e : 数组) {
			if (std::abs(坐标.x - e.x) < 150 &&
				std::abs(坐标.y - e.y) < 最小间距) {

				// 优先往下推（避免文字越推越上）
				if (e.y >= 原始Y) {
					坐标.y = e.y + 最小间距;
				}
				else {
					坐标.y = e.y - 最小间距;
				}

				被挤压 = true;
				break;
			}
		}

		if (!被挤压)
			return true;

		尝试次数++;
	}

	// 如果失败，就保持原始坐标
	坐标.y = 原始Y;
	return false;
}


std::string GetWeaponName(uint64_t id)
{
	// 一、冲锋枪（SMG）
	if (id == 18020000001) return U8("MP5");
	else if (id == 18020000002) return U8("P90");
	else if (id == 18020000003) return U8("Vector");
	else if (id == 18020000004) return U8("UZI");
	else if (id == 18020000005) return U8("野牛");
	else if (id == 18020000006) return U8("SMG-45");
	else if (id == 18020000008) return U8("SR-3M");
	else if (id == 18020000009) return U8("勇士");
	else if (id == 18020000010) return U8("MP7");
	else if (id == 18020000011) return U8("QCQ171");

	// 二、手枪
	else if (id == 18070000002) return U8("QSZ92G");
	else if (id == 18070000003) return U8(".357左轮");
	else if (id == 18070000004) return U8("沙漠之鹰");
	else if (id == 18070000005) return U8("G18");
	else if (id == 18070000006) return U8("93R");
	else if (id == 18070000010) return U8("G17");
	else if (id == 18070000033) return U8("M1911");
	else if (id == 18070000030) return U8("峰医激素枪");  // 特殊手枪

	// 三、步枪
	else if (id == 18010000001) return U8("M4A1");
	else if (id == 18010000006) return U8("AKM");
	else if (id == 18010000008) return U8("QBZ95-1");
	else if (id == 18010000010) return U8("AKS-74U");
	else if (id == 18010000012) return U8("ASh-12");
	else if (id == 18010000013) return U8("K416");
	else if (id == 18010000014) return U8("M16A4");
	else if (id == 18010000015) return U8("AUG");
	else if (id == 18010000016) return U8("M7");
	else if (id == 18010000017) return U8("SG552");
	else if (id == 18010000018) return U8("AK-12");
	else if (id == 18010000021) return U8("SCAR-H");
	else if (id == 18010000023) return U8("G3");
	else if (id == 18010000024) return U8("PTR-32");
	else if (id == 18010000031) return U8("CAR-15");
	else if (id == 18010000037) return U8("AS VAL");
	else if (id == 18010000038) return U8("191式");
	else if (id == 18010000040) return U8("K437");  // 新增步枪
	else if (id == 18010000042) return U8("KC17");  // 新增步枪

	// 四、狙击枪/精确射手步枪
	else if (id == 18050000002) return U8("Mini-14");
	else if (id == 18050000003) return U8("VSS");
	else if (id == 18050000004) return U8("SVD");
	else if (id == 18050000005) return U8("M14");
	else if (id == 18050000006) return U8("SKS");
	else if (id == 18050000007) return U8("SR-25");
	else if (id == 18050000008) return U8("SR9");
	else if (id == 18050000031) return U8("PSG-1");
	else if (id == 18060000007) return U8("SV-98");
	else if (id == 18060000008) return U8("R93");
	else if (id == 18060000009) return U8("M700");
	else if (id == 18060000011) return U8("AWM");

	// 五、机枪
	else if (id == 18040000001) return U8("PKM");
	else if (id == 18040000002) return U8("M249");
	else if (id == 18040000003) return U8("M250");
	else if (id == 18040000004) return U8("QJB201");
	else if (id == 18040000008) return U8("机枪");

	// 六、霰弹枪
	else if (id == 18030000001) return U8("M1014");
	else if (id == 18030000002) return U8("S12K");
	else if (id == 18030000004) return U8("M870");
	else if (id == 18030000005) return U8("727");  // 新增霰弹枪

	// 七、特殊武器（含弓类）
	else if (id == 18130000001) return U8("电击箭矢");
	else if (id == 18130000002) return U8("侦察箭矢");
	else if (id == 18150000001) return U8("复合弓");  // 新增复合弓
	else if (id == 18080000009) return U8("巡飞弹");

	// 八、发射器（榴弹/火箭筒等）
	else if (id == 18080000003) return U8("AT-4发射器");
	else if (id == 18080000005) return U8("毒刺发射器");
	else if (id == 18080000007) return U8("火箭筒");
	else if (id == 18080000010) return U8("三联装手炮");
	else if (id == 18080000011) return U8("虎蹲炮");
	else if (id == 18080000012) return U8("反人员榴弹");
	else if (id == 18080000013) return U8("EMP榴弹");
	else if (id == 18080000014) return U8("标地发射器");
	else if (id == 18080000019) return U8("高爆榴弹发射器");
	else if (id == 18080000020) return U8("烟雾榴弹发射器");

	// 九、近战武器
	else if (id >= 18100000000 && id <= 18100000020) return U8("刀/X");  // 刀类范围

	// 十、投掷物/装备
	else if (id == 21010000001) return U8("增强破片手雷");
	else if (id == 21010000003) return U8("数据飞刀");
	else if (id == 21010000005) return U8("速凝掩体");
	else if (id == 21010000006) return U8("燃烧弹");
	else if (id == 21010000012) return U8("磁吸炸弹");
	else if (id == 21010000017) return U8("流萤集群系统");
	else if (id == 21010000018) return U8("致盲瓦斯弹");
	else if (id == 21010000022) return U8("强化破片手雷");
	else if (id == 21020300006) return U8("声波陷阱");
	else if (id == 21020300007) return U8("烟幕弹");
	else if (id == 21020300009) return U8("弹药箱");
	else if (id == 21020300010) return U8("重生信标");
	else if (id == 21020300011) return U8("工程箱");
	else if (id == 21020300012) return U8("医药箱");
	else if (id == 21020300013) return U8("ADS近防系统");
	else if (id == 21020300017) return U8("反坦克地雷");
	else if (id == 21020300018) return U8("阔剑地雷");

	// 十一、设施/装置
	else if (id == 18080000008) return U8("镭射指示器");
	else if (id == 18990000003) return U8("机枪塔");
	else if (id == 18990000007) return U8("防空炮");
	else if (id == 18990000008) return U8("岸防炮");

	// 未知武器
	return U8("未知武器 ") + std::to_string(id);
}
std::string GetOperatorName(int id)
{
	switch (id)
	{
	case 25: return U8("威龙");
	case 26: return U8("骇爪");
	case 27: return U8("蜂医");
	case 28: return U8("露娜");
	case 29: return U8("牧羊人");
	case 30: return U8("红狼");
	case 35: return U8("乌鲁鲁");
	case 36: return U8("佐娅");
	case 37: return U8("深蓝");
	case 38: return U8("无名");
	case 39: return U8("疾风");
	default: return std::to_string(id);
	}
}
int GetArmorLevel(int id)
{
	if (id == 105 || id == 106 || id == 107 || id == 108)
		return 1;
	else if (id == 81 || id == 82 || id == 83 || id == 84)
		return 2;
	else if (id == 57 || id == 58 || id == 59 || id == 60)
		return 3;
	else if (id == 33 || id == 34 || id == 35 || id == 36 || id == 37 || id == 39 || id == 40)
		return 4;
	else if (id == 9 || id == 10 || id == 11 || id == 12 || id == 17 || id == 18)
		return 5;
	else if (id == 242 || id == 243 || id == 244)
		return 6;
	else
		return id;
}

ImColor GetArmorColor(int ID)
{
	switch (ID)
	{
	case 1: return ImColor(200, 200, 200);    // 轻灰色，代表基础等级
	case 2: return ImColor(100, 220, 100);    // 柔和绿色，代表入门护甲
	case 3: return ImColor(100, 180, 255);    // 淡天蓝，代表中级护甲
	case 4: return ImColor(180, 70, 255);   // 紫色
	case 5: return ImColor(255, 230, 100);    // 柠檬黄，明亮但不刺眼
	case 6: return ImColor(255, 120, 120);    // 淡红色，警告与强力感
	default: return ImColor(150, 150, 150);   // 中灰色，未知或默认
	}
}



void MyCheckBox(const char* str_id, bool* v)
{
	float scale = 0.70f;  // 控件缩放比例
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	float Height = ImGui::GetFrameHeight() * scale;
	float Width = Height * 1.7f;
	float Radius = Height / 2 - 2 * scale;

	// 1. 先绘制文本（左侧）
	ImGui::TextUnformatted(str_id);
	ImGui::SameLine(0, 5.0f);  // 文本和开关间距 5px

	// 2. 更新开关位置（文本右侧）
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton(str_id, ImVec2(Width, Height));
	if (ImGui::IsItemClicked())
		*v = !(*v);

	// 3. 动画逻辑
	float t = *v ? 1.0f : 0.f;
	ImGuiContext& g = *GImGui;
	float AnimationSpeed = 0.08f;
	if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
	{
		float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
		t = *v ? T_Animation : (1.0f - T_Animation);
	}

	// 4. 颜色逻辑（悬停/未悬停状态）
	ImU32 Color;
	if (ImGui::IsItemHovered())
		Color = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.24f, 0.15f, 1.0f), ImVec4(0.55f, 0.85f, 0.13f, 1.0f), t));
	else
		Color = ImGui::GetColorU32(ImLerp(ImVec4(0.90f, 0.29f, 0.20f, 1.0f), ImVec4(0.60f, 0.90f, 0.18f, 1.0f), t));

	// 5. 绘制开关背景和滑块
	DrawList->AddRectFilled(p, ImVec2(p.x + Width, p.y + Height), Color, Height);
	DrawList->AddCircleFilled(
		ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2) * scale, p.y + Radius + 2 * scale),
		Radius, IM_COL32(255, 255, 255, 255), 360);
	DrawList->AddCircle(
		ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2) * scale, p.y + Radius + 2 * scale),
		Radius, IM_COL32(20, 20, 20, 80), 360, 1);
}

struct tab_element {
	float element_opacity = 0.f;
	float rect_opacity = 0.f;
	float text_opacity = 0.f;
};


bool CustomTab(const char* icon, ImFont* icon_font, bool selected)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems || icon == nullptr || *icon == '\0')
		return false;

	if (!icon_font)
		icon_font = ImGui::GetFont();
	if (!icon_font)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(icon); // 使用 icon 内容作为唯一 ID

	const float icon_font_size = 15.0f;
	ImVec2 icon_size = icon_font->CalcTextSizeA(icon_font_size, FLT_MAX, 0.0f, icon);

	const float padding = 12.0f;
	const float tab_height = 31.0f;
	const float tab_width = icon_size.x + padding * 2;

	ImVec2 pos = window->DC.CursorPos;
	ImRect rect(pos, ImVec2(pos.x + tab_width, pos.y + tab_height));
	ImGui::ItemSize(rect, style.FramePadding.y);
	if (!ImGui::ItemAdd(rect, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);

	ImGuiStorage* storage = window->DC.StateStorage;
	ImGuiID anim_id = id;

	float* element_opacity = storage->GetFloatRef(anim_id + 0, 0.0f);
	float* rect_opacity = storage->GetFloatRef(anim_id + 1, 0.0f);
	float* text_opacity = storage->GetFloatRef(anim_id + 2, 0.3f);

	float dt = ImGui::GetIO().DeltaTime;
	*element_opacity = ImLerp(*element_opacity, (selected ? 0.04f : hovered ? 0.01f : 0.0f), 0.07f * (1.0f - dt));
	*rect_opacity = ImLerp(*rect_opacity, (selected ? 1.0f : 0.0f), 0.15f * (1.0f - dt));
	*text_opacity = ImLerp(*text_opacity, (selected ? 1.0f : hovered ? 0.5f : 0.3f), 0.07f * (1.0f - dt));

	// 背景
	window->DrawList->AddRectFilled(rect.Min, rect.Max, ImColor(1.f, 1.f, 1.f, *element_opacity), 3.0f);

	// 图标绘制（居中）
	ImVec2 icon_pos(
		rect.Min.x + (rect.GetWidth() - icon_size.x) * 0.5f,
		rect.Min.y + (tab_height - icon_font_size) * 0.5f
	);
	window->DrawList->AddText(icon_font, icon_font_size, icon_pos, ImColor(1.f, 1.f, 1.f, *text_opacity), icon);

	// 可选：右侧指示条（取消注释启用）
	
	if (*rect_opacity > 0.01f) {
		window->DrawList->AddRectFilled(
			ImVec2(rect.Max.x - 4.0f, rect.Min.y + 6.0f),
			ImVec2(rect.Max.x, rect.Max.y - 6.0f),
			ImColor(147 / 255.f, 190 / 255.f, 66 / 255.f, *rect_opacity),
			2.0f, ImDrawFlags_RoundCornersLeft);
	}
	

	return pressed;
}

ImFont* tab_icons;


#define ICON_TAB1 "B"
#define ICON_TAB2 "C"
#define ICON_TAB3 "D"
#define ICON_TAB4 "E"


const char* icons[] = { ICON_TAB1, ICON_TAB2, ICON_TAB3, ICON_TAB4 };

  // 默认选中“鼠标左键”

void ImColorToFloat3(const ImColor& color, float outColor[3])
{
	outColor[0] = color.Value.x;
	outColor[1] = color.Value.y;
	outColor[2] = color.Value.z;
}

// float[3] 转 ImColor
ImColor Float3ToImColor(const float inColor[3])
{
	return ImColor(inColor[0], inColor[1], inColor[2]);
}

bool ColorEdit3WithImColor(const char* label, ImColor& color)
{
	float tmp[3];
	ImColorToFloat3(color, tmp);
	bool changed = ImGui::ColorEdit3(label, tmp);
	if (changed)
		color = Float3ToImColor(tmp);
	return changed;
}

void ShowMyMenu2()
{
	ImGui::SetNextWindowSize(ImVec2(542, 500), ImGuiCond_Once);

	static int selected_tab = 0;

	ImGuiStyle& style = ImGui::GetStyle();
	float old_rounding = style.ChildRounding;
	style.ChildRounding = 8.0f; // 设置你想要的圆角大小

	ImGui::Begin(U8("[INS显示/隐藏]").c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	float tab_width = 50.0f;

	ImGui::BeginChild(U8("TabList").c_str(), ImVec2(tab_width, 0), true);
	{
		for (int i = 0; i < 4; ++i)
		{
			ImGui::PushID(i);
			if (CustomTab(icons[i], tab_icons, selected_tab == i))
				selected_tab = i;
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild(U8("ContentArea").c_str(), ImVec2(0, 0), true);
	{
		if (selected_tab == 0)
		{
			/*ImGui::Text(U8("常规设置").c_str());*/
			ImGui::Separator();
			MyCheckBox(U8("显示方框").c_str(), &方框开关);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示骨骼").c_str(), &骨骼开关); 
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示人机").c_str(), &人机开关);
			ImGui::SameLine();

			ImGui::Dummy(ImVec2(0.0f, 45));
			MyCheckBox(U8("显示名字").c_str(), &名字开关);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示干员").c_str(), &干员开关);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示手持").c_str(), &手持开关);
			ImGui::SameLine();

			ImGui::Dummy(ImVec2(0.0f, 45));
			MyCheckBox(U8("显示头甲").c_str(), &头甲开关); 
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示职业").c_str(), &职业开关);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示队标").c_str(), &队标开关);
			ImGui::SameLine();

			ImGui::Dummy(ImVec2(0.0f, 45));
			MyCheckBox(U8("显示距离").c_str(), &距离开关);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示血条").c_str(), &血条开关);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("周围预警").c_str(), &周围预警);
			ImGui::SameLine();

			ImGui::Dummy(ImVec2(0.0f, 45));
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("玩家距离").c_str(), &玩家距离, 50.0f, 300.0f, "%.0f");
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("人机距离").c_str(), &人机距离, 50.0f, 200.0f, "%.0f");
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("预警距离").c_str(), &预警距离, 50.0f, 300.0f, "%.0f");


		}
		else if (selected_tab == 1)
		{
			/*ImGui::Text(U8("视觉设置").c_str());*/
			MyCheckBox(U8("人机血条").c_str(), &人机血条);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("人机盒子").c_str(), &人机盒子);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("玩家盒子").c_str(), &人物盒子);

			ImGui::Dummy(ImVec2(0.0f, 20));
			MyCheckBox(U8("盒子透视").c_str(), &盒子透视);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("绘制容器").c_str(), &绘制容器);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("绘制密码").c_str(), &绘制密码);

			ImGui::Dummy(ImVec2(0.0f, 20));
			MyCheckBox(U8("高级容器").c_str(), &高级容器);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("中级容器").c_str(), &中级容器);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("低级容器").c_str(), &低级容器);

			ImGui::Dummy(ImVec2(0.0f, 20));
			MyCheckBox(U8("过滤打开").c_str(), &过滤打开);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("显示物品").c_str(), &物品开关);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("战场模式").c_str(), &战场模式);


			ImGui::SameLine();
			ImGui::Dummy(ImVec2(0.0f, 30));
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("盒子距离").c_str(), &骨灰距离, 50.0f, 200.0f, "%.0f");
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("容器距离").c_str(), &容器距离, 50.0f, 200.0f, "%.0f");
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("物品距离").c_str(), &物品距离, 50.0f, 500.0f, "%.0f");
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("价值过滤").c_str(), &价值过滤, 1000.0f, 100000, "%.0f");

		}
		else if (selected_tab == 2)
		{
			/*ImGui::Text(U8("武器设置页").c_str());*/
			MyCheckBox(U8("漏打自瞄").c_str(), &漏打开关); ImGui::SameLine();
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("平滑自瞄").c_str(), &平滑自瞄);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(48.2f, 0)); ImGui::SameLine();
			MyCheckBox(U8("子弹追踪").c_str(), &追踪开关); 
			ImGui::SameLine();

			ImGui::Dummy(ImVec2(0.0f, 30));

			std::string key_labels[] = {
	           U8("鼠标左键"),
	           U8("鼠标右键"),
	           U8("Shift 键"),
	           U8("Alt 键"),
	           U8("Ctrl 键")
	         };

			const char* key_items[sizeof(key_labels) / sizeof(std::string)];
			for (size_t i = 0; i < sizeof(key_labels) / sizeof(std::string); ++i) {
				key_items[i] = key_labels[i].c_str();
			}

			// 使用 ImGui Combo
			ImGui::Text(U8("触发键位:").c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(120.0f);
			ImGui::Combo("##KeySelector", &selected_key, key_items, IM_ARRAYSIZE(key_items));

			ImGui::SameLine();
			ImGui::Dummy(ImVec2(40.0f, 0));

			std::string lock_parts[] = {
	            U8("头"), U8("脖子"), U8("胸"), U8("盆骨"),
	            U8("左脚"), U8("右脚"),
	            U8("左腿"), U8("右腿")
			};

			const char* key_lock[sizeof(lock_parts) / sizeof(std::string)];
			for (size_t i = 0; i < sizeof(lock_parts) / sizeof(std::string); ++i) {
				key_lock[i] = lock_parts[i].c_str();
			}

			ImGui::Text(U8("锁定部位:").c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(120.0f);
			ImGui::Combo("##LockPartSelector", &selected_part, key_lock, IM_ARRAYSIZE(key_lock));

			ImGui::Dummy(ImVec2(0.0f, 45));
			ImGui::SetNextItemWidth(300.0f);
			ImGui::SliderFloat(U8("瞄准范围").c_str(), &瞄准范围, 10.0f, 200.0f, "%.0f");
			ImGui::SetNextItemWidth(250.0f);
			ImGui::SliderFloat(U8("平滑参数").c_str(), &设置平滑, 1.0f, 20.0f, "%.0f"); ImGui::SameLine();
			ImGui::TextColored(ImColor(255,0,0,255), U8("参数越低越锁").c_str());
		}

		else if (selected_tab == 3)
		{
			/*ImGui::Text(U8("其他功能").c_str());*/
			ColorEdit3WithImColor(U8("玩家可见颜色").c_str(), 玩家可见颜色);
			ColorEdit3WithImColor(U8("人机可见颜色").c_str(), 人机可见颜色);
			ColorEdit3WithImColor(U8("玩家不可见颜色").c_str(), 玩家不可见颜色);
			ColorEdit3WithImColor(U8("玩家盒子颜色").c_str(), 玩家盒子颜色);
			ColorEdit3WithImColor(U8("人机盒子颜色").c_str(), 人机盒子颜色);
			ColorEdit3WithImColor(U8("BOSS颜色").c_str(), BOSS颜色);
			ColorEdit3WithImColor(U8("距离颜色").c_str(), 距离颜色);
			ColorEdit3WithImColor(U8("玩家名字颜色").c_str(), 玩家名字颜色);
		}
	}
	ImGui::EndChild();

	ImGui::End();
}



float FindNearestCoordinate(Vec2 pos, Vec2 center) {
	return (sqrt(pow(pos.x - center.x, 2) + pow(pos.y - center.y, 2)));
}

bool IsContainer(std::string itemCode)
{
	// ========== 安全防护层 ==========
	// 1. 参数安全检查
	if (itemCode.empty()) {
		return false;
	}

	// ========== 容器判断逻辑 ==========
	// 采用更高效的判断顺序：按出现频率从高到低判断

	// 1. 高级容器判断 (合并相同菜单条件的判断)
	if (高级容器) {
		if (itemCode.find(U8("保险箱")) != std::string::npos ||
			itemCode.find(U8("BP_Interactor_Container_SafeBox_C")) != std::string::npos ||
			itemCode.find(U8("鸟窝")) != std::string::npos ||
			itemCode.find(U8("医疗物资堆")) != std::string::npos ||
			itemCode.find(U8("保险柜")) != std::string::npos ||
			itemCode.find(U8("航空储物箱")) != std::string::npos ||
			itemCode.find(U8("高级储物箱")) != std::string::npos) {
			return true;
		}
	}

	// 2. 中级容器判断
	if (中级容器) {
		if (itemCode.find(U8("医疗包")) != std::string::npos ||
			itemCode.find(U8("高级旅行箱")) != std::string::npos ||
			itemCode.find(U8("大武器箱")) != std::string::npos ||
			itemCode.find(U8("手提箱")) != std::string::npos ||
			itemCode.find(U8("野外战备箱")) != std::string::npos) {
			return true;
		}
	}

	// 3. 低级容器判断
	if (低级容器) {
		if (itemCode.find(U8("弹药箱")) != std::string::npos ||
			itemCode.find(U8("工具柜")) != std::string::npos ||
			itemCode.find(U8("快递箱")) != std::string::npos ||
			itemCode.find(U8("藏匿物")) != std::string::npos ||
			itemCode.find(U8("收纳盒")) != std::string::npos ||
			itemCode.find(U8("电脑")) != std::string::npos ||
			itemCode.find(U8("一件衣服")) != std::string::npos) {
			return true;
		}
	}

	return false;
}

float MaxRange = 180.f;
int 预警人数 = 0;
void draw()
{

	if (!Isvalidptr(GWorld->OwningGameInstance))return;
	auto GameInstance = GWorld->OwningGameInstance;
	if (!Isvalidptr(GameInstance->LocalPlayers[0]))return;
	GLocalPlayer = GameInstance->LocalPlayers[0];
	if (!Isvalidptr(GLocalPlayer->PlayerController))return;
	GPlayerController = GLocalPlayer->PlayerController;
	if (!Isvalidptr(GPlayerController->AcknowledgedPawn))return;
	auto LocalPlayer = GPlayerController->AcknowledgedPawn;
	GLocalCharacter = reinterpret_cast<SDK::AGPCharacterBase*>(LocalPlayer);
	GLocalPlayerPos = LocalPlayer->K2_GetActorLocation();
	if (!Isvalidptr(GLocalCharacter->PlayerState_))return;
	auto 本人状态 = reinterpret_cast<SDK::AGPPlayerState*>(GLocalCharacter->PlayerState_);


	static bool 追踪初始 = false;
	if (追踪开关 && !追踪初始)
	{
		HookInitialize();
		追踪初始 = !追踪初始;
	}


	static SDK::UGameplayStatics* UGStatics = nullptr;
	if (UGStatics == nullptr) UGStatics = (SDK::UGameplayStatics*)SDK::UGameplayStatics::StaticClass();
	static SDK::UClass* 玩家类 = nullptr;
	if (玩家类 == nullptr) 玩家类 = SDK::AGPCharacterBase::StaticClass();
	static SDK::UClass* 物资类 = nullptr;
	if (物资类 == nullptr) 物资类 = SDK::AInteractorBase::StaticClass();

	预警人数 = 0;
	SDK::AGPCharacterBase* J_Actors_player = nullptr;
	float Center; MaxRange = 瞄准范围;
	if (漏打开关 || 平滑自瞄)QueueCircle(screenCenter, MaxRange, ImColor(255, 255, 255, 200), 1.5, 24);
	if (true)
	{
		SDK::TArray<SDK::AActor*> 玩家数组;
		if (Isvalidptr(UGStatics))
		{
			UGStatics->GetAllActorsOfClass(GWorld, 玩家类, &玩家数组);
			if (玩家数组.IsValid())
			{
				int 本人队伍;
				if (战场模式)
				{
					auto 队伍指针 = *(uint32_t**)((uint8_t*)GLocalCharacter + 3840);
					if (Isvalidptr(队伍指针))
					{
						本人队伍 = *((uint8_t*)队伍指针 + 272);
					}
				}
				else
				{
					本人队伍 = 本人状态->GetTeamID();
				}
				

				const int 玩家数量 = 玩家数组.Num();
				for (int i = 0; i < 玩家数量; i++) {
					if (!玩家数组.IsValidIndex(i)) continue;

					auto 玩家 = reinterpret_cast<SDK::AGPCharacterBase*>(玩家数组[i]);
					if (!Isvalidptr(玩家)) continue;
					if (玩家 == GLocalCharacter) continue;
					if (!Isvalidptr(玩家->Mesh)) continue;

					bool 是否玩家 = true;
					std::string Name = 玩家->GetName();
					if (Name.find("AI") != std::string::npos || Name.find("Range") != std::string::npos)是否玩家 = false;
					if (是否玩家 && !Isvalidptr(玩家->PlayerState_)) continue;
					if (!是否玩家 && !人机开关 || !是否玩家 && 战斗模式)continue;
					SDK::FVector POS = 玩家->K2_GetActorLocation();
					int Dis = POS.GetDistanceTo(GLocalPlayerPos) / 100;
					if (是否玩家 && Dis > 玩家距离)continue;
					if (!是否玩家 && Dis > 人机距离)continue;
					std::string DistanceString = std::to_string(Dis) + "M";

					SDK::AGPPlayerState* 玩家状态;
					uint64_t weaponID = 0 ;
					int 玩家队伍 = 0; char 干员ID; int headArmorLevel = 0, armorLevel = 0; float headDurability = 0.f, armorDurability = 0.f;

					if (是否玩家)
					{
						玩家状态 = reinterpret_cast<SDK::AGPPlayerState*>(玩家->PlayerState_);
						if (Isvalidptr(玩家状态))
						{
							if (战场模式)
							{
								auto 队伍指针 = *(uint32_t**)((uint8_t*)玩家 + 3840);//更新
								if (Isvalidptr(队伍指针))
								{
									玩家队伍 = *((uint8_t*)队伍指针 + 272);//更新
									if (玩家队伍 == 本人队伍)
										continue;
								}
							}
							else
							{

								玩家队伍 = 玩家状态->GetTeamID();
								if (玩家队伍 == 本人队伍)
									continue;
							}

							if (周围预警 && Dis < 预警距离)预警人数++;

							auto 撤离 = *(uint64_t*)((uint8_t*)玩家状态 + 0x4D8);
							if (撤离 == 1)continue;

							uint64_t weaponPtr = *(uint64_t*)((uint8_t*)玩家 + 0x1558);
							if (Isvalidptr(reinterpret_cast<void*>(weaponPtr)))
							{
								weaponID = *(uint64_t*)(weaponPtr + 0x828);
							}

							干员ID = *(char*)((uint8_t*)玩家状态 + 0x9B8);

							// 读取护甲相关
							uintptr_t CharacterEquipComponent_Offset = 0x1F58;
							uintptr_t EquipedArmorInfoArray_Offset = 0x1E0;

							uintptr_t HeadLevel_Offset = 0x30;
							uintptr_t ArmorLevel_Offset = 0xF0;

							uintptr_t HeadDurability_Offset = 0x48;
							uintptr_t ArmorDurability_Offset = 0x108;

							uint8_t* characterEquipComponent = nullptr;
							uint8_t* equipedArmorInfoArray = nullptr;


							uint8_t* playerBase = reinterpret_cast<uint8_t*>(玩家);

							if (Isvalidptr(playerBase + CharacterEquipComponent_Offset))
								characterEquipComponent = *reinterpret_cast<uint8_t**>(playerBase + CharacterEquipComponent_Offset); 

								if (Isvalidptr(characterEquipComponent))
								{
									if (Isvalidptr(characterEquipComponent + EquipedArmorInfoArray_Offset))
										equipedArmorInfoArray = *reinterpret_cast<uint8_t**>(characterEquipComponent + EquipedArmorInfoArray_Offset);
								}

							if (Isvalidptr(equipedArmorInfoArray))
							{
								// 读等级，1字节
								uint8_t headLevelRaw = *(equipedArmorInfoArray + HeadLevel_Offset);
								uint8_t armorLevelRaw = *(equipedArmorInfoArray + ArmorLevel_Offset);

								headArmorLevel = GetArmorLevel(headLevelRaw);
								armorLevel = GetArmorLevel(armorLevelRaw);

								// 读当前耐久（float）
								headDurability = *reinterpret_cast<float*>(equipedArmorInfoArray + HeadDurability_Offset);
								armorDurability = *reinterpret_cast<float*>(equipedArmorInfoArray + ArmorDurability_Offset);
							}
						}

					}


					int 玩家血量 = (int)玩家->GetHealth();
					if (玩家血量 <= 0) continue;
					float 玩家最大血量 = 玩家->GetHealthMax();
					if (玩家最大血量 > 500 || 玩家最大血量 <= 0) continue;

					auto Root = GetBonePos(玩家->Mesh, L"root");
					auto Head = GetBonePos(玩家->Mesh, L"head");

					Head.Z += 25.f;
					SDK::FVector2D ScreenRoot, ScreenLeftHand, ScreenRightHand, ScreenHead;
					if (WorldToScreen(Root, ScreenRoot) && WorldToScreen(Head, ScreenHead))
					{
						float height = ScreenHead.Y - ScreenRoot.Y;
						float width = height / 2;
						Vec2 ScreenTopLeft = { ScreenRoot.X - width / 2, ScreenRoot.Y };
						Vec2 ScreenBottomRight = { ScreenRoot.X + width / 2, ScreenRoot.Y + height };
						Vec2 ScreenBottomCenter = { ScreenTopLeft.x + width / 2, ScreenTopLeft.y };

						auto 是否可视 = LineTraceSingle(玩家数组, GetBonePos(GLocalCharacter->Mesh, L"head"), Head);
						ImColor 颜色 = 是否玩家 ? 是否可视 ? 玩家可见颜色 : 玩家不可见颜色 : 是否可视 ? 人机可见颜色 : 人机不可见颜色;

						if (玩家最大血量 != 500)
							if (骨骼开关)
							{
								DrawSkeleton(玩家, 颜色);
							}

						int Y = 0;

						if (名字开关)
						{
							std::string 玩家名称;
							if(干员开关)
								玩家名称 = 是否玩家 ? GetOperatorName(干员ID) + " : " + 玩家->PlayerState_->GetPlayerName().ToString() : U8("人机_Bot");
							else
								玩家名称 = 是否玩家 ? 玩家->PlayerState_->GetPlayerName().ToString() : U8("人机_Bot");

							
							if (玩家最大血量 == 500) {
								玩家名称 = U8("AI_Bot");
								颜色 = BOSS颜色;
							}
							QueueStrokeText(玩家名称, { ScreenBottomCenter.x,ScreenBottomCenter.y }, 玩家名字颜色, 16, true);
						}
						Y += 20;



						if (是否玩家)
						{
							if(手持开关)
							{
								QueueStrokeText(GetWeaponName(weaponID), { ScreenBottomCenter.x,ScreenBottomCenter.y + Y }, 颜色, 14, true);
								Y += 16;
							}
							if (头甲开关)
							{
								QueueStrokeText(U8("头") + to_string(headArmorLevel) + " : " + to_string((int)headDurability), { ScreenBottomCenter.x,ScreenBottomCenter.y + Y }, GetArmorColor(headArmorLevel), 14, true);
								Y += 16;
								QueueStrokeText(U8("甲") + to_string(armorLevel) + " : " + to_string((int)armorDurability), { ScreenBottomCenter.x,ScreenBottomCenter.y + Y }, GetArmorColor(armorLevel), 14, true);
								Y += 20;
							}
							
						}
						
						if (距离开关)
						{
							QueueText(DistanceString, { ScreenBottomCenter.x,ScreenBottomCenter.y + Y }, ImColor(255, 255, 255, 255), 15, true);
						}

						if (是否玩家 && 方框开关)
						{

							const float lineLength = height * 0.2f; // 每个角的边长
							const float thickness = 1.5f;           // 黄线粗细
							const float outline = 1.0f;             // 黑边粗细

							ImColor innerColor = 颜色; // 黄线
							ImColor outlineColor = ImColor(0, 0, 0, 255);   // 黑边

							auto DrawCornerLines = [&](float x, float y, bool right, bool bottom)
								{
									float dirX = right ? -1.0f : 1.0f;
									float dirY = bottom ? -1.0f : 1.0f;

									Vec2 start(x, y);
									Vec2 endH = { x + dirX * lineLength, y };
									Vec2 endV = { x, y + dirY * lineLength };
									QueueLine(start, endH, outlineColor, thickness + outline * 1.5);
									QueueLine(start, endV, outlineColor, thickness + outline * 1.5);

									// 主体黄线
									QueueLine(start, endH, innerColor, thickness); // 横向黄
									QueueLine(start, endV, innerColor, thickness); // 纵向黄
								};

							DrawCornerLines(ScreenTopLeft.x, ScreenTopLeft.y, false, false);                        // 左上
							DrawCornerLines(ScreenTopLeft.x + width, ScreenTopLeft.y, true, false);                // 右上
							DrawCornerLines(ScreenTopLeft.x, ScreenTopLeft.y + height, false, true);               // 左下
							DrawCornerLines(ScreenTopLeft.x + width, ScreenTopLeft.y + height, true, true);        // 右下
						}



						auto MaxHealth = 玩家最大血量;
						float HealthPercentage = 玩家血量 / MaxHealth;
						ImColor HealthColor;
						if (HealthPercentage > 0.7f)
						{
							HealthColor = ImColor(0, 255, 0, 255);
						}
						else if (HealthPercentage > 0.3f)
						{
							HealthColor = ImColor(255, 215, 0, 255);
						}
						else
						{
							HealthColor = ImColor(255, 0, 0, 255);
						}
						Vec2 HPTextPos = { ScreenBottomCenter.x, ScreenBottomCenter.y + height - 3 - 8 - 16 };
						if (是否玩家 && 队标开关)
						{
							int 圆半径 = 8;
							ImColor TeamColor = GetColorForTeam(玩家队伍);
							Vec2 CircleCenter = { HPTextPos.x + 3, HPTextPos.y - 15 };

							QueueFillCircle(CircleCenter, (float)圆半径, TeamColor);
							QueueText(std::to_string(玩家队伍), { CircleCenter.x,CircleCenter.y - 8 }, ImColor(255, 255, 255), 14.f, true);

						}
						if (人机血条 && !是否玩家 && !战斗模式 || 血条开关 && 是否玩家)
						{
							//auto HP文本 = "HP : " + std::to_string((int)玩家血量);
							//QueueStrokeText(HP文本, HPTextPos, HealthColor, 16, true);
							QueueFilledRect({ ScreenBottomCenter.x - 50, ScreenBottomCenter.y + height - 3 - 8 }, { 100, 8 }, ImColor(110, 110, 110, 255));
							QueueFilledRect({ ScreenBottomCenter.x - 50 , ScreenBottomCenter.y + height - 3 - 8 }, { 100 * HealthPercentage, 8 }, HealthColor);
							QueueRect({ ScreenBottomCenter.x - 50 - 1 , ScreenBottomCenter.y - 1 + height - 3 - 8 }, { 100 + 2, 8 + 1 }, ImColor(0, 0, 0, 255), 2);
							for (int i = 1; i < 10; ++i)
							{
								float x = ScreenBottomCenter.x - 50 + i * 10;
								float y = ScreenBottomCenter.y + height - 3 - 8;
								float lineHeight = 8.0f * (1.0f / 3.0f);

								QueueLine({ x, y }, { x, y + lineHeight }, ImColor(0, 0, 0, 255), 1);
							}
						}


						Center = FindNearestCoordinate({ ScreenHead.X,ScreenHead.Y }, { screenCenter.x, screenCenter.y });
						if (MaxRange > Center && 是否可视) {
							MaxRange = Center;
							J_Actors_player = 玩家;
						}

					}


				}
			}
		}
	}
	
	if (周围预警 && 预警人数 > 0)
	{
		QueueStrokeText(U8("请注意 您附近 [  ") + to_string(预警距离)+ U8("米 ] 内有敌人，敌人数量:”") + to_string(预警人数), { screenCenter.x,200 }, ImColor(255, 255, 0, 255), 20, true);
	}

	if (Isvalidptr(J_Actors_player))
	{
		_player = J_Actors_player;
		if (Isvalidptr(J_Actors_player->Mesh))
		{
			SDK::FVector POS_ = GetBonePos(J_Actors_player->Mesh, GetBoneNameW(selected_part)); SDK::FVector2D returnPos;
			if (WorldToScreen(POS_, returnPos))
			{
				if (漏打开关 || 平滑自瞄) {
					QueueLine_({ screenCenter.x,screenCenter.y }, { returnPos.X, returnPos.Y }, ImColor(255, 0, 0, 255), 1);
					
				}

				if (GetAsyncKeyState(GetKeyCodeFromSelection(selected_key)))
				{
					if (平滑自瞄)
					{
						AimBot(returnPos);
					}
					if (漏打开关)
					{
						auto aim = GetAimRotation(POS_);
						GPlayerController->ControlRotation.Pitch = aim.Pitch;
						GPlayerController->ControlRotation.Yaw = aim.Yaw;
					}

				}

			}
		}
	}

	SDK::TArray<SDK::AActor*> 物资数组;
	if (Isvalidptr(UGStatics))
	{
		UGStatics->GetAllActorsOfClass(GWorld, 物资类, &物资数组);
		if (物资数组.IsValid())
		{
			容器叠加数组.clear();
			物品叠加数组.clear();
			std::vector<SDK::FVector> 已绘制保险箱位置;

			for (int i = 0; i < 物资数组.Num(); i++) {
				if (i >= 物资数组.Num()) break;
				if (!物资数组.IsValidIndex(i)) continue;

				auto 物资 = reinterpret_cast<SDK::AInteractorBase*>(物资数组[i]);
				if (!Isvalidptr(物资)) continue;
				std::string Name = 物资->GetName();
				SDK::FVector POS = 物资->K2_GetActorLocation();
				int Dis = POS.GetDistanceTo(GLocalPlayerPos) / 100;
				std::string DistanceString = std::to_string(Dis) + "M";


				SDK::FVector2D ScreenPOS;
				if (WorldToScreen(POS, ScreenPOS))
				{
					if (绘制容器 && !战斗模式)
					{
						if (Dis > 容器距离)continue;
						if (Name.find("BP_Interactor") != std::string::npos || Name.find("BP_Interact") != std::string::npos) {

							auto 容器 = reinterpret_cast<SDK::AInventoryPickup*>(物资);
							if (Name.find("BP_Interact_Computer_C") != std::string::npos && 绘制密码)
							{
								int 电脑密码 = *(uint32_t*)((uint8_t*)容器 + 0xD44);
								QueueStrokeText(U8("密码") + "|"+ to_string(电脑密码) + "|" + DistanceString, { ScreenPOS.X, ScreenPOS.Y}, ImColor(255, 9, 255, 255), 16, true);
							}
							else if (Name.find("CodedLock") != std::string::npos && 绘制密码)
							{
								int 门密码 = *(uint32_t*)((uint8_t*)容器 + 0xDA0);
								QueueStrokeText(U8("密码门") + "|" + to_string(门密码) + "|" + DistanceString, { ScreenPOS.X, ScreenPOS.Y }, ImColor(255, 9, 255, 255), 16, true);
							}
							else
							{
								auto ExtractContainerName = [](const std::string& fullName) -> std::string {
									size_t last = fullName.rfind('_');
									if (last == std::string::npos) return "";
									size_t second = fullName.rfind('_', last - 1);
									if (second == std::string::npos) return "";
									size_t third = fullName.rfind('_', second - 1);
									if (third == std::string::npos) return "";
									return fullName.substr(third + 1, second - third - 1);
									};

								std::string label = ExtractContainerName(Name);
								
								if (Name.find("Container_SafeBox") != std::string::npos)label = U8("保险箱");
								if (!IsContainer(label)) continue;

								if (label == U8("保险箱")) {
									bool 太近了 = false;
									for (const auto& 已绘制位置 : 已绘制保险箱位置)
									{
										if ((已绘制位置 - POS).Magnitude() < 500.f)
										{
											太近了 = true;
											break;
										}
									}
									if (太近了) continue; 
									已绘制保险箱位置.push_back(POS);
								}
								
								if (过滤打开) {  // 只有当过滤打开为true时才执行检查
									if (Isvalidptr((void*)容器) && Isvalidptr((void*)(容器 + 0x1C52))) {
										uint8_t 是否打开 = *(uint8_t*)((uint8_t*)容器 + 0x1C52);
										if (是否打开 == 1) {
											continue;  // 如果容器已打开，跳过处理
										}
									}
								}

								if (!label.empty()) {
									Vec2 坐标(ScreenPOS.X, ScreenPOS.Y);

									if (叠加坐标(坐标, 容器叠加数组)) {
										容器叠加数组.push_back(坐标);
									}
									else if (不在叠加数组中(坐标, 容器叠加数组)) {
										容器叠加数组.push_back(坐标);
									}
									QueueText(label + "|" + DistanceString, { 坐标.x, 坐标.y }, ImColor(0, 255, 255, 255), 16, true);
								}



								
							}
							
							continue;
						}
					}

					if (Name.find("BP_InventoryPickup_") != std::string::npos && 物品开关 && !战斗模式) {
						auto 物品 = reinterpret_cast<SDK::AInventoryPickup*>(物资);
						
						if (Dis > 物品距离)continue;
						if (Isvalidptr(物品->ItemComp->name->name2->name3)) {
							auto 物品名称 = 物品->ItemComp->name->name2->name3->item_name.ToString();
							auto 物品价值 = 物品->ItemComp->ItemValue;
							auto 物品颜色 = 物品->ItemComp->cor;
							if (物品价值 <= 价值过滤) continue;
							{
								if (物品价值 <= 0) continue;
								auto 颜色 = ImColor(255, 255, 255, 255);
								switch (物品颜色)
								{
								case 1:
									颜色 = ImColor(255, 255, 255, 255); // 白色
									break;
								case 2:
									颜色 = ImColor(0, 255, 0, 255);     // 绿色
									break;
								case 3:
									颜色 = ImColor(0, 0, 255, 255);     // 蓝色
									break;
								case 4:
									颜色 = ImColor(128, 0, 128, 255);   // 紫色
									break;
								case 5:
									颜色 = ImColor(255, 255, 0, 255);    // 黄色
									break;
								case 6:
									颜色 = ImColor(255, 0, 0, 255);      // 红色
									break;
								default:
									颜色 = ImColor(255, 255, 255, 255); // 默认白色
									break;
								}
								std::string 完整文本 = 物品名称 + " $" + std::to_string(物品价值) + "|" + DistanceString;

								Vec2 坐标(ScreenPOS.X, ScreenPOS.Y);

								if (叠加坐标(坐标, 物品叠加数组)) {
									物品叠加数组.push_back(坐标);
								}
								else if (不在叠加数组中(坐标, 物品叠加数组)) {
									物品叠加数组.push_back(坐标);
								}

								QueueText(完整文本, { 坐标.x, 坐标.y }, 颜色, 16, true);
							}
							
							

							
						}
						continue;
					}


					if (Name.find("Inventory_DeadBody") != std::string::npos && !战斗模式)
					{
						if (Dis > 骨灰距离) continue;
						if (Isvalidptr((void*)物资) && Isvalidptr((void*)((uint8_t*)物资 + 0x1DE0)))
						{
							uint8_t boxType = *(uint8_t*)((uint8_t*)物资 + 0x1DE0);
							bool 是否玩家 = (boxType == 0) ? false : true;
							if(是否玩家 && !人物盒子) continue;
							if (!是否玩家 && !人机盒子) continue;
							auto 颜色 = (boxType == 0) ? 人机盒子颜色 : 玩家盒子颜色;
							std::string boxName = (boxType == 0) ? U8("[AI盒子]") : U8("[玩家盒]");
							int Y = 0;
							QueueStrokeText(boxName + "|" + DistanceString, { ScreenPOS.X, ScreenPOS.Y }, 颜色, 16, true);
							if (盒子透视)
							{
								Y += 16;
								uint64_t initem = *(uint64_t*)((uint8_t*)物资 + 0x18D0);
								int initemcount = *(int*)((uint8_t*)物资 + 0x18D0 + 0x8);

								if (initem && initemcount < 30)
								{
									for (int i = 0; i < initemcount; ++i)
									{
										uint64_t boxItem = *(uint64_t*)((uint8_t*)initem + 0x538 + i * 0x5B0);
										auto 盒子物品 = reinterpret_cast<SDK::ItemComponent*>(boxItem);
										if (!Isvalidptr(盒子物品)) continue;

										if (Isvalidptr(盒子物品->name->name2->name3))
										{
											auto 物品名称 = 盒子物品->name->name2->name3->item_name.ToString();
											auto 物品价值 = 盒子物品->ItemValue;
											auto 物品颜色 = 盒子物品->cor;
											if (物品价值 <= 价值过滤) continue;
											if (物品价值 <= 0) continue;

											auto 颜色 = ImColor(255, 255, 255, 255);
											switch (物品颜色)
											{
											case 1:
												颜色 = ImColor(255, 255, 255, 255); // 白色
												break;
											case 2:
												颜色 = ImColor(0, 255, 0, 255);     // 绿色
												break;
											case 3:
												颜色 = ImColor(0, 0, 255, 255);     // 蓝色
												break;
											case 4:
												颜色 = ImColor(128, 0, 128, 255);   // 紫色
												break;
											case 5:
												颜色 = ImColor(255, 255, 0, 255);    // 黄色
												break;
											case 6:
												颜色 = ImColor(255, 0, 0, 255);      // 红色
												break;
											default:
												颜色 = ImColor(255, 255, 255, 255); // 默认白色
												break;
											}

											// 1. 创建坐标变量并处理叠加逻辑
											Vec2 当前坐标(ScreenPOS.X, ScreenPOS.Y);
											当前坐标.y += 16;
											// 2. 处理坐标叠加
											if (叠加坐标(当前坐标, 物品叠加数组)) {
												物品叠加数组.push_back(当前坐标);
											}
											// 3. 构建文本并绘制
											std::string 显示文本 = 物品名称 + " $" + std::to_string(物品价值) + "|" + DistanceString;
											QueueText(显示文本, { 当前坐标.x, 当前坐标.y }, 颜色, 16, true);




										}
									}
								}
							}

							
							continue;
						}
					}

				}

			}
		}
	}





}










//=========================================================================================================================//

void SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// 样式设置
	style.Alpha = 0.6f;               // 全局透明度（可改为0.8实现整体透明）
	style.WindowPadding = ImVec2(12.0f, 12.0f);
	style.WindowRounding = 11.5f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 3.4f);
	style.FrameRounding = 11.9f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(4.3f, 5.5f);
	style.ItemInnerSpacing = ImVec2(7.1f, 1.8f);
	style.CellPadding = ImVec2(12.1f, 9.2f);
	style.IndentSpacing = 0.0f;
	style.ColumnsMinSpacing = 4.9f;
	style.ScrollbarSize = 11.6f;
	style.ScrollbarRounding = 15.9f;
	style.GrabMinSize = 3.7f;
	style.GrabRounding = 20.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	// 半透明颜色设置（alpha值在0.7-0.9之间）
// 文本颜色配置
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);          // 主文本颜色（纯白不透明）
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.27f, 0.32f, 0.45f, 0.8f); // 禁用状态文本（蓝灰色带透明度）

	// 窗口背景相关
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.96f, 0.96f, 0.96f, 0.9f);    // 弹出窗口背景（深灰半透明）
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.10f, 0.12f, 0.8f);    // 子窗口背景（稍浅的深灰）
	style.Colors[ImGuiCol_Border] = ImVec4(0.16f, 0.17f, 0.19f, 0.8f);     // 窗口边框颜色
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.08f, 0.09f, 0.10f, 0.6f); // 边框阴影（更透明）

	// 输入控件状态颜色
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.13f, 0.15f, 0.8f);    // 输入框/复选框默认背景
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.17f, 0.19f, 0.8f); // 悬停状态
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.17f, 0.19f, 0.9f); // 激活状态（透明度降低）

	// 标题栏颜色组
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.97f, 0.86f, 0.8f);   // 默认标题栏（最深的灰色）
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.05f, 0.07f, 0.9f); // 活动窗口标题栏
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.09f, 0.10f, 0.8f); // 折叠窗口标题栏

	// 菜单栏和滚动条
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.11f, 0.12f, 0.8f);  // 菜单栏背景
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.07f, 0.7f); // 滚动条背景
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.13f, 0.15f, 0.8f); // 滚动条滑块
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.17f, 0.19f, 0.9f); // 滑块悬停
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.12f, 0.13f, 0.15f, 1.0f); // 滑块激活

	// 交互元素颜色
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.97f, 1.0f, 0.50f, 1.0f);   // 复选框勾选标记（亮黄色）
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.97f, 1.0f, 0.50f, 1.0f);  // 滑块默认状态
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.80f, 0.50f, 1.0f); // 滑块激活状态（橙黄色）

	// 按钮状态颜色
	style.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.13f, 0.15f, 0.8f);     // 默认按钮
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.19f, 0.20f, 0.9f); // 悬停按钮（亮度提高）
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f); // 按下按钮（不透明度最大）

	// 高级组件颜色
	style.Colors[ImGuiCol_Header] = ImVec4(0.14f, 0.16f, 0.21f, 0.8f);     // 列表表头（带蓝色调）
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.11f, 0.11f, 0.11f, 0.9f); // 表头悬停
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.08f, 0.09f, 0.10f, 1.0f); // 表头激活

	// 分隔线状态
	style.Colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.15f, 0.19f, 0.8f);  // 默认分隔线
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.16f, 0.18f, 0.25f, 0.9f); // 悬停分隔线
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.16f, 0.18f, 0.25f, 1.0f); // 激活分隔线

	// 调整大小手柄
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.15f, 0.15f, 0.7f); // 默认调整手柄
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.97f, 1.0f, 0.50f, 0.9f); // 悬停手柄
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 激活手柄

	// 选项卡颜色
	style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.09f, 0.10f, 0.8f);        // 非活动选项卡
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.13f, 0.15f, 0.9f); // 悬停选项卡
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.13f, 0.15f, 1.0f);  // 活动选项卡
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.09f, 0.10f, 0.8f); // 非聚焦选项卡
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.27f, 0.57f, 0.9f); // 非聚焦但活动选项卡

	// 图表颜色
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.52f, 0.60f, 0.70f, 1.0f);  // 折线图线条（青灰色）
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.04f, 0.98f, 0.98f, 1.0f); // 悬停折线
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.88f, 0.79f, 0.56f, 1.0f); // 直方图（沙黄色）
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.96f, 0.96f, 0.96f, 1.0f); // 悬停直方图

	// 表格颜色
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.05f, 0.05f, 0.07f, 0.8f); // 表头背景
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.05f, 0.05f, 0.07f, 1.0f); // 内边框（强）
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 0.7f); // 外边框（弱）
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.12f, 0.13f, 0.15f, 0.8f); // 行背景
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.10f, 0.11f, 0.12f, 0.8f); // 交替行背景

	// 特殊效果颜色
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.8f); // 文本选中背景
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.50f, 0.51f, 1.0f, 0.9f); // 拖放目标指示（紫色）
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.27f, 0.29f, 1.0f, 1.0f); // 键盘导航高亮（亮蓝色）
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.50f, 0.51f, 1.0f, 1.0f); // 多窗口导航高亮
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.18f, 0.55f, 0.50f); // 多窗口背景暗化
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.18f, 0.55f, 0.50f); // 模态窗口暗化背景

}


void RenderQueuedDrawCommands_() {
	for (const auto& cmd : g_DrawCommands) {
		switch (cmd.type) {
		case DrawType::Line:
			ImGui::GetBackgroundDrawList()->AddLine(cmd.pos1.ToImVec2(), cmd.pos2.ToImVec2(), cmd.color, cmd.thickness);
			break;
		case DrawType::Line_:
			ImGui::GetBackgroundDrawList()->AddLine(cmd.pos1.ToImVec2(), cmd.pos2.ToImVec2(), ImColor(0, 0, 0, 255), cmd.thickness + 1);
			ImGui::GetBackgroundDrawList()->AddLine(cmd.pos1.ToImVec2(), cmd.pos2.ToImVec2(), cmd.color, cmd.thickness);
			break;
		case DrawType::FilledRect:
			ImGui::GetBackgroundDrawList()->AddRectFilled(
				cmd.pos1.ToImVec2(),
				ImVec2(cmd.pos1.x + cmd.pos2.x, cmd.pos1.y + cmd.pos2.y),
				cmd.color);
			break;
		case DrawType::Text: {
			float x = cmd.pos1.x;
			if (cmd.center) {
				float width = ImGui::GetFont()->CalcTextSizeA(cmd.fontSize, FLT_MAX, 0.f, cmd.text.c_str()).x;
				x -= width / 2.f;
			}
			ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), cmd.fontSize, ImVec2(x, cmd.pos1.y), cmd.color, cmd.text.c_str());
			break;
		}
		case DrawType::StrokeText: {
			StrokeText(cmd.text, cmd.pos1, cmd.color, cmd.fontSize, cmd.center);
			break;
		}
		case DrawType::FillCircle:
			ImGui::GetBackgroundDrawList()->AddCircleFilled(
				cmd.pos1.ToImVec2(),
				cmd.pos2.x,
				cmd.color,
				(int)cmd.pos2.y
			);
			break;
		case DrawType::Rect:
			ImGui::GetBackgroundDrawList()->AddRect(
				cmd.pos1.ToImVec2(),
				ImVec2(cmd.pos1.x + cmd.pos2.x, cmd.pos1.y + cmd.pos2.y),
				cmd.color,
				0.f,
				0,
				cmd.thickness);
			break;
		case DrawType::Circle:
			ImGui::GetBackgroundDrawList()->AddCircle(
				cmd.pos1.ToImVec2(),
				cmd.pos2.x,
				ImColor(0, 0, 0, 255),
				(int)cmd.pos2.y,
				cmd.thickness + 1
			);
			ImGui::GetBackgroundDrawList()->AddCircle(
				cmd.pos1.ToImVec2(),
				cmd.pos2.x,
				cmd.color,
				(int)cmd.pos2.y,
				cmd.thickness
			);
			break;
		}
	}
	g_DrawCommands.clear();
}


int GetDllReadMode()
{
	// 写死的路径，如果你不打算改就直接用：
	const char* configPath = "C:\\config.ini";

	// 读取整型值，失败返回默认值 0
	return GetPrivateProfileIntA("Settings", "DllReadMode", 0, configPath);
}

HRESULT APIENTRY hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
	if (!ImGui_Initialised) {
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&DirectX12Interface::Device))) {
			if (!DirectX12Interface::CommandQueue)
			{
				DXGI_SWAP_CHAIN_DESC desc{};
				if (SUCCEEDED(pSwapChain->GetDesc(&desc)))
				{
					UINT targetWidth = desc.BufferDesc.Width;
					UINT targetHeight = desc.BufferDesc.Height;

					std::uintptr_t base = reinterpret_cast<std::uintptr_t>(pSwapChain);
					const int scanRange = 0x300;

					for (int offset = 0; offset < scanRange; offset += 8)
					{
						void* maybeQueue = *reinterpret_cast<void**>(base + offset);
						if (!maybeQueue)
							continue;

						uint32_t width = *reinterpret_cast<uint32_t*>(base + offset + 8);  // +8 是 width
						uint32_t height = *reinterpret_cast<uint32_t*>(base + offset + 12); // +12 是 height

						if (width == targetWidth && height == targetHeight)
						{
							DirectX12Interface::CommandQueue = reinterpret_cast<ID3D12CommandQueue*>(maybeQueue);

							//wchar_t buffer[256];
							//swprintf_s(buffer, L"找到 CommandQueue！偏移: 0x%X\n地址: 0x%p\n分辨率: %ux%u", offset, maybeQueue, width, height);
							//MessageBoxW(NULL, buffer, L"CommandQueue", MB_OK | MB_ICONINFORMATION);
							break;
						}
					}
				}
			}
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO(); (void)io;
			ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImFont* chinese_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyhbd.ttc", 20.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
			io.FontDefault = chinese_font;
			ImFontConfig font_config;
			font_config.PixelSnapH = false;
			font_config.OversampleH = 5;
			font_config.OversampleV = 5;
			font_config.RasterizerMultiply = 1.2f;

			static const ImWchar ranges[] =
			{
				0x0020, 0x00FF, // Basic Latin + Latin Supplement
				0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
				0x2DE0, 0x2DFF, // Cyrillic Extended-A
				0xA640, 0xA69F, // Cyrillic Extended-B
				0xE000, 0xE226, // icons
				0,
			};

			font_config.GlyphRanges = ranges;
			tab_icons = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 15.0f, &font_config, ranges);

			DXGI_SWAP_CHAIN_DESC Desc;
			pSwapChain->GetDesc(&Desc);
			Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			Desc.OutputWindow = Process::Hwnd;
			Desc.Windowed = ((GetWindowLongPtr(Process::Hwnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

			DirectX12Interface::BuffersCounts = Desc.BufferCount;
			DirectX12Interface::FrameContext = new DirectX12Interface::_FrameContext[DirectX12Interface::BuffersCounts];

			D3D12_DESCRIPTOR_HEAP_DESC DescriptorImGuiRender = {};
			DescriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			DescriptorImGuiRender.NumDescriptors = DirectX12Interface::BuffersCounts;
			DescriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			if (DirectX12Interface::Device->CreateDescriptorHeap(&DescriptorImGuiRender, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapImGuiRender)) != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			ID3D12CommandAllocator* Allocator;
			if (DirectX12Interface::Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)) != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
				DirectX12Interface::FrameContext[i].CommandAllocator = Allocator;
			}

			if (DirectX12Interface::Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator, NULL, IID_PPV_ARGS(&DirectX12Interface::CommandList)) != S_OK ||
				DirectX12Interface::CommandList->Close() != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			D3D12_DESCRIPTOR_HEAP_DESC DescriptorBackBuffers;
			DescriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			DescriptorBackBuffers.NumDescriptors = DirectX12Interface::BuffersCounts;
			DescriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			DescriptorBackBuffers.NodeMask = 1;

			if (DirectX12Interface::Device->CreateDescriptorHeap(&DescriptorBackBuffers, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapBackBuffers)) != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			const auto RTVDescriptorSize = DirectX12Interface::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = DirectX12Interface::DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

			for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
				ID3D12Resource* pBackBuffer = nullptr;
				DirectX12Interface::FrameContext[i].DescriptorHandle = RTVHandle;
				pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
				DirectX12Interface::Device->CreateRenderTargetView(pBackBuffer, nullptr, RTVHandle);
				DirectX12Interface::FrameContext[i].Resource = pBackBuffer;
				RTVHandle.ptr += RTVDescriptorSize;
			}

			SetupImGuiStyle();
			ImGui_ImplWin32_Init(Process::Hwnd);
			ImGui_ImplDX12_Init(DirectX12Interface::Device, DirectX12Interface::BuffersCounts, DXGI_FORMAT_R8G8B8A8_UNORM, DirectX12Interface::DescriptorHeapImGuiRender, DirectX12Interface::DescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(), DirectX12Interface::DescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
			ImGui_ImplDX12_CreateDeviceObjects();
			ImGui::GetIO().ImeWindowHandle = Process::Hwnd;
			Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
		}
		ImGui_Initialised = true;
	}

	if (DirectX12Interface::CommandQueue == nullptr)
		return oPresent(pSwapChain, SyncInterval, Flags);


	if (GetAsyncKeyState(VK_OEM_3) & 1) 战斗模式 = !战斗模式;

	if (GetAsyncKeyState(VK_INSERT) & 1) ShowMenu = !ShowMenu;
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	auto IO = ImGui::GetIO();
	IO.MouseDrawCursor = ShowMenu;
	ImVec2 screenSize = IO.DisplaySize;
	screenCenter = ImVec2(screenSize.x * 0.5f, screenSize.y * 0.5f);
	if (ShowMenu == true) {
		ShowMyMenu2();
	}
	std::string fpsText = "FPS : " + std::to_string((int)ImGui::GetIO().Framerate);
	StrokeText(fpsText, { 0, 0 }, ImColor(255, 255, 255, 255), 25, false);
	RenderQueuedDrawCommands_();
	ImGui::EndFrame();


	DirectX12Interface::_FrameContext& CurrentFrameContext = DirectX12Interface::FrameContext[pSwapChain->GetCurrentBackBufferIndex()];
	CurrentFrameContext.CommandAllocator->Reset();


	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = CurrentFrameContext.Resource;
	Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	DirectX12Interface::CommandList->Reset(CurrentFrameContext.CommandAllocator, nullptr);
	DirectX12Interface::CommandList->ResourceBarrier(1, &Barrier);
	DirectX12Interface::CommandList->OMSetRenderTargets(1, &CurrentFrameContext.DescriptorHandle, FALSE, nullptr);
	DirectX12Interface::CommandList->SetDescriptorHeaps(1, &DirectX12Interface::DescriptorHeapImGuiRender);

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DirectX12Interface::CommandList);
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	DirectX12Interface::CommandList->ResourceBarrier(1, &Barrier);
	DirectX12Interface::CommandList->Close();
	DirectX12Interface::CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&DirectX12Interface::CommandList));
	return oPresent(pSwapChain, SyncInterval, Flags);
}

//=========================================================================================================================//

void hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists)
{

	if (!DirectX12Interface::CommandQueue && queue)
	{
		DirectX12Interface::CommandQueue = queue;
	}

	oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}


DWORD WINAPI MainThread(LPVOID lpParameter) {

	bool WindowFocus = false;
	while (WindowFocus == false) {

		Process::ID = GetCurrentProcessId();
		Process::Handle = GetCurrentProcess();
		Process::Hwnd = FindWindow("UnrealWindow", NULL);
		if (Process::Hwnd)
		{
			RECT TempRect;
			GetWindowRect(Process::Hwnd, &TempRect);
			Process::WindowWidth = TempRect.right - TempRect.left;
			Process::WindowHeight = TempRect.bottom - TempRect.top;

			char TempClassName[MAX_PATH];
			GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
			Process::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
			Process::Path = TempPath;

			WindowFocus = true;
		}
		else
		{
			Sleep(100);
		}
	}
	bool InitHook = false;
	while (InitHook == false) {
		if (DirectX12::Init() == true) {
			//CreateHook(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists);
			CreateHook(140, (void**)&oPresent, hkPresent);
			InitHook = true;
		}
	}
	return 0;
}

using fn_post_render = void(__thiscall*)(SDK::UObject*, SDK::UObject*);
fn_post_render OriginalPostRender;

BYTE* vmt_hook(void** VFTable, uint32_t index, void* TargetFunction)
{
	BYTE* org = reinterpret_cast<BYTE*>(VFTable[index]);

	DWORD protect = 0;

	VirtualProtect(&VFTable[index], 8, PAGE_EXECUTE_READWRITE, &protect);

	VFTable[index] = TargetFunction;

	VirtualProtect(&VFTable[index], 8, protect, 0);

	return org;
}

void post_render_hook(SDK::UObject* Viewport, SDK::UCanvas* Canvas)
{
	GWorld = SDK::UWorld::GetWorld();

	if (!Isvalidptr(GWorld))return;
	if (!Isvalidptr(GWorld->OwningGameInstance))return;

	draw();
	
}

void init() {
	GWorld = SDK::UWorld::GetWorld();

	auto LocalPlayer = GWorld->OwningGameInstance->LocalPlayers[0];
	auto ViewportClient = LocalPlayer->ViewportClient;
	void** VFTable = ViewportClient->VTable;
	OriginalPostRender = reinterpret_cast<decltype(OriginalPostRender)>(vmt_hook(VFTable, 0x5F, &post_render_hook));
}




//=========================================================================================================================//

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		Process::Module = hModule;
		GetModuleFileNameA(hModule, dlldir, 512);
		for (size_t i = strlen(dlldir); i > 0; i--) { if (dlldir[i] == '\\') { dlldir[i + 1] = 0; break; } }
		
		CreateThread(0, 0, LPTHREAD_START_ROUTINE(MainThread), 0, 0, 0);
		CreateThread(0, 0, LPTHREAD_START_ROUTINE(init), 0, 0, 0);

		break;
	case DLL_PROCESS_DETACH:
		FreeLibraryAndExitThread(hModule, TRUE);
		DisableAll();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
	}
	return TRUE;
}

//=========================================================================================================================//

//D3D12 Methods Table:
//[0]   QueryInterface
//[1]   AddRef
//[2]   Release
//[3]   GetPrivateData
//[4]   SetPrivateData
//[5]   SetPrivateDataInterface
//[6]   SetName
//[7]   GetNodeCount
//[8]   CreateCommandQueue
//[9]   CreateCommandAllocator
//[10]  CreateGraphicsPipelineState
//[11]  CreateComputePipelineState
//[12]  CreateCommandList
//[13]  CheckFeatureSupport
//[14]  CreateDescriptorHeap
//[15]  GetDescriptorHandleIncrementSize
//[16]  CreateRootSignature
//[17]  CreateConstantBufferView
//[18]  CreateShaderResourceView
//[19]  CreateUnorderedAccessView
//[20]  CreateRenderTargetView
//[21]  CreateDepthStencilView
//[22]  CreateSampler
//[23]  CopyDescriptors
//[24]  CopyDescriptorsSimple
//[25]  GetResourceAllocationInfo
//[26]  GetCustomHeapProperties
//[27]  CreateCommittedResource
//[28]  CreateHeap
//[29]  CreatePlacedResource
//[30]  CreateReservedResource
//[31]  CreateSharedHandle
//[32]  OpenSharedHandle
//[33]  OpenSharedHandleByName
//[34]  MakeResident
//[35]  Evict
//[36]  CreateFence
//[37]  GetDeviceRemovedReason
//[38]  GetCopyableFootprints
//[39]  CreateQueryHeap
//[40]  SetStablePowerState
//[41]  CreateCommandSignature
//[42]  GetResourceTiling
//[43]  GetAdapterLuid
//[44]  QueryInterface
//[45]  AddRef
//[46]  Release
//[47]  GetPrivateData
//[48]  SetPrivateData
//[49]  SetPrivateDataInterface
//[50]  SetName
//[51]  GetDevice
//[52]  UpdateTileMappings
//[53]  CopyTileMappings
//[54]  ExecuteCommandLists
//[55]  SetMarker
//[56]  BeginEvent
//[57]  EndEvent
//[58]  Signal
//[59]  Wait
//[60]  GetTimestampFrequency
//[61]  GetClockCalibration
//[62]  GetDesc
//[63]  QueryInterface
//[64]  AddRef
//[65]  Release
//[66]  GetPrivateData
//[67]  SetPrivateData
//[68]  SetPrivateDataInterface
//[69]  SetName
//[70]  GetDevice
//[71]  Reset
//[72]  QueryInterface
//[73]  AddRef
//[74]  Release
//[75]  GetPrivateData
//[76]  SetPrivateData
//[77]  SetPrivateDataInterface
//[78]  SetName
//[79]  GetDevice
//[80]  GetType
//[81]  Close
//[82]  Reset
//[83]  ClearState
//[84]  DrawInstanced
//[85]  DrawIndexedInstanced
//[86]  Dispatch
//[87]  CopyBufferRegion
//[88]  CopyTextureRegion
//[89]  CopyResource
//[90]  CopyTiles
//[91]  ResolveSubresource
//[92]  IASetPrimitiveTopology
//[93]  RSSetViewports
//[94]  RSSetScissorRects
//[95]  OMSetBlendFactor
//[96]  OMSetStencilRef
//[97]  SetPipelineState
//[98]  ResourceBarrier
//[99]  ExecuteBundle
//[100] SetDescriptorHeaps
//[101] SetComputeRootSignature
//[102] SetGraphicsRootSignature
//[103] SetComputeRootDescriptorTable
//[104] SetGraphicsRootDescriptorTable
//[105] SetComputeRoot32BitConstant
//[106] SetGraphicsRoot32BitConstant
//[107] SetComputeRoot32BitConstants
//[108] SetGraphicsRoot32BitConstants
//[109] SetComputeRootConstantBufferView
//[110] SetGraphicsRootConstantBufferView
//[111] SetComputeRootShaderResourceView
//[112] SetGraphicsRootShaderResourceView
//[113] SetComputeRootUnorderedAccessView
//[114] SetGraphicsRootUnorderedAccessView
//[115] IASetIndexBuffer
//[116] IASetVertexBuffers
//[117] SOSetTargets
//[118] OMSetRenderTargets
//[119] ClearDepthStencilView
//[120] ClearRenderTargetView
//[121] ClearUnorderedAccessViewUint
//[122] ClearUnorderedAccessViewFloat
//[123] DiscardResource
//[124] BeginQuery
//[125] EndQuery
//[126] ResolveQueryData
//[127] SetPredication
//[128] SetMarker
//[129] BeginEvent
//[130] EndEvent
//[131] ExecuteIndirect
//[132] QueryInterface
//[133] AddRef
//[134] Release
//[135] SetPrivateData
//[136] SetPrivateDataInterface
//[137] GetPrivateData
//[138] GetParent
//[139] GetDevice
//[140] Present
//[141] GetBuffer
//[142] SetFullscreenState
//[143] GetFullscreenState
//[144] GetDesc
//[145] ResizeBuffers
//[146] ResizeTarget
//[147] GetContainingOutput
//[148] GetFrameStatistics
//[149] GetLastPresentCount