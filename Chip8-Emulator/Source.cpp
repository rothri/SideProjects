#include <d3d11.h>
#include "cstdlib"
#include <d3dcompiler.h>
#include "cstdio"
#include "chip8.h"
#include <d3dcsx.h>
#include <windows.h>
#include <DirectXColors.h>
#include <iostream>
#include <fstream>  
#include <stdlib.h>
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "d3d11.lib")
using namespace DirectX;

HWND windowHandle;
IDXGISwapChain* swapChain;
ID3D11Device *device;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *deviceContext;           // the pointer to our Direct3D device context
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
HRESULT InitGraphics();
void setGraphics();
ID3D11RenderTargetView* backbuffer;
ID3D11Buffer* gfxbuffer; 
ID3D11VertexShader* vertexshader;
ID3D11PixelShader* pixelshader;
ID3D11InputLayout* inputlayout;
int vertexCount =0;
bool drawFlag =false;
struct VERTEX
{
	float X, Y, Z;    // vertex position
};
VERTEX gfx[64*32*3] ;

chip8 myChip8;
void CleanupDevice();
void Render(float intensity);
void keyPress(int key, bool down);
LRESULT CALLBACK    WindowProc( HWND, UINT, WPARAM, LPARAM );

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );
	MSG msg;
	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
		return 0;

	if( FAILED( InitDevice() ) )
	{
		CleanupDevice();
		return 0;
	}
	if(FAILED( InitGraphics() ) )
	{
		CleanupDevice();
		return 0;
	}
	// this struct holds Windows event messages
	myChip8.initialize();
	//switch to command line upon release
	myChip8.loadApplication("C:/Users/William/Downloads/c8Games/PONG");
	// wait for the next message in the queue, store the result in 'msg'
	while(TRUE)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			//send message to WindowProc function
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT){
				break;
			}

		}
		else{
			myChip8.emulateCycle();
			Sleep(2);
			TranslateMessage(&msg);
			drawFlag=myChip8.getDrawFlag();
			if (drawFlag){
				setGraphics();
				Render(1.0f);
			}
		}
	}


	// return this part of the WM_QUIT message to Windows
	return msg.wParam;
}
void CleanupDevice()
{
	swapChain->Release();
	device->Release();
	deviceContext->Release();
	backbuffer->Release();
	gfxbuffer->Release();
}
HRESULT InitGraphics(){
	HRESULT hr= S_OK;

	D3D11_BUFFER_DESC bd = {0};
	bd.ByteWidth = sizeof(VERTEX) * ARRAYSIZE(gfx);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd = {gfx, 0, 0};

	device->CreateBuffer(&bd, &srd, &gfxbuffer);
	ID3DBlob* blobShader = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr= D3DCompileFromFile(L"VertexShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_4_0",
		D3DCOMPILE_DEBUG|D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&blobShader,&errorBlob);
	if( errorBlob )
	{
		std::cout<<( reinterpret_cast<const char*>( errorBlob->GetBufferPointer() ) );
		errorBlob->Release();
		return hr;
	}

	// create the shader objects
	hr = device->CreateVertexShader( blobShader->GetBufferPointer(), blobShader->GetBufferSize(), nullptr, &vertexshader );
	if( FAILED( hr ) )
	{	
		blobShader->Release();
		return hr;
	}
	ID3DBlob* blobPixel = nullptr;
	hr= D3DCompileFromFile(L"PixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_4_0",
		D3DCOMPILE_DEBUG|D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&blobPixel,&errorBlob);
	if( errorBlob )
	{
		std::cout<< reinterpret_cast<const char*>( errorBlob->GetBufferPointer() ) ;
		errorBlob->Release();
		return hr;
	}

	device->CreatePixelShader(blobPixel->GetBufferPointer(), blobPixel->GetBufferSize(), nullptr, &pixelshader);

	deviceContext->VSSetShader(vertexshader,nullptr,0);
	deviceContext->PSSetShader(pixelshader,nullptr,0);

	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	device->CreateInputLayout(ied, ARRAYSIZE(ied), blobShader->GetBufferPointer(), blobShader->GetBufferSize(), &inputlayout);
	deviceContext->IASetInputLayout(inputlayout);
	return hr;
}
void Render(float intensity)
{
	// Just clear the backbuffer
	float color[4] ={0.0f, 0.0f, 0.0f, intensity};
	deviceContext->ClearRenderTargetView(backbuffer,color);
	// set the vertex buffer
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	deviceContext->UpdateSubresource(gfxbuffer,0,NULL,gfx,0,0);
	deviceContext->IASetVertexBuffers(0, 1, &gfxbuffer, &stride, &offset);
	// set the primitive topology
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw 3 vertices, starting from vertex 0
	deviceContext->Draw(vertexCount, 0);
	swapChain->Present(0, 0);
}
HRESULT InitDevice(){
	HRESULT hr = S_OK;
	RECT rc;
	GetClientRect( windowHandle, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	UINT createDeviceFlags =0;
	DXGI_SWAP_CHAIN_DESC swapStruct;//strcture for swapchain
	ZeroMemory( &swapStruct, sizeof(swapStruct) );
	swapStruct.BufferCount =1;									   // One back buffer
	swapStruct.OutputWindow = windowHandle;						   //The windle handle
	swapStruct.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	swapStruct.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	swapStruct.SampleDesc.Count = 4;                               // how many multisamples
	swapStruct.Windowed = TRUE;// windowed/full-screen mode


	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&swapStruct,
		&swapChain,
		&device,
		NULL,
		&deviceContext);

	ID3D11Texture2D *texBackBuffer;//COM object
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texBackBuffer);
	//	(texture,structure of renderTarget,address of object
	device->CreateRenderTargetView(texBackBuffer, NULL, &backbuffer);
	texBackBuffer->Release();

	deviceContext->OMSetRenderTargets(1, &backbuffer,NULL);

	//setting Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	deviceContext->RSSetViewports(1, &viewport);
	return hr;
}
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow){
	WNDCLASSEX  windowsClass;
	ZeroMemory(&windowsClass,sizeof(windowsClass));
	windowsClass.cbSize =sizeof(windowsClass);
	windowsClass.style = CS_HREDRAW | CS_VREDRAW;// look into this
	windowsClass.lpfnWndProc = WindowProc;
	windowsClass.cbClsExtra = 0;
	windowsClass.cbWndExtra = 0;
	windowsClass.hInstance = hInstance;
	windowsClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	windowsClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	windowsClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowsClass.lpszMenuName = "Menu";
	windowsClass.lpszClassName = "WClass";
	windowsClass.hIconSm- NULL;
	RegisterClassEx(&windowsClass);
	RECT rectangle = {0,0,640,320};
	AdjustWindowRect(&rectangle,WS_OVERLAPPEDWINDOW, false );
	windowHandle= CreateWindow("WClass","Chip8 Emulator",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rectangle.right -rectangle.left,rectangle.bottom-rectangle.top,(HWND) NULL,(HMENU) NULL,hInstance,(LPVOID) NULL);
	if( !windowHandle){
		return E_FAIL;
	}
	ShowWindow(windowHandle,nCmdShow);
	return S_OK;
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// sort through and find what code to run for the message given
	switch(message)
	{
		// this message is read when the window is closed
	case WM_DESTROY:
		{
			// close the application entirely
			PostQuitMessage(0);
			CleanupDevice();
			return 0;
		} 
		break;
	case WM_KEYDOWN:
		{
			keyPress(wParam,true);
			break;
		}
	case WM_KEYUP:
		{
			keyPress(wParam,false);
			break;
		}

		// Handle any messages the switch statement didn't
	default:
		return DefWindowProc (hWnd, message, wParam, lParam);
	}
}
void setGraphics(){
	vertexCount=0;
	VERTEX toAdd ={0};
	for (int i=64*32-1;i>=0;i--){
		if (myChip8.getGfx(i) ==1){//always draw clockwise
			toAdd.X=(i%64)/32.0f -1;
			toAdd.Y =1.0-(i/64)/16.0f;
			toAdd.X+= 1/64.0f;
			toAdd.Y+= 1/32.0f;
			gfx[vertexCount]=toAdd;
			vertexCount++;
			toAdd.Y-=1/16.0f;
			gfx[vertexCount]=toAdd;
			vertexCount++;
			toAdd.X-=1/32.0f;
			gfx[vertexCount]=toAdd;
			vertexCount++;
			gfx[vertexCount]=toAdd;
			vertexCount++;
			toAdd.Y+=1/16.0f;
			gfx[vertexCount]=toAdd;
			vertexCount++;
			toAdd.X+=1/32.0f;
			gfx[vertexCount]=toAdd;
			vertexCount++;
		}

	}
	toAdd.X=0;
	toAdd.Y=0;
	for(int i=vertexCount*3;i<32*64*3;i++){
		gfx[i]=toAdd;
	}
}
void keyPress(int key, bool down){
	//123c<->1234
	//456d<->qwer
	//789e<->asdf
	//a0bf<->zxcv
	int value;
	if (down == true){
		value=1;
	}
	else{
		value =0;
	}
	switch(key)
	{
	case 49:
		{
			myChip8.key[0x1]=value;
			break;
		}
	case 50:
		{
			myChip8.key[0x2]=value;
			break;
		}
	case 51:
		{
			myChip8.key[0x3]=value;
			break;
		}
	case 52:
		{
			myChip8.key[0xC]=value;
			break;
		}
	case 81:
		{
			myChip8.key[0x4]=value;
			break;
		}
	case 87:
		{
			myChip8.key[0x5]=value;
			break;
		}
	case 69:
		{
			myChip8.key[0x6]=value;
			break;
		}
	case 82:
		{
			myChip8.key[0xD]=value;
			break;
		}
	case 65:
		{
			myChip8.key[0x7]=value;
			break;
		}
	case 83:
		{
			myChip8.key[0x8]=value;
			break;
		}
	case 68:
		{
			myChip8.key[0x9]=value;
			break;
		}
	case 70:
		{
			myChip8.key[0xE]=value;
			break;
		}
	case 90:
		{
			myChip8.key[0xA]=value;
			break;
		}
	case 88:
		{
			myChip8.key[0x0]=value;
			break;
		}
	case 67:
		{
			myChip8.key[0xB]=value;
			break;
		}
	case 86:
		{
			myChip8.key[0xF]=value;
			break;
		}
	}
}
	

