#ifndef RESOURCE_UTILS_H
#define RESOURCE_UTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "opengl_utils.h"
using namespace std;
VAO* loadSplineControlPoints(string path)
{

	ifstream in(path);
	
	float tmp;
	vector<float> data;
	in >> tmp;

	while (in) {
		in >> tmp;
		if (!in) {
			break;
		}
		data.push_back(tmp);
	}
	in.close();
	vector<unsigned int> size;
	size.push_back(3);
	return getVAOFromAttribData(data, size);
}

VAO* loadBezierSurfaceControlPoints(string path)
{
	//(Optional)TODO: load surface control point data and return VAO.
	//You can make use of getVAOFromAttribData in opengl_utils.h
	ifstream in(path);
	
	int N, I, J;
	in >> N;
	float tmp;
	vector<float> data;
	for (int k = 0; k < N; k++) {
		in >> I >> J;

		for (int w = 0; w < 48; w++) {
			in >> tmp;
			data.push_back(tmp);
		}


	}
	in.close();
	vector<unsigned int> size;
	size.push_back(3);
	return getVAOFromAttribData(data, size);
}
#endif