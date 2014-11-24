//////////////////////////////////////////////////////////////////////

#pragma once

namespace Graphics
{
	bool Init(HWND hWnd);
	void Resize(int width, int height);
	void Release();
	void ClearBackBuffer(Color color);
	void Present();

	void EnableScissoring(bool enable);
	void SetScissorRectangle(Rect2D const &rect);

	extern DXPtr<ID3D11Device>			Device;
	extern DXPtr<ID3D11DeviceContext>	Context;

};

//////////////////////////////////////////////////////////////////////
