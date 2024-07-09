#pragma once

struct Matrix {
private:
	int m_numRows = 1;
	int m_numCols = 1;
public:

	float* data;

	Matrix();
	Matrix(int numRow, int numCol);
	~Matrix();

	bool setData(int numRow, int numCol, float val);
};