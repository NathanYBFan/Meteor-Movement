// Button art reference: http://opengameart.org/content/ui-pack (public domain)
//

#include "MyProject.h"
#include <CommonStates.h>
#include <Windowsx.h> // for GET__LPARAM macros
#include <SpriteBatch.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <DirectXColors.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

// helper function
//
// returns a random float between 0 & 1
float RandFloat() { return float(rand())/float(RAND_MAX); } 

//----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
	MyProject application(hInstance);    // Create the class variable

	if( application.InitWindowsApp(L"Timing and Movement", nShowCmd) == false )    // Initialize the window, if all is well show and update it so it displays
	{
		return 0;                   // Error creating the window, terminate application
	}

	if( application.InitializeDirect3D() )
	{
		application.SetDepthStencil(true);      // Tell DirectX class to create and maintain a depth stencil buffer
		application.InitializeTextures();
		application.MessageLoop();				// Window has been successfully created, start the application message loop
	}

	return 0;
}

//----------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------
MyProject::MyProject(HINSTANCE hInstance)
	: DirectXClass(hInstance)
{
	message = L"Mouse was clicked here";
	mousePos = Vector2(clientWidth * 0.5f, clientHeight * 0.5f);
	buttonDown = false;
	spriteBatch = NULL;
	
	

	ClearColor = Color(DirectX::Colors::Black.v);

	movementSpeed = 600;  // 60 pixels per second
	rotationSpeed = 90; 
}

//----------------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------------
MyProject::~MyProject()
{
	delete spriteBatch;
}

//----------------------------------------------------------------------------------------------
// Initialize any fonts we need to use here
//----------------------------------------------------------------------------------------------
void MyProject::InitializeTextures()
{
	// initialize the sprite batch
	states =new CommonStates(D3DDevice);
	spriteBatch = new DirectX::SpriteBatch( DeviceContext );

	backGroundTex.Load( D3DDevice, L"..\\Textures\\starfield.dds" );
	rockTex.Load( D3DDevice, L"..\\Textures\\rock.png" );

	for (int i = 0; i < 100; i++) {
		Color newColor = Color(rand() % 1 + 0.5f, rand() % 1 + 0.5f, rand() % 1 + 0.5f, 1);
		meteors[i].Initialize(&rockTex, Vector2(rand() % clientWidth, rand() % clientHeight), rand() % 180 + 90, rand() % 2 + 1, Vector2(rand() % 600 + 300, -(rand() % 600 + 300)), 90.0f, newColor, 0.0f);
	}

	for (int i = 100; i < 200; i++) {
		Color newColor = Color(rand() % 1 + 0.5f, rand() % 2, rand() % 1 + 0.5f, 1);
		meteors[i].Initialize(&rockTex, Vector2(rand() % clientWidth, rand() % clientHeight), rand() % 180 + 90, rand() % 2 + 1, Vector2(-(rand() % 600 + 300), rand() % 600 + 300), 90.0f, newColor, 0.0f);
	}
}

//----------------------------------------------------------------------------------------------
// Window message handler
//----------------------------------------------------------------------------------------------
LRESULT MyProject::ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg )
	{
	case WM_MOUSEMOVE:
		mousePos.x = (float) GET_X_LPARAM(lParam);
		mousePos.y = (float) GET_Y_LPARAM(lParam);
		return 0;
	case WM_LBUTTONUP:
		buttonDown = false;
		mousePos.x = (float) GET_X_LPARAM(lParam);
		mousePos.y = (float) GET_Y_LPARAM(lParam);
		break;
	case WM_LBUTTONDOWN:
		buttonDown = true;
		mousePos.x = (float) GET_X_LPARAM(lParam);
		mousePos.y = (float) GET_Y_LPARAM(lParam);
		OnMouseDown();
		break;
	case WM_KEYUP:
		if (wParam >= '0' && wParam <= '4')
		{
			PresentInterval = wParam - '0';
		}
		break;

	}

	// let the base class handle remaining messages
	return DirectXClass::ProcessWindowMessages(msg, wParam, lParam);
}

//----------------------------------------------------------------------------------------------
// Called by the render loop to render a single frame
//----------------------------------------------------------------------------------------------
void MyProject::Render(void)
{
	backGroundTex.Draw(DeviceContext, BackBuffer, 0, 0);

	// draw your stuff here
	spriteBatch->Begin(SpriteSortMode_Deferred,states->NonPremultiplied());
	
	for each (Sprite sprite in meteors)
		sprite.Draw(spriteBatch);
	spriteBatch->End();


	// render the base class
	DirectXClass::Render();
}

//----------------------------------------------------------------------------------------------
// Called every frame to update objects.
//	deltaTime: how much time in seconds has elapsed since the last frame
//----------------------------------------------------------------------------------------------
void MyProject::Update(float deltaTime)
{
	for (int i = 0; i < 200; i++) {
		// move the rock 
		Vector2 pos = meteors[i].GetPosition();
		pos.x += meteors[i].GetMoveSpeed().x * deltaTime;
		pos.y += meteors[i].GetMoveSpeed().y * deltaTime;
		meteors[i].SetPosition(pos);

		//rotate the rock
		float rot = meteors[i].GetRotation();
		rot += meteors[i].GetSpinSpeed() * deltaTime;
		meteors[i].SetRotation(rot);

		// if it's off screen, switch directions
		if (pos.x < 0) {
			Color newColor = Color(rand() % 1 + 0.5f, rand() % 1 + 0.5f, rand() % 2, 1);
			meteors[i].SetMoveSpeed(Vector2(fabsf(300 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (900 - 300)))), meteors[i].GetMoveSpeed().y));
			meteors[i].SetColor(newColor);
		}
		else if (pos.x > clientWidth) {
			Color newColor = Color(rand() % 2, rand() % 1 + 0.5f, rand() % 1 + 0.5f, 1);
			meteors[i].SetMoveSpeed(Vector2(-fabsf(300 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (900 - 300)))), meteors[i].GetMoveSpeed().y));
			meteors[i].SetColor(newColor);
		}

		if (pos.y < 0)
			meteors[i].SetMoveSpeed(Vector2(meteors[i].GetMoveSpeed().x, fabsf(300 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (900 - 300))))));
		else if (pos.y > clientHeight)
			meteors[i].SetMoveSpeed(Vector2(meteors[i].GetMoveSpeed().x , -fabsf(300 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (900 - 300))))));
	}
}

//----------------------------------------------------------------------------------------------
// Called when the mouse is released
//----------------------------------------------------------------------------------------------
void MyProject::OnMouseDown()
{
	// this is called when the left mouse button is clicked
	// mouse position is stored in mousePos variable


}