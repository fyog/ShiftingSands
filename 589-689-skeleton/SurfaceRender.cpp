#include "BSpline.h"
#include "Geometry.h"


//When the the control points for the surface are generated, they will be passed to the surface generator as either a vector of glm::vec3 vertices, or else a CPU_Geometry object (which contains a vertex vector)

/* If the width of a surface is k units, and it's length is j, then the vertex vector should be constructed such that it has one dimension, but every block of k vertices will represent some row of control points,
and there should be j blocks of k control points. So, let's make a function that allows us to refer to vertices in the vector as though it were a 2d array. We'll pass it a CPU_Geom though.*/

glm::vec3 point2dindex(CPU_Geometry obj, int rowdex, int coldex, int width)
{
	glm::vec3 returner = obj.verts[(width * coldex) + rowdex]; //If this were a 2d [width] by [length] array, then this element would be equivalent to the [rowdex][coldex] element
	return returner; 
}

void getcontrolpoints(CPU_Geometry control, CPU_Geometry* copy, int index, int width, int leng, bool alongwidth)
{
	copy->verts.clear();
	int i;
	glm::vec3 temp;
	if (alongwidth)
	{
		for (i = 0; i < width; i++)
		{
			temp = point2dindex(control, i, index, width); //If alongwidth is true, then we get every element in the row at column "index"
			copy->verts.push_back(temp);
		}
	}
	else
	{
		for (i = 0; i < leng; i++)
		{
			temp = point2dindex(control, index, i, width); //If alongwidth is false, then we go down the column "index"
			copy->verts.push_back(temp);
		}
	}
}

glm::vec3 onesplinepoint(CPU_Geometry control, int width, int length, float u, float v, float * uknots, float * vknots)
{
	//std::cout << "Entering onesplinepoint()\n";
	//onesplinepoint() will use the quickspline() function to calculate one point on the b-spline surface.
	//We assume that u is going along the width, and v is going along the length. 
	CPU_Geometry ucontrol, vrange;
	int m = length - 1;
	int k = 4; //We'll assume all b-spline curves should be of order four (cubic)
	int vdelta = getdelta(vknots, (m + k + 1), v, 0);

	m = width - 1;
	int i;
	glm::vec3 temp;
	//std::cout << "Initial calls to quickspline()\n";
	for (i = 0; i < k; i++)
	{
		getcontrolpoints(control, &ucontrol, (vdelta - i), width, length, true); //This gets the control points for a row at column (vdelta - i) which we'll need for the quickspine() call
		temp = quickspline(&ucontrol, uknots, u, k, m);
		vrange.verts.push_back(temp);
	}
	//std::cout << "Initial calls to quickspline() succeeded\n";

	int r, tempdelt;
	float omega;

	for (r = k; r >= 2; r--) //on each iteration of the outer loop, the range of values to consider shrinks
	{
		tempdelt = vdelta;
		for (i = 0; i < (r - 1); i++)
		{
			omega = calcomega(vknots, v, tempdelt, r);
			vrange.verts[i] = onesplinecalc(omega, &vrange, i); //onesplinecalc() handles the actual point/line calculations
			tempdelt = tempdelt - 1;
		}
	}
	return vrange.verts[0];
}

