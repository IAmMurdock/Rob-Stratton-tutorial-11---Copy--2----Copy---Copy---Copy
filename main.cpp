#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include <dinput.h>

#include "camera.h"
#include "text2D.h"
#include "Model.h"

//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pD3DDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pBackBufferRTView = NULL;
Model*					g_pModel = NULL;

//camera
camera* gamecamera = NULL;
Text2D* g_2DText;
ID3D11ShaderResourceView* g_pTexture0;

//lighting
XMVECTOR g_directional_light_shines_from;
XMVECTOR g_directional_light_colour;
XMVECTOR g_ambient_light_colour;

//model
Model* my_model;
Model* model_2;

//input

IDirectInput8* g_direct_input;
IDirectInputDevice8* g_keyboard_device;
unsigned char g_keyboard_keys_state[256];






ID3D11Buffer* g_pVertexBuffer;

ID3D11VertexShader* g_pVertexShader;

ID3D11PixelShader* g_pPixelShader;

ID3D11InputLayout* g_pInputLayout;

ID3D11Buffer* g_pConstantBuffer0;

float degrees = 0;

ID3D11DepthStencilView* g_pZBuffer;

ID3D11SamplerState* g_pSampler0; // Declared with other global variables at start of main()

struct CONSTANT_BUFFER0
{

	XMMATRIX WorldViewProjectionMatrix; //64 bytes
	XMVECTOR directional_light_vector;	// 16 bytes
	XMVECTOR directional_light_colour;	// 16 bytes
	XMVECTOR ambient_light_colour;		// 16 bytes

	//float RedAmmount;
	//float scale;

	XMFLOAT2 packing_bytes;

};

// Define vertex structure

struct POS_COL_TEX_NORM_VERTEX

{
	XMFLOAT3 Pos;

	XMFLOAT4 Col;

	XMFLOAT2 texture0;

	XMFLOAT3 normal;
};

// Rename for each tutorial
char		g_TutorialName[100] = "Delilah";


//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();

void ShutdownD3D();
void RenderFrame(void);
HRESULT InitialiseGraphics(void);
HRESULT DirectInputStart();

void ReadInputStates();
bool IsKeyPressed(unsigned char DI_Keycode);


//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}
	
	// Main message loop
	MSG msg = { 0 };

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		return 0;
	}

	if (FAILED(InitialiseGraphics()))

	{

		DXTRACE_MSG("Failed to initialise graphics");

		return 0;

	}

	if (FAILED(DirectInputStart()))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// do something
			RenderFrame();
			my_model->CalculateModelCenterPoint();
			model_2->CalculateModelCenterPoint();
			//my_model->CalculateBoundingSphereRadius();
		}
	}

	ShutdownD3D();

	return (int)msg.wParam;
}

void ReadInputStates()
{
	HRESULT hr;
	hr = g_keyboard_device->GetDeviceState(sizeof(g_keyboard_keys_state),
		(LPVOID)&g_keyboard_keys_state);
	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			g_keyboard_device->Acquire();
		}
	}
}

bool IsKeyPressed(unsigned char DI_keycode)
{
	return g_keyboard_keys_state[DI_keycode] & 0x80;
}

