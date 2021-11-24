//Include and link appropriate libraries and headers//

#include"main.h"


//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;
LPDIRECTINPUT8 DirectInput;
IDirectInputDevice8* DIKeyboard;

ID3D11Buffer* squareIndexBuffer;
ID3D11Buffer* squareVertBuffer;

ID3D11Buffer* sphereIndexBuffer;
ID3D11Buffer* sphereVertBuffer;

ID3D11Buffer* planeIndexBuffer;
ID3D11Buffer* planeVertBuffer;

ID3D11DepthStencilView* depthstencilView;
ID3D11Texture2D* depthStencilBuffer;

ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;

ID3D11VertexShader* SKYMAP_VS;
ID3D11PixelShader* SKYMAP_PS;
ID3D10Blob* SKYMAP_VS_Buffer;
ID3D10Blob* SKYMAP_PS_Buffer;


ID3D11VertexShader* REFL_VS;
ID3D11PixelShader* REFL_PS;
ID3D10Blob*			REFL_VS_Buffer;
ID3D10Blob*			REFL_PS_Buffer;

ID3D11ShaderResourceView* smrv;

ID3D11DepthStencilState* DSLessEqual;
ID3D11RasterizerState* RSCullNone;

ID3D11InputLayout* vertLayout;

ID3D11Buffer* cbPerObjectBuffer;
ID3D11Buffer* cbPerFrameBuffer;

ID3D11RasterizerState* WireFrame;

//TextureRes
ID3D11ShaderResourceView* CubeTexture;
ID3D11SamplerState* CubeTexSamplerState;

ID3D11BlendState* Transparency;
ID3D11RasterizerState* CCWcullMode;
ID3D11RasterizerState* CWcullMode;


//Global Declarations - Others//
LPCTSTR WndClassName = L"DXDEMO";
HWND hwnd = NULL;
HRESULT hr;

const int Width = 1600;
const int Height = 900;

XMMATRIX cube1World;
XMMATRIX cube1Inverse;
XMMATRIX cube2World;
XMMATRIX cube2Inverse;
XMMATRIX sphereWorld;
XMMATRIX planeWorld;
XMMATRIX planeInverse;

int NumSphereVertices;
int NumSphereFaces;

XMMATRIX RotationX;
XMMATRIX RotationY;
XMMATRIX RotationZ;
XMMATRIX Scale;
XMMATRIX Translation;

double countsPerSecond = 0.0;
double frameTime;
__int64 frameTimeOld = 0;


XMMATRIX SpRotationx;
XMMATRIX SpRotationy;
XMMATRIX groundWorld;

float rotZ = 0.01f;
float rotY = 0.01f;
float rotX = 0.01f;

XMMATRIX WVP;
XMMATRIX World;
XMMATRIX InverseWorld;
XMMATRIX camView;
XMMATRIX camProjection;

XMVECTOR camPosition;
XMVECTOR camTarget;
XMVECTOR camUp;


//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance);
void CleanUp();
bool InitScene();
void UpdateScene();
void DrawScene();

void DetectInput(double time);
void StartTimer();
double GetFrameTime();
bool InitDirectInput(HINSTANCE hInstance);

void CreateSphere(int LatLines, int LongLines);

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed);
int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);


struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}
	XMFLOAT3 dir;
	float pad;
	XMFLOAT3 pos;
	float range;
	XMFLOAT3 att;
	float pad2;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

struct cbPerObject
{
	XMMATRIX WVP;
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX invMatrix;
	XMVECTOR cameraPos;
};

Light light;

struct cbPerFrame
{
	Light light;
};

cbPerFrame constbuffPerFrame;

struct reflBuffer
{
	XMMATRIX reflMatrix;
};

cbPerObject cbPerObj;

reflBuffer reflBuff;

struct Vertex    //Overloaded Vertex Structure  //Textures
{
	Vertex() {}
	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz)
		: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) {}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
};

D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

UINT numElements = ARRAYSIZE(layout);

