/*
 * $Id: sample.cpp,v 1.23 2006/11/01 23:33:56 nathanst Exp $
 *
 *  sample.cpp
 *  hog
 *
 *  Created by Nathan Sturtevant on 5/31/05.
 *  Copyright 2005 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 * This file is part of HOG.
 *
 * HOG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * HOG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Common.h"
#include "Sample.h"
#include "ScenarioLoader.h"
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "Voxels.h"

//struct voxelWorld {
//	uint32_t header;
//	float voxelSize;
//	uint64_t numVoxelsGrids;
//	float minbounds[4];
//	float maxbounds[4];
//	uint64_t *morton;
//	uint64_t *grid;
//};
//
//voxelWorld LoadData();
//void Draw(voxelWorld w);
bool mouseTracking = false;
bool recording = false;

recVec cameraOffset(0,0,0);
Voxels *v = 0;
//voxelWorld theWorld;

int main(int argc, char* argv[])
{
//	theWorld = LoadData();
	InstallHandlers();
	RunHOGGUI(argc, argv, 1024, 512);
}


/**
 * This function is used to allocate the unit simulated that you want to run.
 * Any parameters or other experimental setup can be done at this time.
 */
void CreateSimulation(int id)
{
//	//FILE *f = fopen("/Users/nathanst/Desktop/3DNavMaps/test3dnavC1.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full4_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full3_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full2_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full1_test3dnav.3dnav", "r");
//	FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Complex_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Simple_test3dnav.3dnav", "r");

	v = new Voxels("/Users/nathanst/Desktop/3dmaps/Complex_test3dnav.3dnav");
//	Map *map;
//	if (gDefaultMap[0] == 0)
//	{
//		map = new Map(mazeSize, mazeSize);
//		MakeMaze(map, 10);
////		map->Scale(mazeSize, mazeSize);
//	}
//	else {
//		map = new Map(gDefaultMap);
//		//map->Scale(512, 512);
//	}
//	
//	if (0 && gDefaultMap[0] == 0)
//	{
//		MakeStaircase();
////		m3d = new Map3DGrid(map, 16);
//		//m3d->AddMap(map, 0);
//		m3d->SetDrawGrid(true);
//		m3d->PrintStats();
//		//m3d->AddMap(map, 50);
//	}
//	else {
//		m3d = new Map3DGrid(map, 16);
//	}
//	map->SetTileSet(kWinter);
//	msa = new MapSectorAbstraction(map, 8);
//	msa->ToggleDrawAbstraction(0);
//	//msa->ToggleDrawAbstraction(2);
//	//msa->ToggleDrawAbstraction(3);
//	unitSims.resize(id+1);
//	unitSims[id] = new UnitSimulation<xyLoc, tDirection, MapEnvironment>(new MapEnvironment(map));
//	unitSims[id]->SetStepType(kMinTime);
//	m3d->SetDrawGrid(true);
}

/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */
void InstallHandlers()
{
//	InstallKeyboardHandler(MyDisplayHandler, "Toggle Abstraction", "Toggle display of the ith level of the abstraction", kAnyModifier, '0', '9');
	InstallKeyboardHandler(MyDisplayHandler, "Reset Cam", "Reset camera", kAnyModifier, '|');
	InstallKeyboardHandler(MyDisplayHandler, "Record", "Begin/end movie recording", kAnyModifier, 'r');
	InstallKeyboardHandler(MyDisplayHandler, "Fly", "Fly camera forward", kAnyModifier, 'w');
	InstallKeyboardHandler(MyDisplayHandler, "Fly", "Fly camera back", kAnyModifier, 's');
	InstallKeyboardHandler(MyDisplayHandler, "Fly", "Slide camera left", kAnyModifier, 'a');
	InstallKeyboardHandler(MyDisplayHandler, "Fly", "Slide camera right", kAnyModifier, 'd');
	InstallWindowHandler(MyWindowHandler);
//	
//	InstallMouseClickHandler(MyClickHandler);
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType)
{
	if (eType == kWindowDestroyed)
	{
		printf("Window %ld destroyed\n", windowID);
		RemoveFrameHandler(MyFrameHandler, windowID, 0);
		
		mouseTracking = false;
	}
	else if (eType == kWindowCreated)
	{
		printf("Window %ld created\n", windowID);
		InstallFrameHandler(MyFrameHandler, windowID, 0);
		CreateSimulation(windowID);
		SetNumPorts(windowID, 1);
	}
}


