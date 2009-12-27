#include "DXUT.h"
#include "Md3.h"

//void fixCoords(D3DXVECTOR3& v) 
//{
//	float temp = v.y;
//	v.y = v.z;
//	v.z = temp;
//}

Md3::Md3(void)
{
//	m_header = NULL;
	m_frames	= NULL;
	m_tags		= NULL;
	m_surfaces	= NULL;
	m_shaders	= NULL;
	m_triangles	= NULL;
	m_texcoords	= NULL;
	m_normals	= NULL;
	m_vertices	= NULL;
	m_indices	= NULL;
}

Md3::~Md3(void)
{
	if (m_shaders != NULL) {
		for (int i = 0; i < m_header.num_surfaces; i++) {
			delete [] m_shaders[i];
		}
		delete [] m_shaders;
	}

	if (m_triangles != NULL) {
		for (int i = 0; i < m_header.num_surfaces; i++) {
			delete [] m_triangles[i];
		}
		delete [] m_triangles;
	}

	if (m_texcoords != NULL) {
		for (int i = 0; i < m_header.num_surfaces; i++) {
			delete [] m_texcoords[i];
		}
		delete [] m_texcoords;
	}

	if (m_normals != NULL) {
		for (int i = 0; i < m_header.num_surfaces; i++) {
			delete [] m_normals[i];
		}
		delete [] m_normals;
	}

	if (m_vertices != NULL) {
		for (int i = 0; i < m_header.num_surfaces; i++) {
			delete [] m_vertices[i];
		}
		delete [] m_vertices;
	}

	if (m_indices != NULL) {
		for (int i = 0; i < m_header.num_surfaces; i++) {
			delete [] m_indices[i];
		}
		delete [] m_indices;
	}

	//if (m_header != NULL) delete m_header;
	if (m_frames != NULL) delete [] m_frames;
	if (m_tags != NULL) delete [] m_tags;
	if (m_surfaces != NULL) delete [] m_surfaces;
}

// TODO: maybe free unused data like files' index and vertexes! (got my own structure for that)
Md3::Md3(std::string filename)
{
	std::ifstream fin(filename.c_str(), std::ios::binary);

	fin.read((char *)(&m_header), sizeof(md3_header));

	m_frames	= new md3_frame[m_header.num_frames];
	m_tags		= new md3_tag[m_header.num_tags*m_header.num_frames];
	m_surfaces	= new md3_surface[m_header.num_surfaces];
	m_shaders	= new md3_shader*[m_header.num_surfaces];
	m_triangles = new md3_triangle*[m_header.num_surfaces];
	m_texcoords = new md3_texcoord*[m_header.num_surfaces];
	m_normals	= new md3_vertex*[m_header.num_surfaces];
	m_vertices	= new my_vertex*[m_header.num_surfaces];
	m_indices	= new WORD*[m_header.num_surfaces];

	fin.seekg(m_header.ofs_frames);
	for(int i = 0; i < m_header.num_frames; ++i) {
		fin.read((char *) (&m_frames[i]), sizeof(md3_frame));
	}

	fin.seekg(m_header.ofs_tags);
	for(int i = 0; i < m_header.num_tags*m_header.num_frames; ++i) {
		fin.read((char *) (&m_tags[i]), sizeof(md3_tag));
	}

	fin.seekg(m_header.ofs_surfaces);
	for(int i = 0; i < m_header.num_surfaces; ++i) {
		fin.read((char *) (&m_surfaces[i]), sizeof(md3_surface));

		m_shaders[i]	= new md3_shader[m_surfaces[i].num_shaders];
		m_triangles[i]	= new md3_triangle[m_surfaces[i].num_triangles];
		m_texcoords[i]	= new md3_texcoord[(m_surfaces[i].num_verts)*(m_surfaces[i].num_frames)];
		m_normals[i]	= new md3_vertex[(m_surfaces[i].num_verts)*(m_surfaces[i].num_frames)];
		m_vertices[i]	= new my_vertex[(m_surfaces[i].num_verts)*(m_surfaces[i].num_frames)];
		m_indices[i]	= new WORD[(m_surfaces[i].num_triangles)*3];

		fin.seekg(m_header.ofs_surfaces+m_surfaces[i].ofs_shaders);
		for(int j = 0; j < m_surfaces[i].num_shaders; ++j) {
			fin.read((char*)(&m_shaders[i][j]), sizeof(md3_shader));
		}

		fin.seekg(m_header.ofs_surfaces+m_surfaces[i].ofs_triangles);
		for(int j = 0, k = 0; j < m_surfaces[i].num_triangles; ++j, k += 3) {
			fin.read((char*)(&m_triangles[i][j]), sizeof(md3_triangle));
			m_indices[i][k+0] = m_triangles[i][j].indexes[0];
			m_indices[i][k+1] = m_triangles[i][j].indexes[2];
			m_indices[i][k+2] = m_triangles[i][j].indexes[1];
			// switch y and z coordinates 
		}

		fin.seekg(m_header.ofs_surfaces+m_surfaces[i].ofs_st);
		for(int j = 0; j < m_surfaces[i].num_verts; ++j) {
			fin.read((char*)(&m_texcoords[i][j]), sizeof(md3_texcoord));
		}

		for(int j = 0, k = 0; j < m_surfaces[i].num_frames*m_surfaces[i].num_verts; ++j, ++k) {
			if (k >= m_surfaces[i].num_verts) k = 0;
			m_vertices[i][j].u = m_texcoords[i][k].st[0];
			m_vertices[i][j].v = m_texcoords[i][k].st[1];
		}

		fin.seekg(m_header.ofs_surfaces+m_surfaces[i].ofs_xyznormal);
		for(int j = 0; j < m_surfaces[i].num_frames*m_surfaces[i].num_verts; ++j) {
			fin.read((char*)(&m_normals[i][j]), sizeof(md3_vertex));
			m_vertices[i][j].position.x = (float)m_normals[i][j].coord[0] * (1.0/64);
			m_vertices[i][j].position.y = (float)m_normals[i][j].coord[1] * (1.0/64);
			m_vertices[i][j].position.z = (float)m_normals[i][j].coord[2] * (1.0/64);

			// extracting normals with code from q3source :)
			float lat;
			float lng;
			lat = ( m_normals[i][j].normal >> 8 ) & 0xff;
			lng = ( m_normals[i][j].normal & 0xff );
			lat *= Q_PI/128;
			lng *= Q_PI/128;

			m_vertices[i][j].normal.x = cos(lat) * sin(lng);
			m_vertices[i][j].normal.y = sin(lat) * sin(lng);
			m_vertices[i][j].normal.z = cos(lng);
		}

		fin.seekg(m_header.ofs_surfaces+m_surfaces[i].ofs_end);
		m_header.ofs_surfaces = m_header.ofs_surfaces+m_surfaces[i].ofs_end;
	}
}