int WINAPI WinMain(HINSTANCE hInstance,    //Main windows function
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{

	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!InitializeDirect3d11App(hInstance))    //Initialize Direct3D
	{
		MessageBox(0, L"Direct3D Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!InitScene())    //Initialize our scene
	{
		MessageBox(0, L"Scene Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!InitDirectInput(hInstance))    //Initialize Dinput
	{
		MessageBox(0, L"DirectInput Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}
	StartTimer();

	messageloop();

	CleanUp();

	return 0;
}

bool InitDirectInput(HINSTANCE hInstance)
{
	hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&DirectInput, NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	return true;
}

void DetectInput(double time)
{
	BYTE keyboardState[256];

	DIKeyboard->Acquire();

	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	if (keyboardState[DIK_ESCAPE] & 0x80)
		PostMessage(hwnd, WM_DESTROY, 0, 0);

	if (keyboardState[DIK_E]) rotZ -= 1.0f * time;
	if (keyboardState[DIK_Q]) rotZ += 1.0f * time;
	if (keyboardState[DIK_A]) rotY -= 1.0f * time;
	if (keyboardState[DIK_D]) rotY += 1.0f * time;
	if (keyboardState[DIK_S]) rotX -= 1.0f * time;
	if (keyboardState[DIK_W]) rotX += 1.0f * time;
	return;
}

void StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);
	countsPerSecond = double(frequencyCount.QuadPart);
}

double GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if (tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount) / countsPerSecond;
}

void CreateSphere(int LatLines, int LongLines)
{
	NumSphereVertices = ((LatLines - 2) * LongLines) + 2;
	NumSphereFaces = ((LatLines - 3)*(LongLines) * 2) + (LongLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;

	std::vector<Vertex> vertices(NumSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].pos.x = 0.0f;
	vertices[0].pos.y = 0.0f;
	vertices[0].pos.z = 1.0f;

	for (DWORD i = 0; i < LatLines - 2; ++i)
	{
		spherePitch = (i + 1) * (3.14 / (LatLines - 1));
		SpRotationx = XMMatrixRotationX(spherePitch);
		for (DWORD j = 0; j < LongLines; ++j)
		{
			sphereYaw = j * (6.28 / (LongLines));
			SpRotationy = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (SpRotationx * SpRotationy));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*LongLines + j + 1].pos.x = XMVectorGetX(currVertPos);
			vertices[i*LongLines + j + 1].pos.y = XMVectorGetY(currVertPos);
			vertices[i*LongLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[NumSphereVertices - 1].pos.x = 0.0f;
	vertices[NumSphereVertices - 1].pos.y = 0.0f;
	vertices[NumSphereVertices - 1].pos.z = -1.0f;


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * NumSphereVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &sphereVertBuffer);


	std::vector<DWORD> indices(NumSphereFaces * 3);

	int k = 0;
	for (DWORD l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = LongLines;
	indices[k + 2] = 1;
	k += 3;

	for (DWORD i = 0; i < LatLines - 3; ++i)
	{
		for (DWORD j = 0; j < LongLines - 1; ++j)
		{
			indices[k] = i*LongLines + j + 1;
			indices[k + 1] = i*LongLines + j + 2;
			indices[k + 2] = (i + 1)*LongLines + j + 1;

			indices[k + 3] = (i + 1)*LongLines + j + 1;
			indices[k + 4] = i*LongLines + j + 2;
			indices[k + 5] = (i + 1)*LongLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*LongLines) + LongLines;
		indices[k + 1] = (i*LongLines) + 1;
		indices[k + 2] = ((i + 1)*LongLines) + LongLines;

		indices[k + 3] = ((i + 1)*LongLines) + LongLines;
		indices[k + 4] = (i*LongLines) + 1;
		indices[k + 5] = ((i + 1)*LongLines) + 1;

		k += 6;
	}

	for (DWORD l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = NumSphereVertices - 1;
		indices[k + 1] = (NumSphereVertices - 1) - (l + 1);
		indices[k + 2] = (NumSphereVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = NumSphereVertices - 1;
	indices[k + 1] = (NumSphereVertices - 1) - LongLines;
	indices[k + 2] = NumSphereVertices - 2;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * NumSphereFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &sphereIndexBuffer);

}

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed)
{
	typedef struct _WNDCLASS {
		UINT cbSize;
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		HANDLE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCTSTR lpszMenuName;
		LPCTSTR lpszClassName;
	} WNDCLASS;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(
		NULL,
		WndClassName,
		L"DirectX Object Rendering",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

bool InitializeDirect3d11App(HINSTANCE hInstance)
{
	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = Width;
	bufferDesc.Height = Height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	//Create our SwapChain
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	//Create our BackBuffer
	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

	//Create our Render Target
	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();

	//DepthStencil
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = Width;
	depthStencilDesc.Height = Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthstencilView);

	//Set our Render Target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthstencilView);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

	D3D11_RENDER_TARGET_BLEND_DESC RTBdesc;
	ZeroMemory(&RTBdesc, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));

	RTBdesc.BlendEnable = true;
	RTBdesc.SrcBlend = D3D11_BLEND_SRC_COLOR;
	RTBdesc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	RTBdesc.BlendOp = D3D11_BLEND_OP_ADD;
	RTBdesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	RTBdesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	RTBdesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	RTBdesc.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = RTBdesc;

	d3d11Device->CreateBlendState(&blendDesc, &Transparency);

	return true;
}

void CleanUp()
{
	//Release the COM Objects we created
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	DirectInput->Release();

	DIKeyboard->Release();

	squareVertBuffer->Release();
	squareIndexBuffer->Release();

	planeIndexBuffer->Release();
	planeVertBuffer->Release();

	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();

	REFL_VS->Release();
	REFL_PS->Release();
	REFL_VS_Buffer->Release();
	REFL_PS_Buffer->Release();

	sphereIndexBuffer->Release();
	sphereVertBuffer->Release();

	SKYMAP_VS->Release();
	SKYMAP_PS->Release();
	SKYMAP_VS_Buffer->Release();
	SKYMAP_PS_Buffer->Release();

	smrv->Release();


	depthStencilBuffer->Release();
	depthstencilView->Release();

	cbPerObjectBuffer->Release();
	cbPerFrameBuffer->Release();

	WireFrame->Release();

	CubeTexture->Release();
	CubeTexSamplerState->Release();



	CCWcullMode->Release();
	CWcullMode->Release();
	Transparency->Release();
}

bool InitScene()
{
	CreateSphere(10, 10);
	//Compile Shaders from shader file
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "SKYMAP_VS", "vs_4_0", 0, 0, 0, &SKYMAP_VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "SKYMAP_PS", "ps_4_0", 0, 0, 0, &SKYMAP_PS_Buffer, 0, 0);

	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "RGB_VS", "vs_4_0", 0, 0, 0, &REFL_VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "RGB_PS", "ps_4_0", 0, 0, 0, &REFL_PS_Buffer, 0, 0);


	//Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

	hr = d3d11Device->CreateVertexShader(SKYMAP_VS_Buffer->GetBufferPointer(), SKYMAP_VS_Buffer->GetBufferSize(), NULL, &SKYMAP_VS);
	hr = d3d11Device->CreatePixelShader(SKYMAP_PS_Buffer->GetBufferPointer(), SKYMAP_PS_Buffer->GetBufferSize(), NULL, &SKYMAP_PS);

	hr = d3d11Device->CreateVertexShader(REFL_VS_Buffer->GetBufferPointer(), REFL_VS_Buffer->GetBufferSize(), NULL, &REFL_VS);
	hr = d3d11Device->CreatePixelShader(REFL_PS_Buffer->GetBufferPointer(), REFL_PS_Buffer->GetBufferSize(), NULL, &REFL_PS);

	//Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);


	//Create the vertex buffer
	Vertex v[] =
	{
		//Front
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,0.0f,-1.0f),
		Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f,0.0f,-1.0f),
		Vertex(+1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f,0.0f,-1.0f),
		Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,0.0f,-1.0f),

		//Back
		Vertex(-1.0f, -1.0f, +1.0f, 1.0f, 1.0f, 0.0f,0.0f, 1.0f),
		Vertex(+1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 0.0f,0.0f, 1.0f),
		Vertex(+1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 0.0f,0.0f, 1.0f),
		Vertex(-1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f,0.0f, 1.0f),

		//Top
		Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f,1.0f, 0.0f),
		Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 0.0f,1.0f, 0.0f),
		Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f,1.0f, 0.0f),
		Vertex(+1.0f, +1.0f, -1.0f, 1.0f, 1.0f, 0.0f,1.0f, 0.0f),

		//Bot
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,-1.0f, 0.0f),
		Vertex(+1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,-1.0f, 0.0f),
		Vertex(+1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 0.0f,-1.0f, 0.0f),
		Vertex(-1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f,-1.0f, 0.0f),

		//Left
		Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 1.0f, -1.0f,0.0f, 0.0f),
		Vertex(-1.0f, -1.0f, +1.0f, 0.0f, 0.0f, -1.0f,0.0f, 0.0f),
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, -1.0f,0.0f, 0.0f),
		Vertex(-1.0f, +1.0f, -1.0f, 1.0f, 1.0f, -1.0f,0.0f, 0.0f),

		//Right
		Vertex(+1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,0.0f, 0.0f),
		Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f,0.0f, 0.0f),
		Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 1.0f,0.0f, 0.0f),
		Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 1.0f, 1.0f,0.0f, 0.0f),
	};

	Vertex v2[] = //this is the plane
	{
		Vertex(-1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f,1.0f, 0.0f),
		Vertex(+1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,1.0f, 0.0f),
		Vertex(+1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f,1.0f, 0.0f),
		Vertex(-1.0f, 0.0f, +1.0f, 1.0f, 0.0f, 0.0f,1.0f, 0.0f),
	};

	DWORD indices[] =
	{
		0,1,2,
		0,2,3,

		5,6,4,
		6,7,4,

		8,9,10,
		8,10,11,

		12,13,14,
		12,14,15,

		18,17,16,
		19,18,16,

		20,21,22,
		20,22,23
	};

	DWORD indices2[] =
	{
		0,1,2,
		0,2,3
	};

	//SKYMAP

	D3DX11_IMAGE_LOAD_INFO loadSMInfo;
	loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* SMTexture = 0;
	hr = D3DX11CreateTextureFromFile(d3d11Device, L"skymap.dds",
		&loadSMInfo, 0, (ID3D11Resource**)&SMTexture, 0);

	D3D11_TEXTURE2D_DESC SMTextureDesc;
	SMTexture->GetDesc(&SMTextureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = SMTextureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = d3d11Device->CreateShaderResourceView(SMTexture, &SMViewDesc, &smrv);

	//CUBE

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &squareIndexBuffer);


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 24;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &squareVertBuffer);

	//PLANE

	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;


	iinitData.pSysMem = indices2;
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &planeIndexBuffer);


	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v2;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &planeVertBuffer);


	//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &squareVertBuffer, &stride, &offset);

	//Create the Input Layout
	hr = d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);

	//Set the Input Layout
	d3d11DevCon->IASetInputLayout(vertLayout);

	//Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Set the Viewport
	d3d11DevCon->RSSetViewports(1, &viewport);

	D3D11_BUFFER_DESC cbbd;

	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	//Frame buffer
	cbbd.ByteWidth = sizeof(cbPerFrame);
	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerFrameBuffer);

	//CAMERA
	camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	cbPerObj.cameraPos = camPosition;

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);


	camProjection = XMMatrixPerspectiveFovLH(0.4*3.14f, (float)Width / Height, 1.0f, 1000.0f);

	//Rasterizer
	D3D11_RASTERIZER_DESC WFdesc;
	ZeroMemory(&WFdesc, sizeof(D3D11_RASTERIZER_DESC));
	WFdesc.FillMode = D3D11_FILL_WIREFRAME;
	WFdesc.CullMode = D3D11_CULL_NONE;
	hr = d3d11Device->CreateRasterizerState(&WFdesc, &WireFrame);

	//d3d11DevCon->RSSetState(WireFrame);

	hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, L"wall.jpg", NULL, NULL, &CubeTexture, NULL);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = d3d11Device->CreateSamplerState(&sampDesc, &CubeTexSamplerState);


	D3D11_RASTERIZER_DESC cmDesc;
	ZeroMemory(&cmDesc, sizeof(D3D11_RASTERIZER_DESC));
	cmDesc.FillMode = D3D11_FILL_SOLID;
	cmDesc.CullMode = D3D11_CULL_BACK;

	cmDesc.FrontCounterClockwise = true;
	hr = d3d11Device->CreateRasterizerState(&cmDesc, &CCWcullMode);

	cmDesc.FrontCounterClockwise = false;
	hr = d3d11Device->CreateRasterizerState(&cmDesc, &CWcullMode);

	cmDesc.CullMode = D3D11_CULL_NONE;
	hr = d3d11Device->CreateRasterizerState(&cmDesc, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	d3d11Device->CreateDepthStencilState(&dssDesc, &DSLessEqual);
	return true;
}