void MyDisplayHandler(unsigned long windowID, tKeyboardModifier mod, char key)
{
	switch (key)
	{
		case '|': resetCamera(); break;
		case 'r': recording = !recording; break;
		case 'w':
		{
			recVec v = cameraLookingAt();
			v.normalise();
			v *= 0.05;
			cameraOffset += v;
			break;
		}			
		case 's':
		{
			recVec v = cameraLookingAt();
			v.normalise();
			v *= 0.05;
			cameraOffset -= v;
			break;
		}
	}
}

void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *)
{
	rotateObject();
	glTranslatef(cameraOffset.x, cameraOffset.y, cameraOffset.z);
	v->OpenGLDraw();
	//LoadData();
	//Draw(theWorld);

	if (recording && viewport == GetNumPorts(windowID)-1)
	{
		static int cnt = 0;
		char fname[255];
		sprintf(fname, "/Users/nathanst/Movies/tmp/%d%d%d%d", (cnt/1000)%10, (cnt/100)%10, (cnt/10)%10, cnt%10);
		SaveScreenshot(windowID, fname);
		printf("Saved %s\n", fname);
		cnt++;
	}
}


bool MyClickHandler(unsigned long windowID, int, int, point3d loc, tButtonType button, tMouseEventType mType)
{
	return false;
}