HRESULT DirectInputStart()
{
	HRESULT hr;
	ZeroMemory(g_keyboard_keys_state, sizeof(g_keyboard_keys_state));

	hr = DirectInput8Create(g_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_direct_input, NULL);
	if (FAILED(hr)) return hr;

	hr = g_direct_input->CreateDevice(GUID_SysKeyboard, &g_keyboard_device, NULL);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->Acquire();
	if (FAILED(hr)) return hr;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app window your own name
	char Name[100] = "Dave Horne\0";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//   wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
		//degrees++;
		gamecamera->rotate(-10.0);
		break;

	case WM_RBUTTONDOWN:
		//degrees++;
		gamecamera->forward(-5.0);
		break;

	/*case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(g_hWnd);
		else if (wParam == VK_UP)
			gamecamera->forward(5.0);
		else if (wParam == VK_DOWN)
			gamecamera->forward(-5.0);
		else if (wParam == VK_LEFT)
			gamecamera->side(5.0);
		else if (wParam == VK_RIGHT)
			gamecamera->side(-5.0);
		else if (wParam == VK_SPACE)
			my_model->MoveForward(5);
		return 0;*/



	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain,
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;
	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBufferTexture);

	if (FAILED(hr)) return hr;

	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
		&g_pBackBufferRTView);
	pBackBufferTexture->Release();

	if (FAILED(hr)) return hr;

	//create a z buffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D *pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);

	if (FAILED(hr)) return hr;

	
	// Create the Z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();




	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_pImmediateContext->RSSetViewports(1, &viewport);


	g_2DText = new Text2D("assets/font1.bmp", g_pD3DDevice, g_pImmediateContext);


	return S_OK;

}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_keyboard_device)
	{
		g_keyboard_device->Unacquire();
		g_keyboard_device->Release();
	}

	if (g_direct_input) g_direct_input->Release();

	gamecamera = NULL;
	delete gamecamera;

	g_2DText = NULL;
	delete g_2DText;

	model_2 = NULL;
	delete model_2;

	my_model = NULL;
	delete my_model; 

	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pInputLayout) g_pInputLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();

	//delete texture
	if (g_pTexture0) g_pTexture0->Release();
	if (g_pSampler0) g_pSampler0->Release();

	if (g_pConstantBuffer0) g_pConstantBuffer0->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pD3DDevice) g_pD3DDevice->Release();
	if (g_pBackBufferRTView) g_pBackBufferRTView->Release();
}


HRESULT InitialiseGraphics()

{
	my_model = new Model(g_pD3DDevice, g_pImmediateContext);
	my_model->LoadObjModel("assets/sphere.obj");
	my_model->setPosition(100, 200, 300);
	model_2 = new Model(g_pD3DDevice, g_pImmediateContext);
	model_2->LoadObjModel("assets/cube.obj");
	model_2->setPosition(3, 2, 1);
	HRESULT hr = S_OK;

	// Define vertices of a triangle - screen coordinates -1.0 to +1.0

	//POS_COL_TEX_NORM_VERTEX vertices[] =

	//{

	//	/*{ XMFLOAT3(0.9f, 0.9f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

	//	{ XMFLOAT3(0.9f, -0.9f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },

	//	{ XMFLOAT3(-0.9f, -0.9f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	//	*/

	//	//back
	//	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	//	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	//	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

	//	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	//	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	//	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

	//	//front
	//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
	//	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
	//	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

	//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
	//	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
	//	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

	//	//left
	//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

	//	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

	//	//right
	//	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

	//	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
	//	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

	//	//bottom

	//	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
	//	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

	//	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

	//	//top
	//	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	//	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

	//	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	//	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	//	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }


	//};

	// Set up and create vertex buffer

	//D3D11_BUFFER_DESC bufferDesc;

	//ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	//bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // Used by CPU and GPU

	//bufferDesc.ByteWidth = sizeof(vertices); // Total size of buffer, 3 vertices

	//bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Use as a vertex buffer

	//bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// Allow CPU access

	//hr = g_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &g_pVertexBuffer);//Create the buffer

	//if (FAILED(hr)) // return error code on failure

	/*{

		return hr;

	}*/

	//set up constant buffer

	//D3D11_BUFFER_DESC constant_buffer_desc;
	//ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));

	//constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	//constant_buffer_desc.ByteWidth = 64;
	//constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	//hr = g_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &g_pConstantBuffer0);



	//// Copy the vertices into the buffer

	//D3D11_MAPPED_SUBRESOURCE ms;

	//g_pImmediateContext->Map(g_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); 
	//// Lock the buffer to allow writing

	//	memcpy(ms.pData, vertices, sizeof(vertices)); // Copy the data

	//g_pImmediateContext->Unmap(g_pVertexBuffer, NULL); // Unlock the buffer

	// Load and compile pixel and vertex shaders - use vs_5_0 to target DX11 hardware only

	//ID3DBlob *VS, *PS, *error;

	//hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error,

	//	0);

	//if (error != 0) // check for shader compilation error

	//{

	//	OutputDebugStringA((char*)error->GetBufferPointer());

	//	error->Release();

	//	if (FAILED(hr)) // don't fail if error is just a warning

	//	{

	//		return hr;

	//	};

	//}

	//hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error,

	//	0);

	//if (error != 0)// check for shader compilation error

	//{

	//	OutputDebugStringA((char*)error->GetBufferPointer());

	//	error->Release();

	//	if (FAILED(hr))// don't fail if error is just a warning

	//	{

	//		return hr;

	//	};

	//}

	//// Create shader objects

	//hr = g_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL,&g_pVertexShader);

	//if (FAILED(hr))

	//{

	//	return hr;

	//}



	//hr = g_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL,

	//	&g_pPixelShader);
	//
	//if (FAILED(hr))

	//{

	//	return hr;

	//}

	//// Set the shader objects as active

	//g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);

	//g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);

	//create camera

	gamecamera = new camera(0.0, 0.0, -0.5, 0);


	// load texture

	//D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice, "assets/crate.bmp", NULL, NULL, &g_pTexture0, NULL);

	//// Create and set the input layout object

	//D3D11_INPUT_ELEMENT_DESC iedesc[] =

	//{

	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}

	//};
	////sampler
	//D3D11_SAMPLER_DESC sampler_desc;
	//ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	//sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	//g_pD3DDevice->CreateSamplerState(&sampler_desc, &g_pSampler0);



	/*hr = g_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(),

		&g_pInputLayout);*/

	/*if (FAILED(hr))

	{

		return hr;

	}*/

	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	return S_OK;

}




