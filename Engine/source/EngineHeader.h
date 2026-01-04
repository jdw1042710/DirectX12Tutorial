#pragma once

#ifdef ENGINE_EXPORTS

#define D3D_API __declspec(dllexport)

#else

#define D3D_API __declspec(dllimport)

#endif // ENGINE_EXPORTS
