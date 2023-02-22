#include "class.h"

using namespace std;


ostream& operator<<(ostream& os, const coord& cd) {
	os << cd.x << " " << cd.y << " " << cd.z;
	return os;
}
bool operator==(const coord& cd1, const coord& cd2) {

	return cd1.x == cd2.x && cd1.y == cd2.y && cd1.z == cd2.z;
}

object* obj_init()
{
	object* newobj = new object();
	newobj->v_list = new std::vector<vertex*>();
	newobj->e_list = new std::vector<edge*>();
	newobj->f_list = new std::vector<face*>();
	newobj->vertices = new std::vector<GLfloat>();
	newobj->vertexIndices = new std::vector<unsigned int>();
	return newobj;
}

vertex* vtx_init()
{
	vertex* newvtx = new vertex();
	newvtx->e_list = new std::vector<edge*>();
	newvtx->f_list = new std::vector<face*>();
	newvtx->v_new = NULL;
	newvtx->idx = -1;
	return newvtx;
}

edge* edge_init()
{
	edge* newedge = new edge();
	newedge->f_list = new std::vector<face*>();
	newedge->v1 = NULL;
	newedge->v2 = NULL;
	newedge->edge_pt = NULL;
	return newedge; 
}

face* face_init()
{
	face* newface = new face();
	newface->v_list = new std::vector<vertex*>();
	newface->e_list = new std::vector<edge*>();
	newface->face_pt = NULL;
	return newface;
}

vertex* add_vertex(object* obj, const coord& coord)
{
	for (int i = 0; i < obj->v_list->size(); i++) {
		if ( (*obj->v_list)[i]->xyz == coord) {
			return (*obj->v_list)[i];
		}
	}
	vertex* newvtx = vtx_init();
	newvtx->xyz.x = coord.x;
	newvtx->xyz.y = coord.y;
	newvtx->xyz.z = coord.z;
	newvtx->idx = obj->v_list->size();
	obj->v_list->push_back(newvtx);
	return newvtx;
}

edge* find_edge(object* obj, vertex* v1, vertex* v2)
{
	std::vector<edge*>* v1_edgeList = v1->e_list;
	for(int i = 0; i < v1_edgeList->size(); i++)
	{
		if((*v1_edgeList)[i]->v1 == v2 || (*v1_edgeList)[i]->v2 == v2)
		{
			return (*v1_edgeList)[i];
		}
	}
	return NULL;
}

edge* add_edge(object* obj, vertex* v1, vertex* v2)
{
	edge* newedge = edge_init();
	newedge->v1 = v1;
	newedge->v2 = v2;
	v1->e_list->push_back(newedge);
	v2->e_list->push_back(newedge);
	obj->e_list->push_back(newedge);
	return newedge;
}

face* add_face(object* obj, const std::vector<int>& vertexIndices)
{
	face* newface = face_init();
	int n = vertexIndices.size();
	for (int i = 0; i < n; i++)
	{
		vertex* v1 = (*(obj->v_list))[vertexIndices[i]];
		vertex* v2 = (*(obj->v_list))[vertexIndices[(i+1)%n]];
		v1->f_list->push_back(newface);

		edge* temp = find_edge(obj, v1, v2);
		if(!temp) temp = add_edge(obj, v1, v2);

		temp->f_list->push_back(newface);
		newface->e_list->push_back(temp);
		newface->v_list->push_back(v1);
	}
	obj->f_list->push_back(newface);
	return newface;
}

coord add(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.x + ord2.x;
	temp.y = ord1.y + ord2.y;
	temp.z = ord1.z + ord2.z;
	return temp;
}

coord sub(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.x - ord2.x;
	temp.y = ord1.y - ord2.y;
	temp.z = ord1.z - ord2.z;
	return temp;
}

coord mul(const coord& ord1, GLfloat m)
{
	coord temp;
	temp.x = ord1.x * m;
	temp.y = ord1.y * m;
	temp.z = ord1.z * m;
	return temp;
}

coord div(const coord& ord1, GLfloat d)
{
	coord temp;
	temp.x = ord1.x / d;
	temp.y = ord1.y / d;
	temp.z = ord1.z / d;
	return temp;
}

coord cross(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.y * ord2.z - ord1.z * ord2.y;
	temp.y = ord1.z * ord2.x - ord1.x * ord2.z;
	temp.z = ord1.x * ord2.y - ord1.y * ord2.x;
	return temp;
}