void UpdateScene()
{
	//Reset cube1World
	groundWorld = XMMatrixIdentity();

	//Define cube1's world space matrix
	Scale = XMMatrixScaling(500.0f, 10.0f, 500.0f);
	Translation = XMMatrixTranslation(0.0f, 10.0f, 0.0f);

	//Set cube1's world space using the transformations
	groundWorld = Scale * Translation;

	//Reset sphereWorld
	sphereWorld = XMMatrixIdentity();

	//Define sphereWorld's world space matrix
	Scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	//Make sure the sphere is always centered around camera
	Translation = XMMatrixTranslation(XMVectorGetX(camPosition), XMVectorGetY(camPosition), XMVectorGetZ(camPosition));

	//Set sphereWorld's world space using the transformations
	sphereWorld = Scale * Translation;


	cube1World = XMMatrixIdentity();



	XMVECTOR rotAxisX = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	RotationX = XMMatrixRotationAxis(rotAxisX, rotX);
	XMVECTOR rotAxisY = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	RotationY = XMMatrixRotationAxis(rotAxisY, rotY);
	XMVECTOR rotAxisZ = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	RotationZ = XMMatrixRotationAxis(rotAxisZ, rotZ);
	Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

	cube1World = RotationX*RotationY*RotationZ * Translation * RotationY;
	cube1Inverse = XMMatrixInverse(&XMMatrixDeterminant(cube1World), cube1World);

	cube2World = XMMatrixIdentity();
	rotAxisY = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	RotationY = XMMatrixRotationAxis(rotAxisY, -rotY);
	Scale = XMMatrixScaling(1.2f, 1.2f, 1.2f);

	cube2World = Scale * RotationX *RotationY*RotationZ;
	cube2Inverse = XMMatrixInverse(&XMMatrixDeterminant(cube2World), cube2World);


	planeWorld = XMMatrixIdentity();
	Translation = XMMatrixTranslation(0.0f, -4.0f, 2.0f);
	Scale = XMMatrixScaling(4.0f, 4.0f, 4.0f);
	planeWorld = Scale * Translation;
	planeInverse = XMMatrixInverse(&XMMatrixDeterminant(planeWorld), planeWorld);

	XMVECTOR lightVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	lightVector = XMVector3TransformCoord(lightVector, cube1World);
	light.dir.x = -XMVectorGetX(lightVector);
	light.dir.y = -XMVectorGetY(lightVector);
	light.dir.z = -XMVectorGetZ(lightVector);

	constbuffPerFrame.light = light;
	d3d11DevCon->UpdateSubresource(cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0);
	d3d11DevCon->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);

}

