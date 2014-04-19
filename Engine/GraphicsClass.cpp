#include "GraphicsClass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = NULL;
	m_Camera = NULL;
	m_Model = NULL;
	m_ColorShader = NULL;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}

GraphicsClass::~GraphicsClass()
{
}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}
	result = m_D3D -> Initialize(screenWidth, screenHeight, VSYNC_ENABLED,hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create the camera
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial camera position
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

	// Create the model
	m_Model = new ModelClass;
	if(!m_Model)
	{
		return false;
	}

	// Initialize the model
	result = m_Model->Initialize(m_D3D->GetDevice());
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object", L"Error", MB_OK);
		return false;
	}

	// Create the color shader object.
	m_ColorShader = new ColorShaderClass;
	if(!m_ColorShader)
	{
		return false;
	}
	
	// Initialize the color shader object.
	result = m_ColorShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object", L"Error", MB_OK);
		return false;
	}

	return true;
}

void GraphicsClass::Shutdown()
{
	if(m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = NULL;
	}

	if(m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = NULL;
	}

	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = NULL;
	}

	if (m_D3D)
	{
		m_D3D -> Shutdown();
		delete m_D3D;
		m_D3D = NULL;
	}

	return;
}


bool GraphicsClass::Frame()
{
	bool result;
	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render()
{
	D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix;
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);

	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	m_Model->Render(m_D3D->GetDevice());

	m_ColorShader->Render(m_D3D->GetDevice(), m_Model->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix);

	m_D3D->EndScene();

	return true;
}