void setNorm(object* obj)
{
	for (int i = 0; i < obj->f_list->size(); i++)
	{
		face* temp = (*(obj->f_list))[i];
		coord v01 = sub((*(temp->v_list))[1]->xyz, (*(temp->v_list))[0]->xyz);
		coord v12 = sub((*(temp->v_list))[2]->xyz, (*(temp->v_list))[1]->xyz);
		coord crs = cross(v01, v12);
		crs.normalize();
		temp->norm = crs;
	}

	for (int i = 0; i < obj->v_list->size(); i++)
	{
		coord sum;
		std::vector<face*>* temp = (*(obj->v_list))[i]->f_list;
		int n = temp->size();
		for (int j = 0; j < n; j++)
		{
			sum.add((*temp)[j]->norm);
		}
		sum.div((GLfloat)n);
		sum.normalize();
		(*(obj->v_list))[i]->avg_norm = sum;
	}
}

void aggregate_vertices(object* obj)
{
	obj->vertices->clear();
	obj->vertexIndices->clear();

	for (int i = 0; i < obj->v_list->size(); i++)
	{
		coord temp_pos = (*(obj->v_list))[i]->xyz;
		coord temp_norm = (*(obj->v_list))[i]->avg_norm;
		obj->vertices->push_back(temp_pos.x);
		obj->vertices->push_back(temp_pos.y);
		obj->vertices->push_back(temp_pos.z);
		obj->vertices->push_back(temp_norm.x);
		obj->vertices->push_back(temp_norm.y);
		obj->vertices->push_back(temp_norm.z);
	}

	if (obj->vertices_per_face == 3)
	{
		for (int i = 0; i < obj->f_list->size(); i++)
		{
			std::vector<vertex*>* temp = (*(obj->f_list))[i]->v_list;
			obj->vertexIndices->push_back((*temp)[0]->idx);
			obj->vertexIndices->push_back((*temp)[1]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
		}
	}

	else if (obj->vertices_per_face == 4)
	{
		for (int i = 0; i < obj->f_list->size(); i++)
		{
			std::vector<vertex*>* temp = (*(obj->f_list))[i]->v_list;
			obj->vertexIndices->push_back((*temp)[0]->idx);
			obj->vertexIndices->push_back((*temp)[1]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
			obj->vertexIndices->push_back((*temp)[3]->idx);
			obj->vertexIndices->push_back((*temp)[0]->idx);
		}
	}
}

object* cube()
{
	object* newobj = obj_init();
	newobj->vertices_per_face = 4;
	for (int x = -1; x <= 1; x += 2)
	{
		for (int y = -1; y <= 1; y += 2)
		{
			for (int z = -1; z <= 1; z += 2)
			{
				add_vertex(newobj, coord((GLfloat)x, (GLfloat)y, (GLfloat)z));
			}
		}
	}
	add_face(newobj, { 0,2,6,4 });
	add_face(newobj, { 0,4,5,1 });
	add_face(newobj, { 0,1,3,2 });
	add_face(newobj, { 2,3,7,6 });
	add_face(newobj, { 6,7,5,4 });
	add_face(newobj, { 1,5,7,3 });

	setNorm(newobj);

	aggregate_vertices(newobj);

	return newobj;
}


object* donut()
{
	object* m = obj_init();
	m->vertices_per_face = 4;
	int i;
	coord v[] = {
		{ -2, -.5, -2 }, { -2, -.5,  2 }, {  2, -.5, -2 }, {  2, -.5,  2 },
		{ -1, -.5, -1 }, { -1, -.5,  1 }, {  1, -.5, -1 }, {  1, -.5,  1 },
		{ -2,  .5, -2 }, { -2,  .5,  2 }, {  2,  .5, -2 }, {  2,  .5,  2 },
		{ -1,  .5, -1 }, { -1,  .5,  1 }, {  1,  .5, -1 }, {  1,  .5,  1 },
	};

	for (i = 0; i < 16; i++) add_vertex(m, coord(v[i].x, v[i].y, v[i].z));
	add_face(m, { 4, 5, 1, 0 });
	add_face(m, { 3, 1, 5, 7 });
	add_face(m, { 0, 2, 6, 4 });
	add_face(m, { 2, 3, 7, 6 });

	add_face(m, { 8, 9, 13, 12 });
	add_face(m, { 15, 13, 9, 11 });
	add_face(m, { 12, 14, 10, 8 });
	add_face(m, { 14, 15, 11, 10 });

	add_face(m, { 0, 1, 9, 8 });
	add_face(m, { 1, 3, 11, 9 });
	add_face(m, { 2, 0, 8, 10 });
	add_face(m, { 3, 2, 10, 11 });

	add_face(m, { 12, 13, 5, 4 });
	add_face(m, { 13, 15, 7, 5 });
	add_face(m, { 14, 12, 4, 6 });
	add_face(m, { 15, 14, 6, 7 });

	setNorm(m);

	aggregate_vertices(m);

	return m;
}

object* star()
{
	object* m = obj_init();
	m->vertices_per_face = 3;
	int ang, i;
	double rad;
	coord v[15];

	for (i = 0; i < 5; i++) {
		ang = i * 72;
		rad = ang * 3.1415926 / 180;
		v[i].x = 2.2 * cos(rad); v[i].y = 2.2 * sin(rad); v[i].z = 0;

		rad = (ang + 36) * 3.1415926 / 180;
		v[i + 5].x = v[i + 10].x = cos(rad);
		v[i + 5].y = v[i + 10].y = sin(rad);
		v[i + 5].z = .5;
		v[i + 10].z = -.5;
	}

	for (i = 0; i < 15; i++) add_vertex(m, coord(v[i].x, v[i].y, v[i].z));
	add_face(m, { 0, 5, 9 });
	add_face(m, { 1, 6, 5 });
	add_face(m, { 2, 7, 6 });
	add_face(m, { 3, 8, 7 });
	add_face(m, { 4, 9, 8 });

	add_face(m, { 0, 14, 10 });
	add_face(m, { 1, 10, 11 });
	add_face(m, { 2, 11, 12 });
	add_face(m, { 3, 12, 13 });
	add_face(m, { 4, 13, 14 });

	add_face(m, { 0, 10, 5 });
	add_face(m, { 1, 5, 10 });
	add_face(m, { 1, 11, 6 });
	add_face(m, { 2, 6, 11 });
	add_face(m, { 2, 12, 7 });
	add_face(m, { 3, 7, 12 });
	add_face(m, { 3, 13, 8 });
	add_face(m, { 4, 8, 13 });
	add_face(m, { 4, 14, 9 });
	add_face(m, { 0, 9, 14 });

	setNorm(m);

	aggregate_vertices(m);

	return m;
}

bool is_holeEdge(edge* e)
{
	return e->f_list->size() == 1;
	
}

bool is_holeVertex(vertex* v)
{
	if (v->f_list->size() != v->e_list->size()) {
		return true;
	}
	else { return false; }
	
}

vertex* face_point(face* f)
{
	if (f->face_pt != NULL) {
		return f->face_pt;
	}
	else {
		vertex* v= vtx_init();
		for (vertex* vtx :*(f->v_list)) {
			v->xyz.add(vtx->xyz);
		}
		v->xyz.div((GLfloat)f->v_list->size());
		
		return f->face_pt = v;
	}
	

}

vertex* edge_point(edge* e)
{
	if (e->edge_pt != NULL) {
	
		return e->edge_pt;
	}
	else {
		if (is_holeEdge(e)) {
			vertex* v = vtx_init();
			v->xyz.add(e->v1->xyz);
			v->xyz.add(e->v2->xyz);
			v->xyz.div((GLfloat)2.0f);
			return e->edge_pt = v;
		}
		else {
			vertex* v = vtx_init();
			for (face* fc : *e->f_list) {
				v->xyz.add(face_point(fc)->xyz);
			}
			v->xyz.add(e->v1->xyz);
			v->xyz.add(e->v2->xyz);
			v->xyz.div((GLfloat)e->f_list->size() + 2.0f);
			return e->edge_pt = v;

		}
	
	}
	
}

vertex* vertex_point(vertex* v)
{
	if (v->v_new != NULL) {
		return v->v_new;
	}
	else {
		if (is_holeVertex(v)) {
		
			vertex* nv = vtx_init();
			int cnt = 0;
			for (edge* ed : *v->e_list) {
				if (is_holeEdge(ed) == false) {
					continue;
				}
				cnt++;
				nv->xyz.add(ed->v1->xyz);
				nv->xyz.add(ed->v2->xyz);
			}
			nv->xyz.add(v->xyz);
			nv->xyz.div(cnt*2.0+1);
			return v->v_new = nv;
		}
		else {
			GLint n = v->f_list->size();
			
			vertex* nv = vtx_init();
			vertex* afp = vtx_init();
			for (face* fc : *v->f_list) {
				afp->xyz.add(face_point(fc)->xyz);
			}
			afp->xyz.div((GLfloat)v->f_list->size());
			vertex* amp = vtx_init();
			for (edge* eg : *v->e_list) {
				amp->xyz.add(eg->v1->xyz);
				amp->xyz.add(eg->v2->xyz);
			}
			amp->xyz.div((GLfloat)v->e_list->size());
			afp->xyz.div((GLfloat)n);
		
			amp->xyz.div((GLfloat)n);
			nv->xyz.add(v->xyz);
			nv->xyz.mul(n - 3);
			nv->xyz.div((GLfloat)n);
			nv->xyz.add(amp->xyz);
			nv->xyz.add(afp->xyz);
			delete amp;
			delete afp;
		/*	cout << nv->xyz.x << " " << nv->xyz.y << " " << nv->xyz.z << endl;*/
			return v->v_new = nv;
		}
	
	}
	
}

object* catmull_clark(object* obj)
{
	object* newobj = obj_init();
	newobj->vertices_per_face = 4;
	/*if (obj->vertices_per_face == 3) {
	
	
	*/

	vector<int> v;

	if (obj->vertices_per_face == 3) {

		

		for (int i = 0; i < obj->f_list->size(); i++) {
			face* curFace = (*obj->f_list)[i];
			vertex* va = (*curFace->v_list)[0];
			vertex* vb = (*curFace->v_list)[1];
			vertex* vc = (*curFace->v_list)[2];
		

			v.push_back(add_vertex(newobj, vertex_point(va)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, va, vb))->xyz)->idx);
			v.push_back(add_vertex(newobj, face_point(curFace)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vc, va))->xyz)->idx);

			v.push_back(add_vertex(newobj, vertex_point(vb)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vb, vc))->xyz)->idx);
			v.push_back(add_vertex(newobj, face_point(curFace)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, va, vb))->xyz)->idx);

			v.push_back(add_vertex(newobj, vertex_point(vc)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vc, va))->xyz)->idx);
			v.push_back(add_vertex(newobj, face_point(curFace)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vb, vc))->xyz)->idx);

			
		}
		/*for (vertex* v : *newobj->v_list) {
			cout << v->xyz.x <<","<<v->xyz.y << endl;

		}*/



		for (int i = 0; i < obj->f_list->size(); i++) {
			add_face(newobj, { v[i * 12],v[i * 12 + 1],v[i * 12 + 2],v[i * 12 + 3] });
			add_face(newobj, { v[i * 12 + 4],v[i * 12 + 5],v[i * 12 + 6],v[i * 12 + 7] });
			add_face(newobj, { v[i * 12 + 8],v[i * 12 + 9],v[i * 12 + 10],v[i * 12 + 11] });
		}
	}

	else {

		for (int i = 0; i < obj->f_list->size(); i++) {
			face* curFace = (*obj->f_list)[i];
			vertex* va = (*curFace->v_list)[0];
			vertex* vb = (*curFace->v_list)[1];
			vertex* vc = (*curFace->v_list)[2];
			vertex* vd = (*curFace->v_list)[3];

			v.push_back(add_vertex(newobj, vertex_point(va)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, va, vb))->xyz)->idx);
			v.push_back(add_vertex(newobj, face_point(curFace)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vd, va))->xyz)->idx);

			v.push_back(add_vertex(newobj, vertex_point(vb)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vb, vc))->xyz)->idx);
			v.push_back(add_vertex(newobj, face_point(curFace)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, va, vb))->xyz)->idx);

			v.push_back(add_vertex(newobj, vertex_point(vc)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vc, vd))->xyz)->idx);
			v.push_back(add_vertex(newobj, face_point(curFace)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vb, vc))->xyz)->idx);

			v.push_back(add_vertex(newobj, vertex_point(vd)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vd, va))->xyz)->idx);
			v.push_back(add_vertex(newobj, face_point(curFace)->xyz)->idx);
			v.push_back(add_vertex(newobj, edge_point(find_edge(obj, vc, vd))->xyz)->idx);

		}
		/*for (vertex* v : *newobj->v_list) {
			cout << v->xyz.x <<","<<v->xyz.y << endl;

		}*/



		for (int i = 0; i < obj->f_list->size(); i++) {
			add_face(newobj, { v[i * 16],v[i * 16 + 1],v[i * 16 + 2],v[i * 16 + 3] });
			add_face(newobj, { v[i * 16 + 4],v[i * 16 + 5],v[i * 16 + 6],v[i * 16 + 7] });
			add_face(newobj, { v[i * 16 + 8],v[i * 16 + 9],v[i * 16 + 10],v[i * 16 + 11] });
			add_face(newobj, { v[i * 16 + 12],v[i * 16 + 13],v[i * 16 + 14],v[i * 16 + 15] });
		}


	}


	setNorm(newobj);

	aggregate_vertices(newobj);

	delete obj;

	return newobj;
}
