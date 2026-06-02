#include "ObjReader.h"

#include <fstream>

size_t getFileSize(FILE* file)
{
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	return size;
}

// Out of the objs I've used, an average of 33 length lines is the least
// Overestimating isn't as bad as underestimating (nvm 400MB mem spike)
// 17,894,607 / 331,366 = 54.00
//  1,850,225 /  54,816 = 33.75
// 12,692,459 / 235,544 = 53.89
//    134,864 /   2,784 = 48.44

// Expect vector lengths to be about (line count / 3.33)
// v, vt, vn were all the same in 3/4 models
// smaller models are closer to (line count / 1.00)
size_t getLineEstimate(FILE* file)
{
	size_t size = getFileSize(file);

	size_t lineEst = size / 54;
	
	return size;
}

std::vector<GLH::Vertex> readObjFile(const std::string& name)
{
	/*
		# = comment
		g = group (name opt)
		mtllib = material (name)
		v = vertex (x, y, z, w opt)
		vt = texture coordinate (u, v, w opt)
		s = smooth shading (off, 1, 2, 3)
		f = face (vertex index) x3
		f = face (vertex index/texture index) x3
		f = face (vertex index/texture index/normal index) x3
		f = face (vertex index//normal index) x3

		I only care about vertex, texture coord, and face
	*/

	// strtof is the best function

	std::vector<GLH::Vertex> verts;

	std::vector<GLH::Vec3f> points;
	std::vector<GLH::Vec2f> texCoords;
	std::vector<GLH::Vec3f> normals;

	FILE* file = fopen(name.data(), "rb");
	if (file)
	{
		size_t lineNum = 0;
		//size_t lineEst = getLineEstimate(file);
		//points.reserve(lineEst / 3.33f);
		//texCoords.reserve(lineEst / 3.33f);
		//normals.reserve(lineEst / 3.33f);
		//verts.reserve(lineEst / 3.33f);

		char line[200];
		while (fgets(line, 200, file))
		{
			if (line[0] == 'v')
			{
				if (line[1] == 't' && line[2] == ' ')
				{
					char* str = &line[2];
					GLH::Vec2f point;
					point.u = strtof(str, &str);
					point.v = strtof(str, &str);
					texCoords.push_back(point);
				}
				if (line[1] == 'n' && line[2] == ' ')
				{
					char* str = &line[2];
					GLH::Vec3f point;
					point.x = strtof(str, &str);
					point.y = strtof(str, &str);
					point.z = strtof(str, &str);
					normals.push_back(point);
				}
				else if (line[1] == ' ')
				{
					char* str = &line[1];
					GLH::Vec3f point;
					point.x = strtof(str, &str);
					point.y = strtof(str, &str);
					point.z = strtof(str, &str);
					points.push_back(point);
				}
			}
			else if (line[0] == 'f' && line[1] == ' ')
			{
				char* str = &line[1];
				bool hasNormals = true;
				for (int i = 0; i < 3; ++i)
				{
					GLH::Vertex vertex = { 0 };
					int64_t coord = strtoll(str, &str, 10);
					int64_t texCoord = strtoll(str + 1, &str, 10);
					int64_t normal = strtoll(str + 1, &str, 10);

					// negative means size - pos

					if (coord < 0) coord = points.size() + coord + 1;
					if (coord > 0) vertex.pos = points[coord - 1];

					// textures are optional, default (0, 0) works fine
					if (texCoord < 0) texCoord = texCoords.size() + texCoord + 1;
					if (texCoord > 0) vertex.tex = texCoords[texCoord - 1];

					// normals are optional, generate if not found
					if (normal < 0) normal = normals.size() + normal + 1;
					if (normal > 0) vertex.norm = normals[normal - 1];
					// there shouldn't be normals missing on SOME
					// but if so, regenerate all
					else hasNormals = false; 

					verts.push_back(vertex);
				}
				if (!hasNormals)
				{
					// These are per face normals, not per vertex
					// I duplicate points for faces, no indexes, so it doesn't matter
					GLH::Vec3f u = verts[verts.size() - 1 - 1].pos - verts[verts.size() - 1].pos;
					GLH::Vec3f v = verts[verts.size() - 1 - 2].pos - verts[verts.size() - 1].pos;

					GLH::Vec3f norm = u.cross(v).normalize();

					verts[verts.size() - 1 - 0].norm = norm;
					verts[verts.size() - 1 - 1].norm = norm;
					verts[verts.size() - 1 - 2].norm = norm;
				}
			}

			++lineNum;
		}

		//printf("\"%s\"\nlines:%llu size:%llu lineLen:%.2f\n", name.data(), lineNum, fileSize, fileSize / (float)lineNum);
		//printf("v:%llu\nvt:%llu\nvn:%llu\nverts:%llu\n\n",
		//	points.size(), texCoords.size(), normals.size(), verts.size());

		fclose(file);
	}

	return verts;
}