void placesurfacevecs(CPU_Geometry control, CPU_Geometry* surface, int width, int length)
{
	//std::cout << "Entering placesurfacevecs()\n";
	float * uknots;
	float * vknots;
	int k = 4;
	glm::vec3 temp;

	CPU_Geometry ucontrol;
	CPU_Geometry vcontrol;

	int m = width - 1;
	uknots = (float*)malloc((m + k + 1) * sizeof(float));
	m = length - 1;
	vknots = (float*)malloc((m + k + 1) * sizeof(float));
	//std::cout << "Creating standard knots\n";
	standardknots(uknots, k, (width - 1));
	standardknots(vknots, k, (length - 1));
	//std::cout << "standard knots created\n";

	float step = 0.01f;
	float u, v; //

	surface->verts.clear();

	for (v = 0.f; v <= 1.f; v += step)
	{
		for (u = 0.f; u <= 1.f; u += step)
		{
			if (u == 0.f && v == 0.f)
			{
				temp = point2dindex(control, 0, 0, width);
				surface->verts.push_back(temp);
			}
			else if (u == 1.f && v == 1.f)
			{
				temp = point2dindex(control, (width - 1), (length - 1), width);
				surface->verts.push_back(temp);
			}
			else if (u == 0.f && v == 1.f)
			{
				temp = point2dindex(control, 0, (length - 1), width);
				surface->verts.push_back(temp);
			}
			else if (u == 1.f && v == 0.f)
			{
				temp = point2dindex(control, (width - 1), 0, width);
				surface->verts.push_back(temp);
			}
			else if (u == 0.f) //u == 0, but v is between 0 and 1
			{
				getcontrolpoints(control, &vcontrol, 0, width, length, false);
				temp = quickspline(&vcontrol, vknots, v, k, (length - 1));
				surface->verts.push_back(temp);
			}
			else if (u == 1.f)
			{
				getcontrolpoints(control, &vcontrol, (length - 1), width, length, false);
				temp = quickspline(&vcontrol, vknots, v, k, (length - 1));
				surface->verts.push_back(temp);
			}
			else if (v == 0.f)
			{
				getcontrolpoints(control, &ucontrol, 0, width, length, true);
				temp = quickspline(&ucontrol, uknots, u, k, (width - 1));
				surface->verts.push_back(temp);
			}
			else if (v == 1.f)
			{
				getcontrolpoints(control, &ucontrol, (width - 1), width, length, true);
				temp = quickspline(&ucontrol, uknots, u, k, (width - 1));
				surface->verts.push_back(temp);
			}
			else
			{
				temp = onesplinepoint(control, width, length, u, v, uknots, vknots);
				surface->verts.push_back(temp);
			}
		}
	}

	free(uknots);
	free(vknots);
}

void zigzagdraw(CPU_Geometry obj, CPU_Geometry* newobj, int width, int length) //This is just a test function to see if point2dindex works as intended
{
	int i, j;
	glm::vec3 temp;
	newobj->verts.clear();
	for (j = 0; j < length; j++)
	{
		if (j % 2 == 0)
		{
			for (i = 0; i < width; i++)
			{
				temp = point2dindex(obj, i, j, width);
				newobj->verts.push_back(temp);
			}
		}
		else
		{
			for (i = (width - 1); i >= 0; i--)
			{
				temp = point2dindex(obj, i, j, width);
				newobj->verts.push_back(temp);
			}
		}
	}
}

void zagzigdraw(CPU_Geometry obj, CPU_Geometry* newobj, int width, int length)
{
	int i, j;
	glm::vec3 temp;
	int checkval
	//don't clear the vertices here, because you want to add onto a line strip that should have already had vertices placed
	for (i = (width - 1); i >= 0; i--)
	{
		checkval = (width - 1) - i; //as i get's smaller, this will get largeer
		if (checkval % 2 == 0)
		{
			for (j = (length - 1); j >= 0; j--)
			{
				temp = point2dindex(obj, i, j, width);
				newobj->verts.push_back(temp);
			}
		}
		else
		{
			for (j = 0; j < length; j++)
			{
				temp = point2dindex(obj, i, j, width);
				newobj->verts.push_back(temp);
			}
		}
	}
}

void rendertest(CPU_Geometry lineobj, GPU_Geometry* output_gpu)
{
	output_gpu->setVerts(lineobj.verts);
	output_gpu->bind();
	glDrawArrays(GL_LINE_STRIP, 0, GLsizei(lineobj.verts.size()));
	//output_cpu.verts.clear();
}