void RenderFrame(void)
{
	ReadInputStates();

	if (IsKeyPressed(DIK_ESCAPE)) DestroyWindow(g_hWnd);
	if (IsKeyPressed(DIK_LEFT)) gamecamera->side(0.01);
	if (IsKeyPressed(DIK_RIGHT)) gamecamera->side(-0.01);
	if (IsKeyPressed(DIK_UP)) gamecamera->forward(0.01);
	if (IsKeyPressed(DIK_DOWN)) gamecamera->forward(-0.01);

	//set lighting
	g_directional_light_shines_from = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	g_directional_light_colour = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);
	g_ambient_light_colour = XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f);

	//upload the new values for the constant buffer
	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);

	// Set vertex buffer

	//UINT stride = sizeof(POS_COL_TEX_NORM_VERTEX);

	//UINT offset = 0;

	//g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Select which primtive type to use

	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);

	g_pImmediateContext->ClearDepthStencilView
		(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	//CONSTANT_BUFFER0 cb0_values;
	//cb0_values.RedAmmount = 0.5f;
	//cb0_values.scale = 1.0f;	
	XMMATRIX projection, world, world2, view;

	//lighting
	
	//XMMATRIX transpose;
	//CONSTANT_BUFFER0 cb0_values;

	//transpose = XMMatrixTranspose(world); // model world matrix

	/*cb0_values.directional_light_colour = g_directional_light_colour;
	cb0_values.ambient_light_colour = g_ambient_light_colour;
	cb0_values.directional_light_vector = XMVector3Transform(g_directional_light_shines_from, transpose);
	cb0_values.directional_light_vector = XMVector3Normalize(cb0_values.directional_light_vector);
*/
	

	//world = XMMatrixRotationY(XMConvertToRadians(degrees));
	//world *= XMMatrixTranslation(1, 0, 25);

	//
	//world2 = XMMatrixRotationY(XMConvertToRadians(degrees));
	//world2 *= XMMatrixTranslation(5, 0, 25);
	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), 640.0 / 480.0, 1.0, 100.0);


	//view = gamecamera->getviewmatrix();
	////view = XMMatrixIdentity();

	//cb0_values.WorldViewProjectionMatrix = world * view * projection;


	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);

	//g_pImmediateContext->Draw(36, 0);

	//cb0_values.WorldViewProjectionMatrix = world2 * view * projection;
	//// Draw the vertex buffer to the back buffer

	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);

	//g_pImmediateContext->Draw(36, 0);

	////texture
	//g_pImmediateContext->PSSetSamplers(0, 1, &g_pSampler0);
	//g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture0);

	// RENDER HERE

	my_model->LookAt_XZ(gamecamera->GetX(), gamecamera->GetY());
	my_model->draw(&gamecamera->getviewmatrix(), &projection);
	model_2->LookAt_XZ(gamecamera->GetX(), gamecamera->GetY());
	model_2->draw(&gamecamera->getviewmatrix(), &projection);


	//TEXT


	//g_2DText->AddText("Some Text", -1.0, 1.0, 0.2);
	//g_2DText->RenderText();

	// Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}
