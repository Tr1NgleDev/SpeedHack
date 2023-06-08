//#define DEBUG_CONSOLE // Uncomment this if you want a debug console

// Mod Name. Make sure it matches the mod folder's name
#define MOD_NAME "Speed-Hack"
#define MOD_VER "1.0"

#include <Windows.h>
#include <cstdio>
#include <4dm.h>
using namespace fdm;
using namespace fdm::gui;

float speed = 1.0;
bool g = false;

FontRenderer* speedText;

void(__thiscall* Player_update)(Player* self, GLFWwindow* window, World* world, double dt);
void __fastcall Player_update_H(Player* self, GLFWwindow* window, World* world, double dt) 
{
	Player_update(self, window, world, dt * speed);
}

bool(__thiscall* Player_keyInput)(Player* self, GLFWwindow* window, int key, int scancode, int action, int mods);
bool __fastcall Player_keyInput_H(Player* self, GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	if (key == GLFW_KEY_G) 
	{
		if (action == GLFW_PRESS) g = true;
		else if (action == GLFW_RELEASE) g = false;
	}
	
	return Player_keyInput(self, window, key, scancode, action, mods);
}

void(__thiscall* GameState_scrollInput)(GameState* self, StateManager& s, double xOff, double yOff);
void __fastcall GameState_scrollInput_H(GameState* self, StateManager& s, double xOff, double yOff)
{
	GameState_scrollInput(self, s, xOff, yOff);
	if (g)
	{
		self->player.angleToRotate -= yOff * self->player.scrollFactor * 0.0099999998f;
		speed += yOff * 0.05f;
		speed = glm::clamp(speed, 0.05f, 10.0f);
	}
}

void(__thiscall* GameState_mouseButtonInput)(GameState* self, StateManager& s, int button, int action, int mods);
void __fastcall GameState_mouseButtonInput_H(GameState* self, StateManager& s, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS && g) speed = 1.0;
	GameState_mouseButtonInput(self, s, button, action, mods);
}
std::stringstream str;
void(__thiscall* Player_renderHud)(Player* self, GLFWwindow* window);
void __fastcall Player_renderHud_H(Player* self, GLFWwindow* window)
{
	Player_renderHud(self, window);
	str.str(std::string());
	str << "Speed:" << std::setprecision(3) << speed;
	speedText->setText(str.str());
	speedText->updateModel();

	glDepthMask(0);
	speedText->render();
	glDepthMask(1);
}
const Shader* fontShader;
const Tex2D* fontTex;

void(__thiscall* GameState_init)(GameState* self, StateManager& s);
void __fastcall GameState_init_H(GameState* self, StateManager& s)
{
	glfwInit();

	GameState_init(self, s);

	int width, height;
	glfwGetWindowSize(s.window, &width, &height);

	fontTex = ResourceManager::get("pixelfont.png");
	fontShader = ShaderManager::get("textShader");

	speedText = new FontRenderer(fontTex, fontShader);
	speedText->centered = true;
	speedText->color = glm::vec4{ 1.0f };
	speedText->charSize = glm::ivec2{ 8, 8 };
	speedText->fontSize = 2;
	speedText->pos = glm::ivec2{ width / 2.0f, 25 };
}

DWORD WINAPI Main_Thread(void* hModule)
{
	// Create console window if DEBUG_CONSOLE is defined
#ifdef DEBUG_CONSOLE
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
#endif
	Hook(reinterpret_cast<void*>(FUNC_PLAYER_UPDATE), reinterpret_cast<void*>(&Player_update_H), reinterpret_cast<void**>(&Player_update));
	Hook(reinterpret_cast<void*>(FUNC_PLAYER_KEYINPUT), reinterpret_cast<void*>(&Player_keyInput_H), reinterpret_cast<void**>(&Player_keyInput));
	Hook(reinterpret_cast<void*>(FUNC_GAMESTATE_SCROLLINPUT), reinterpret_cast<void*>(&GameState_scrollInput_H), reinterpret_cast<void**>(&GameState_scrollInput));
	Hook(reinterpret_cast<void*>(FUNC_GAMESTATE_MOUSEBUTTONINPUT), reinterpret_cast<void*>(&GameState_mouseButtonInput_H), reinterpret_cast<void**>(&GameState_mouseButtonInput));
	
	Hook(reinterpret_cast<void*>(FUNC_PLAYER_RENDERHUD), reinterpret_cast<void*>(&Player_renderHud_H), reinterpret_cast<void**>(&Player_renderHud));
	Hook(reinterpret_cast<void*>(FUNC_GAMESTATE_INIT), reinterpret_cast<void*>(&GameState_init_H), reinterpret_cast<void**>(&GameState_init));
	
	EnableHook(0);
	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD _reason, LPVOID lpReserved)
{
	if (_reason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main_Thread, hModule, 0, NULL);
	return TRUE;
}
