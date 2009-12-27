//--------------------------------------------------------------------------------------
// File: md3 loader.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "resource.h"
#include "Md3.h"
#include "camera.h"

// convert from our coordinate system (looking down X)
// to OpenGL's coordinate system (looking down -Z)
D3DXMATRIXA16 quake2oglMatrix(	0, 0, -1, 0,
								-1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 0, 1);

// flip sign of 31 32 33 34 
D3DXMATRIXA16 right2leftMatrix( 1, 0, 0, 0, 
								0, 1, 0, 0,
								0, 0, -1, 0,
								0, 0, 0, 1);

LPDIRECT3DVERTEXBUFFER9 g_pVB;
LPDIRECT3DINDEXBUFFER9 g_IB;
LPDIRECT3DTEXTURE9 g_texture;
LPDIRECT3DTEXTURE9 g_texture_weapon;
LPDIRECT3DTEXTURE9 g_texture_weapon_4;
LPDIRECT3DTEXTURE9 g_texture_weapon_3;
LPDIRECT3DTEXTURE9 g_texture_weapon_2;

Md3* head;
Md3* upper;
Md3* lower;
Md3* weapon;

int skip = 153 - 90;

int g_lower_fps = 18;
int g_lower_startFrame = 173 - skip;
int g_lower_numFrames = 9;
int g_lower_frame = g_lower_startFrame;

WORD g_lower_indices[MD3_MAX_TRIANGLES*3];

int g_upper_fps = 15;
int g_upper_startFrame = 151;
int g_upper_numFrames = 1;
int g_upper_frame = g_upper_startFrame;

WORD g_upper_indices[MD3_MAX_TRIANGLES*3];

// Custom flexible vertex format (FVF) describing the custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

D3DXVECTOR3 vEyePt( 0.0f, 0.0f, -100.f );
D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
camera g_cam(&vEyePt, &vLookatPt, &vUpVec);

int g_mouseXOfs = 0;
int g_mouseYOfs = 0;

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext )
{
    // Typically want to skip back buffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
	head = new Md3("visor\\head.md3");
	upper = new Md3("visor\\upper.md3");
	lower = new Md3("visor\\lower.md3");
	weapon = new Md3("railgun\\railgun.md3");
	D3DXCreateTextureFromFile(pd3dDevice, L"visor\\red.tga", &g_texture);
	D3DXCreateTextureFromFile(pd3dDevice, L"railgun\\railgun1.jpg", &g_texture_weapon);
	D3DXCreateTextureFromFile(pd3dDevice, L"railgun\\railgun4.jpg", &g_texture_weapon_4);
	D3DXCreateTextureFromFile(pd3dDevice, L"railgun\\railgun3.jpg", &g_texture_weapon_3);
	D3DXCreateTextureFromFile(pd3dDevice, L"railgun\\railgun2.glow.jpg", &g_texture_weapon_2);

	//if( FAILED( pd3dDevice->CreateVertexBuffer( md3->m_surfaces[0].num_verts*sizeof(CUSTOMVERTEX),
	//	0 /*Usage*/, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
	//	return E_FAIL;

	//VOID* pVertices;

	//if(FAILED(g_pVB->Lock(0, sizeof(md3->m_surfaces[0].num_verts*sizeof(CUSTOMVERTEX)), (VOID**)&pVertices, 0 ) ) ) 
	//	return E_FAIL;

	//memcpy(pVertices, md3->m_vertex[0], sizeof(md3->m_surfaces[0].num_verts*sizeof(CUSTOMVERTEX)));
	//g_pVB->Unlock();

	//if( FAILED( pd3dDevice->CreateIndexBuffer( 16384 *sizeof(WORD),
	//	D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, 
	//	&g_IB, NULL ) ) )
	//	return E_FAIL;


	//VOID* pIndices;

	//if( FAILED(g_IB->Lock( 0, sizeof(md3->m_surfaces[0].num_triangles*sizeof(WORD)*3), (VOID**)&pIndices, 0) ) )
	//{
	//	SAFE_RELEASE(g_IB);
	//	return E_FAIL;
	//}

	//memcpy( pIndices, md3->m_indices[0] , sizeof(sizeof(WORD)*md3->m_surfaces[0].num_triangles*3) );
	//g_IB->Unlock();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50,50,50));
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	static float fLowerLastTime = 0.0f;

	if (fTime - fLowerLastTime > (1.0 / g_lower_fps)) {
		++g_lower_frame;
		if (g_lower_frame >= g_lower_startFrame+g_lower_numFrames) { g_lower_frame = g_lower_startFrame; }
		fLowerLastTime = fTime;

		for (int i = 0; i < lower->m_surfaces[0].num_triangles*3; ++i) {
			g_lower_indices[i] = lower->m_indices[0][i] + g_lower_frame*(lower->m_surfaces[0].num_verts);
		}
	}

	static float fUpperLastTime = 0.0f;

	if (fTime - fUpperLastTime > (1.0 / g_upper_fps)) {
		++g_upper_frame;
		if (g_upper_frame >= g_upper_startFrame+g_upper_numFrames) { g_upper_frame = g_upper_startFrame; }
		fUpperLastTime = fTime;

		for (int i = 0; i < upper->m_surfaces[0].num_triangles*3; ++i) {
			g_upper_indices[i] = upper->m_indices[0][i] + g_upper_frame*(upper->m_surfaces[0].num_verts);
		}
	}

	if (DXUTIsKeyDown(VK_LEFT)) {
		g_cam.strafe(2.0f);
	}
	if (DXUTIsKeyDown(VK_RIGHT)) {
		g_cam.strafe(-2.0f);
	}
	if (DXUTIsKeyDown(VK_UP)) {
		g_cam.move(2.0f);
	}
	if (DXUTIsKeyDown(VK_DOWN)) {
		g_cam.move(-2.0f);
	}

	POINT mousePos;
	GetCursorPos(&mousePos);
	g_mouseXOfs = (640/2) - mousePos.x;
	g_mouseYOfs = (480/2) - mousePos.y;
	g_cam.rotate(g_mouseXOfs, g_mouseYOfs);
	SetCursorPos((640/2),(480/2));
}

