#include "MyMath.h"

Matrix::Matrix() {
	m_numCols = 1;
	m_numRows = 1;
	data = new float(1);
};

Matrix::Matrix(int numRow, int numCol) {
	
	if (numRow  > 1) {
		m_numRows = numRow;
	} else {
		m_numRows = 1;
	}

	if (numCol > 1) {
		m_numCols = numCol;
	}
	else {
		m_numCols = 1;
	}
	
	data = new float(m_numRows * m_numCols);

	int x = sizeof(data);

	for (int index = 0; index < m_numRows * m_numCols; index++) {
		data[index] = 0;
	}

};

Matrix::~Matrix() {};

bool Matrix::setData(int numRow, int numCol, float val) {
	if (numRow >= 0 && numRow <= m_numRows) {
		data[numRow * m_numRows + numCol] = val;
		return true;
	}
	return false;
};
