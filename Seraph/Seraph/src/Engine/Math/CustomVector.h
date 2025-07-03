#pragma once

template<int N, typename T>

#define Vector4(T)  Vec(4, T);
#define Vector3(T)  Vec(3, T);
#define Vector2(T)  Vec(2, T);

struct Vec {
	
	T values[N];

	Vec(int N, typename T);

	Vec(T* input);
};