VOID SetupMatrices(IDirect3DDevice9* pd3dDevice)
{
	//// Set up world matrix
	//D3DXMATRIXA16 matWorld;
	//D3DXMatrixRotationX( &matWorld,  D3DXToRadian(-90.0));
	//pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	// Set up our view matrix. A view matrix can be defined given an eye point,
	// a point to lookat, and a direction for which way is up. Here, we set the
	// eye five units back along the z-axis and up three units, look at the 
	// origin, and define "up" to be in the y-direction.
	//D3DXVECTOR3 vEyePt( g_xPos, g_yPos, g_zPos );
	//D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	//D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &g_cam.m_position, &g_cam.m_lookat, &g_cam.m_up );
	D3DXMatrixMultiply(&matView, &right2leftMatrix, &matView);
	D3DXMatrixMultiply(&matView, &quake2oglMatrix, &matView);
	pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 45, 50, 170 ), 1.0f, 0 ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		SetupMatrices(pd3dDevice);

		//pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
		pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		pd3dDevice->SetTexture(0, g_texture);

		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);
		pd3dDevice->SetTransform(D3DTS_WORLD, &(matWorld));

		// lower.md3 tags
		// 0 = tag_torso
		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (lower->m_surfaces[0].num_frames*lower->m_surfaces[0].num_verts), lower->m_surfaces[0].num_triangles, g_lower_indices, D3DFMT_INDEX16, lower->m_vertices[0], sizeof(my_vertex));

		// upper.md3 tags
		// 0 = tag_head
		// 1 = tag_weapon
		// 2 = tag_torso
		int ofs_tag = g_lower_frame*(lower->m_header.num_tags);
		D3DXMATRIXA16 rotationMatrix1(lower->m_tags[0+ofs_tag].axis[0].x, lower->m_tags[0+ofs_tag].axis[0].y, lower->m_tags[0+ofs_tag].axis[0].z, 0,
									lower->m_tags[0+ofs_tag].axis[1].x, lower->m_tags[0+ofs_tag].axis[1].y, lower->m_tags[0+ofs_tag].axis[1].z, 0,
									lower->m_tags[0+ofs_tag].axis[2].x, lower->m_tags[0+ofs_tag].axis[2].y, lower->m_tags[0+ofs_tag].axis[2].z, 0,
									lower->m_tags[0+ofs_tag].origin.x, lower->m_tags[0+ofs_tag].origin.y, lower->m_tags[0+ofs_tag].origin.z, 1);

		pd3dDevice->GetTransform(D3DTS_WORLD, &matWorld);
		pd3dDevice->SetTransform(D3DTS_WORLD, &(rotationMatrix1*matWorld));

		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (upper->m_surfaces[0].num_frames*upper->m_surfaces[0].num_verts), upper->m_surfaces[0].num_triangles, g_upper_indices, D3DFMT_INDEX16, upper->m_vertices[0], sizeof(my_vertex));


		ofs_tag = g_upper_frame*(upper->m_header.num_tags);
		D3DXMATRIXA16 rotationMatrix2(upper->m_tags[1+ofs_tag].axis[0].x, upper->m_tags[1+ofs_tag].axis[0].y, upper->m_tags[1+ofs_tag].axis[0].z, 0,
									upper->m_tags[1+ofs_tag].axis[1].x, upper->m_tags[1+ofs_tag].axis[1].y, upper->m_tags[1+ofs_tag].axis[1].z, 0,
									upper->m_tags[1+ofs_tag].axis[2].x, upper->m_tags[1+ofs_tag].axis[2].y, upper->m_tags[1+ofs_tag].axis[2].z, 0,
									upper->m_tags[1+ofs_tag].origin.x, upper->m_tags[1+ofs_tag].origin.y, upper->m_tags[1+ofs_tag].origin.z, 1);

		pd3dDevice->GetTransform(D3DTS_WORLD, &matWorld);
		pd3dDevice->SetTransform(D3DTS_WORLD, &(rotationMatrix2*matWorld));

		pd3dDevice->SetTexture(0, g_texture_weapon);
		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (weapon->m_surfaces[0].num_frames*weapon->m_surfaces[0].num_verts), weapon->m_surfaces[0].num_triangles, weapon->m_indices[0], D3DFMT_INDEX16, weapon->m_vertices[0], sizeof(my_vertex));
		pd3dDevice->SetTexture(0, g_texture_weapon_2);
		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (weapon->m_surfaces[1].num_frames*weapon->m_surfaces[1].num_verts), weapon->m_surfaces[1].num_triangles, weapon->m_indices[1], D3DFMT_INDEX16, weapon->m_vertices[1], sizeof(my_vertex));
		pd3dDevice->SetTexture(0, g_texture_weapon_4);
		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (weapon->m_surfaces[2].num_frames*weapon->m_surfaces[2].num_verts), weapon->m_surfaces[2].num_triangles, weapon->m_indices[2], D3DFMT_INDEX16, weapon->m_vertices[2], sizeof(my_vertex));
		pd3dDevice->SetTexture(0, g_texture_weapon_3);
		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (weapon->m_surfaces[3].num_frames*weapon->m_surfaces[3].num_verts), weapon->m_surfaces[3].num_triangles, weapon->m_indices[3], D3DFMT_INDEX16, weapon->m_vertices[3], sizeof(my_vertex));
		pd3dDevice->SetTexture(0, g_texture_weapon);
		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (weapon->m_surfaces[4].num_frames*weapon->m_surfaces[4].num_verts), weapon->m_surfaces[4].num_triangles, weapon->m_indices[4], D3DFMT_INDEX16, weapon->m_vertices[4], sizeof(my_vertex));
		pd3dDevice->SetTexture(0, g_texture);

		// head.md3 tags
		// 0 = tag_head
		ofs_tag = g_upper_frame*(upper->m_header.num_tags);
		D3DXMATRIXA16 rotationMatrix0(upper->m_tags[0+ofs_tag].axis[0].x, upper->m_tags[0+ofs_tag].axis[0].y, upper->m_tags[0+ofs_tag].axis[0].z, 0,
									upper->m_tags[0+ofs_tag].axis[1].x, upper->m_tags[0+ofs_tag].axis[1].y, upper->m_tags[0+ofs_tag].axis[1].z, 0,
									upper->m_tags[0+ofs_tag].axis[2].x, upper->m_tags[0+ofs_tag].axis[2].y, upper->m_tags[0+ofs_tag].axis[2].z, 0,
									upper->m_tags[0+ofs_tag].origin.x, upper->m_tags[0+ofs_tag].origin.y, upper->m_tags[0+ofs_tag].origin.z, 1);

		pd3dDevice->SetTransform(D3DTS_WORLD, &(rotationMatrix0*matWorld));

		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, (head->m_surfaces[0].num_frames*head->m_surfaces[0].num_verts), head->m_surfaces[0].num_triangles, head->m_indices[0], D3DFMT_INDEX16, head->m_vertices[0], sizeof(my_vertex));

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    return 0;
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
	delete head;
	delete upper;
	delete lower;

	g_texture->Release();
	g_texture_weapon->Release();
	g_texture_weapon_4->Release();
	g_texture_weapon_3->Release();
	g_texture_weapon_2->Release();
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
}

void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, 
					  bool bSideButton1Down, bool bSideButton2Down, INT nMouseWheelDelta,
					  INT xPos, INT yPos, void* pUserContext)
{
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions
    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackMouse( OnMouse, false );

    // TODO: Perform any application-level initialization here

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"md3 loader" );
    DXUTCreateDevice( true, 640, 480 );

	SetCursorPos((640>>1),(480>>1));
    // Start the render loop
    DXUTMainLoop();

    // TODO: Perform any application-level cleanup here

    return DXUTGetExitCode();
}