//// "Insert" two 0 bits after each of the 10 low bits of x
//uint32_t Part1By2(uint32_t x)
//{
//	x &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
//	x = (x ^ (x << 16)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
//	x = (x ^ (x <<  8)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
//	x = (x ^ (x <<  4)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
//	x = (x ^ (x <<  2)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
//	return x;
//}
//
//uint32_t EncodeMorton3(uint32_t x, uint32_t y, uint32_t z)
//{
//	return (Part1By2(z) << 2) + (Part1By2(y) << 1) + Part1By2(x);
//}
//
//// Inverse of Part1By2 - "delete" all bits not at positions divisible by 3
//uint32_t Compact1By2(uint32_t x)
//{
//	x &= 0x09249249;                  // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
//	x = (x ^ (x >>  2)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
//	x = (x ^ (x >>  4)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
//	x = (x ^ (x >>  8)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
//	x = (x ^ (x >> 16)) & 0x000003ff; // x = ---- ---- ---- ---- ---- --98 7654 3210
//	return x;
//}
//
//uint32_t DecodeMorton3X(uint32_t code)
//{
//	return Compact1By2(code >> 0);
//}
//
//uint32_t DecodeMorton3Y(uint32_t code)
//{
//	return Compact1By2(code >> 1);
//}
//
//uint32_t DecodeMorton3Z(uint32_t code)
//{
//	return Compact1By2(code >> 2);
//}
//
//size_t GetIndex(size_t x, size_t y, size_t z)
//{
//	return(x * 16 + y * 4 + z);
//}
//
//void GetCoordsForIndex(size_t i, size_t& x, size_t& y, size_t& z)
//{
//	x = i >> 4;
//	y = (i & 0x0F) >> 2;
//	z = (i & 0x03);
//}
//
//point3d GetVoxelCoordinate(uint64_t morton, float voxelSize, float minbounds[4])
//{
//	point3d pt(DecodeMorton3X(morton), DecodeMorton3Y(morton), DecodeMorton3Z(morton));
//	pt.x*=voxelSize*4;
//	pt.x += minbounds[0];
//	pt.y*=voxelSize*4;
//	pt.y += minbounds[1];
//	pt.z*=voxelSize*4;
//	pt.z += minbounds[2];
//	//const size_t ix = static_cast<size_t>(floorf((coords.x - mBounds.MinX()) / (voxelSize * 4)));
//	return pt;
//}
//
//voxelWorld LoadData()
//{
//
//	//FILE *f = fopen("/Users/nathanst/Desktop/3DNavMaps/test3dnavC1.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full4_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full3_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full2_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Full1_test3dnav.3dnav", "r");
//	FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Complex_test3dnav.3dnav", "r");
//	//FILE *f = fopen("/Users/nathanst/Desktop/3dmaps/Simple_test3dnav.3dnav", "r");
//	if (f == 0)
//	{
//		printf("Error opening file\n");
//		exit(0);
//	}
//	voxelWorld w;
//	fread(&w.header, sizeof(w.header), 1, f);
//	printf("Header is 0x%lX\n", w.header);
//	fread(&w.voxelSize, sizeof(w.voxelSize), 1, f);
//	printf("Voxel size is %f\n", w.voxelSize);
//	fread(&w.numVoxelsGrids, sizeof(w.numVoxelsGrids), 1, f);
//	printf("%llu voxel grids to follow\n", w.numVoxelsGrids);
//	fread(w.minbounds, sizeof(w.minbounds[0]), 4, f);
//	fread(w.maxbounds, sizeof(w.maxbounds[0]), 4, f);
//	printf("Min bounds: ");
//	for (int x = 0; x < 4; x++)
//	{
//		printf("%f ", w.minbounds[x]);
//	}
//	printf("\n");
//	printf("Max bounds: ");
//	for (int x = 0; x < 4; x++)
//	{
//		printf("%f ", w.maxbounds[x]);
//	}
//	printf("\n");
//	w.morton = new uint64_t[w.numVoxelsGrids];
//	w.grid = new uint64_t[w.numVoxelsGrids];
//	for (int x = 0; x < w.numVoxelsGrids; x++)
//	{
//		
//		assert(fread(&w.morton[x], sizeof(w.morton[x]), 1, f) == 1);
//		assert(fread(&w.grid[x], sizeof(w.grid[x]), 1, f) == 1);
//		printf("0x%llX\n", w.morton[x]);
//		point3d p = GetVoxelCoordinate(w.morton[x], w.voxelSize, w.minbounds);
//		printf("(%f, %f, %f)\n", p.x, p.y, p.z);
//		printf("0x%llX\n", w.grid[x]);
//		for (int i = 0; i < 64; i++)
//		{
//			size_t a, b, c;
//			GetCoordsForIndex(i, a, b, c);
//			if ((w.grid[x]>>i)&1) // blocked
//			{
//				//printf("{%f, %f, %f} ", (p.x+a*w.voxelSize)/8, (p.y+b*w.voxelSize)/8, (p.z+c*w.voxelSize)/8);
//			}
//		}
//		printf("\n");
//	}
//
//	printf("\n");
//	
//	fclose(f);
//	return w;
//}
//
//void Draw(voxelWorld w)
//{
//	double xRange = max(w.maxbounds[0],-w.minbounds[0]);
//	double yRange = max(w.maxbounds[1],-w.minbounds[1]);
//	double zRange = max(w.maxbounds[2],-w.minbounds[2]);
//	double range = std::max(xRange, std::max(yRange, zRange));
//	for (int x = 0; x < w.numVoxelsGrids; x++)
//	{
//		point3d p = GetVoxelCoordinate(w.morton[x], w.voxelSize, w.minbounds);
//		bool drawFrame = false;
//		for (int i = 0; i < 64; i++)
//		{
//			size_t a, b, c;
//			GetCoordsForIndex(i, a, b, c);
//			if ((w.grid[x]>>i)&1) // blocked
//			{
//				drawFrame = true;
//				GLfloat rr, gg, bb;
//				rr = (1.+(p.x+a*w.voxelSize)/range)/2.0;
//				gg = (1.+(p.y+b*w.voxelSize)/range)/2.0;
//				bb = (1.+(p.z+c*w.voxelSize)/range)/2.0;
//				// 7?
//				rr = getColor(rr, 0, 1, 7).r;
//				gg = getColor(bb, 0, 1, 9).g*0.9;
//				bb = getColor(bb, 0, 1, 9).b;
//				glColor3f(gg, rr, bb);
//				glEnable(GL_LIGHTING);
//				DrawBox((p.x+a*w.voxelSize+0.5*w.voxelSize)/range,
//						-(p.y+b*w.voxelSize+0.5*w.voxelSize)/range,
//						(p.z+c*w.voxelSize+0.5*w.voxelSize)/range,
//						1*(w.voxelSize/2.)/range);
//			}
//		}
//		//if (drawFrame)
//		if (0)
//		{
//			//DrawBoxFrame(p.x+w.voxelSize*2, p.y+w.voxelSize*2, p.z+w.voxelSize*2, w.voxelSize*2);
//			glDisable(GL_LIGHTING);
//			glColor4f(1, 1, 1, 1);
//			DrawBoxFrame((p.x+2*w.voxelSize)/range,
//						 (p.y+2*w.voxelSize)/range,
//						 (p.z+2*w.voxelSize)/range,
//						 2*w.voxelSize/range);
//		}
//	}
//}
