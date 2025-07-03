#include "CustomVector.h"
 
 template<int N, typename T>
 Vec<N, T>::Vec(int N, typename T) {
	 
	 values = T[N];

	 for (int counter = 0; counter < N; counter++) {
		 values[counter] = 0;
	 }
 }

 template<int N, typename T>
 Vec<N, T>::Vec(T* input) {

	 for (int counter = 0; counter < N; counter++) {
		 values[counter] = (*input);
		 input++;
	 }
 }

 