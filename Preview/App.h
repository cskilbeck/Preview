//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct App
{
	App() {}
	virtual ~App() {}

	virtual void Init() = 0;
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Release() = 0;
};