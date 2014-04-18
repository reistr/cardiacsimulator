#include "ColorShaderClass.h"


ColorShaderClass::ColorShaderClass()
{
	m_effect = 0;
	m_technique = 0;
	m_layout = 0;

	m_worldMatrixPtr = 0;
	m_viewMatrixPtr = 0;
	m_projectionMatrixPtr = 0;
}


ColorShaderClass::~ColorShaderClass()
{
}

bool ColorShaderClass::Initialize(ID3D10Device* device, HWND hwnd)
{
	bool result;

	// Initialize the shader
	result = InitializeShader(device, hwnd, L"../Engine/color.fx");
	if(!result)
	{
		return false;
	}

	return true;
}

void ColorShaderClass::Shutdown()
{
	// Shutdown the shader effect.
	ShutdownShader();

	return;
}

void ColorShaderClass::Render(ID3D10Device* device, int indexCount,
							  D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
							  D3DXMATRIX projectionMatrix)
{
	// Set render parameters
	SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix);

	// Render the prepared buffers with shader
	RenderShader(device, indexCount);
	
	return;
}

bool ColorShaderClass::InitializeShader(ID3D10Device* device, HWND hwnd, WCHAR* filename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	D3D10_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D10_PASS_DESC passDesc;

	// Initialize errorMessage
	errorMessage = 0;

	// Load the shader in from the file.
	result = D3DX10CreateEffectFromFile(filename, NULL, NULL, "fx_4_0", 
										D3D10_SHADER_ENABLE_STRICTNESS,
										0, device, NULL, NULL, &m_effect,
										&errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should initialize errorMessage
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, filename);
		}
		// Otherwise the shader file was not found.
		else
		{
			MessageBox(hwnd, filename, L"Missing shader file", MB_OK);
		}

		return false;
	}
	m_technique = m_effect->GetTechniqueByName("ColorTechnique");
	if(!m_technique)
	{
		return false;
	}

	// Setup the layout of the data that goes into shader
	// matching the VertexType structure in ModelClass and shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Get the description of the first pass described in the shader technique.
	m_technique -> GetPassByIndex(0)->GetDesc(&passDesc);

	// Create the input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, passDesc.pIAInputSignature,
									   passDesc.IAInputSignatureSize, &m_layout);

	// Get pointers to the three matrices inside the shader so we can update them from this class.
	m_worldMatrixPtr = m_effect->GetVariableByName("worldMatrix")->AsMatrix();
	m_viewMatrixPtr = m_effect->GetVariableByName("viewMatrix")->AsMatrix();
	m_projectionMatrixPtr = m_effect->GetVariableByName("projectionMatrix")->AsMatrix();

	return true;
}

void ColorShaderClass::ShutdownShader()
{
	// Release the pointers to the matrixes
	m_worldMatrixPtr = 0;
	m_viewMatrixPtr = 0;
	m_projectionMatrixPtr = 0;

	// Release the pointer to the shader layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pointer to the shader technique
	m_technique = 0;

	// Release the pointer to the shader.
	if(m_effect)
	{
		m_effect->Release();
		m_effect = 0;
	}

	return;
}

void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, 
												WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write an error message to
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", 
			shaderFilename, MB_OK);

	return;
}

void ColorShaderClass::SetShaderParameters(D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
										   D3DXMATRIX projectionMatrix)
{
	// Set matrices inside the shader
	m_worldMatrixPtr->SetMatrix((float*)&worldMatrix);
	m_viewMatrixPtr->SetMatrix((float*)&viewMatrix);
	m_projectionMatrixPtr->SetMatrix((float*)&projectionMatrix);

	return;
}

void ColorShaderClass::RenderShader(ID3D10Device* device, int indexCount)
{
	D3D10_TECHNIQUE_DESC techniqueDesc;
	unsigned int i;

	// Set the input layout.
	device->IASetInputLayout(m_layout);

	// Get the description structure of the technique from 
	// inside the shader so it can be used for rendering.
	m_technique->GetDesc(&techniqueDesc);

	// Go through each pass in the technique and render
	for (i=0; i<techniqueDesc.Passes; i++)
	{
		m_technique->GetPassByIndex(i)->Apply(0);
		device->DrawIndexed(indexCount,0,0);
	}

	return;
}