void DrawScene()
{
	//Clear our backbuffer
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

	d3d11DevCon->ClearDepthStencilView(depthstencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);




	//Set the grounds vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	d3d11DevCon->IASetIndexBuffer(sphereIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the spheres vertex buffer
	d3d11DevCon->IASetVertexBuffers(0, 1, &sphereVertBuffer, &stride, &offset);

	//Set the WVP matrix and send it to the constant buffer in effect file
	WVP = sphereWorld * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	cbPerObj.worldMatrix = XMMatrixTranspose(sphereWorld);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	//Send our skymap resource view to pixel shader
	d3d11DevCon->PSSetShaderResources(0, 1, &smrv);
	d3d11DevCon->PSSetSamplers(0, 2, &CubeTexSamplerState);

	//Set the new VS and PS shaders
	d3d11DevCon->VSSetShader(SKYMAP_VS, 0, 0);
	d3d11DevCon->PSSetShader(SKYMAP_PS, 0, 0);
	//Set the new depth/stencil and RS states
	d3d11DevCon->OMSetDepthStencilState(DSLessEqual, 0);
	d3d11DevCon->RSSetState(RSCullNone);
	d3d11DevCon->DrawIndexed(NumSphereFaces * 3, 0, 0);

	//Set the default VS shader and depth/stencil state
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->OMSetDepthStencilState(NULL, 0);

	//DRAW PLANE

	d3d11DevCon->VSSetShader(REFL_VS, 0, 0);
	d3d11DevCon->PSSetShader(REFL_PS, 0, 0);

	d3d11DevCon->IASetVertexBuffers(0, 1, &planeVertBuffer, &stride, &offset);
	d3d11DevCon->IASetIndexBuffer(planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	d3d11DevCon->PSSetShaderResources(0, 1, &smrv);
	d3d11DevCon->PSSetSamplers(0, 1, &CubeTexSamplerState);

	WVP = planeWorld*camView*camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	cbPerObj.worldMatrix = XMMatrixTranspose(planeWorld);
	cbPerObj.invMatrix = XMMatrixTranspose(planeInverse);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	d3d11DevCon->RSSetState(CWcullMode);
	d3d11DevCon->DrawIndexed(6, 0, 0);

	d3d11DevCon->RSSetState(CCWcullMode);
	d3d11DevCon->DrawIndexed(6, 0, 0);

	//RENDER OPAQUE

	//Set the grounds index buffer
	d3d11DevCon->IASetVertexBuffers(0, 1, &squareVertBuffer, &stride, &offset);
	d3d11DevCon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


	XMVECTOR cubePos = XMVectorZero();

	cubePos = XMVector3TransformCoord(cubePos, cube1World);

	float distX = XMVectorGetX(cubePos) - XMVectorGetX(camPosition);
	float distY = XMVectorGetY(cubePos) - XMVectorGetY(camPosition);
	float distZ = XMVectorGetZ(cubePos) - XMVectorGetZ(camPosition);

	float cube1Dist = distX*distX + distY*distY + distZ*distZ;

	cubePos = XMVectorZero();
	cubePos = XMVector3TransformCoord(cubePos, cube2World);

	distX = XMVectorGetX(cubePos) - XMVectorGetX(camPosition);
	distY = XMVectorGetY(cubePos) - XMVectorGetY(camPosition);
	distZ = XMVectorGetZ(cubePos) - XMVectorGetZ(camPosition);

	float cube2Dist = distX*distX + distY*distY + distZ*distZ;

	//if cube 1 near to cam
	if (cube1Dist < cube2Dist)
	{
		//switch render order;
		XMMATRIX tmpMatrix = cube1World;
		cube1World = cube2World;
		cube2World = tmpMatrix;
	}

	//DRAW CUBE 1
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	WVP = cube1World*camView*camProjection;
	cbPerObj.invMatrix = cube1Inverse;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	cbPerObj.worldMatrix = cube1World;
	cbPerObj.viewMatrix = camView;
	cbPerObj.projectionMatrix = camProjection;


	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//SetTexture
	d3d11DevCon->PSSetShaderResources(0, 1, &CubeTexture);
	d3d11DevCon->PSSetSamplers(0, 2, &CubeTexSamplerState);

	d3d11DevCon->RSSetState(CCWcullMode);
	d3d11DevCon->DrawIndexed(36, 0, 0);

	d3d11DevCon->RSSetState(CWcullMode);
	d3d11DevCon->DrawIndexed(36, 0, 0);

	//DRAW CUBE 2

	float blendFactor[] = { 0.7f,0.7f ,0.7f ,1.0f };
	d3d11DevCon->OMSetBlendState(0, 0, 0xffffffff);
	d3d11DevCon->OMSetBlendState(Transparency, blendFactor, 0xffffffff);

	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	WVP = cube2World*camView*camProjection;
	cbPerObj.invMatrix = cube2Inverse;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	cbPerObj.worldMatrix = cube2World;
	cbPerObj.viewMatrix = camView;
	cbPerObj.projectionMatrix = camProjection;

	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	//SetTexture
	d3d11DevCon->PSSetShaderResources(0, 1, &CubeTexture);
	d3d11DevCon->PSSetSamplers(0, 2, &CubeTexSamplerState);

	d3d11DevCon->RSSetState(CCWcullMode);
	d3d11DevCon->DrawIndexed(36, 0, 0);

	d3d11DevCon->RSSetState(CWcullMode);
	d3d11DevCon->DrawIndexed(36, 0, 0);

	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 1.0f;
	d3d11DevCon->OMSetBlendState(0, 0, 0xffffffff);
	d3d11DevCon->OMSetBlendState(Transparency, blendFactor, 0xffffffff);



	//Present the backbuffer to the screen
	SwapChain->Present(0, 0);
}

int messageloop() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		BOOL PeekMessageL(
			LPMSG lpMsg,
			HWND hWnd,
			UINT wMsgFilterMin,
			UINT wMsgFilterMax,
			UINT wRemoveMsg
		);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// run game code     
			frameTime = GetFrameTime();
			DetectInput(frameTime);
			UpdateScene();
			DrawScene();
		}
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}