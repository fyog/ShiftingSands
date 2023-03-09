#include "BSpline.h"
#include "Geometry.h"

#define K_ORDER   4


//When the the control points for the surface are generated, they will be passed to the surface generator as either a vector of glm::vec3 vertices, or else a CPU_Geometry object (which contains a vertex vector)

/* If the width of a surface is k units, and it's length is j, then the vertex vector should be constructed such that it has one dimension, but every block of k vertices will represent some row of control points,
and there should be j blocks of k control points. So, let's make a function that allows us to refer to vertices in the vector as though it were a 2d array. We'll pass it a CPU_Geom though.*/

glm::vec3 point2dindex(CPU_Geometry obj, int rowdex, int coldex, int width)
{
	//When we defined rowdex and coldex, rowdex was meant to be the index in some row and the index of some column, but the names might be a bit ambiguous since one might think of "rowdex == x" means that we're
	//looking at the xth row, but actually it would be the xth element in that row. basically, you should think of like (rowdex) is the x-coord on a cartesian plane and (coldex) is the y-coord.
	glm::vec3 returner = obj.verts[(width * coldex) + rowdex]; //If this were a 2d [width] by [length] array, then this element would be equivalent to the [rowdex][coldex] element
	return returner;
}

void getcontrolpoints(CPU_Geometry control, CPU_Geometry* copy, int index, int width, int leng, bool alongwidth)
{
	//getcontrolpoints is used to populate the vertex vector of the CPU_Geometry object that "copy" is pointing to. "control" should have the grid of control points (actually a 1d vector, but close enough for
	//our purposes), and each row and column could be thought of as control points for one 2d b-spline curve. We want to use getcontrolpoints() to get these points so we can use them while generating the 3d
	//surface. "index" will be the index of the row, or column we want to grab, and if "alongwidth" is true, then we want to get a row, and if it's false we should get a column instead.
	//width and leng are the width and length of the grid :-)
	copy->verts.clear();
	int i;
	glm::vec3 temp;
	if (alongwidth)
	{
		for (i = 0; i < width; i++) //if we want to go along a row, then there should be "width" elements to grab.
		{
			temp = point2dindex(control, i, index, width); //If alongwidth is true, then we get every element in the row "index" by iterating through the columns
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
	CPU_Geometry ucontrol, vrange; //ucontrol will have the control points going in the u direction, vrange will be the control points in some relevant range in the v direction
	int m = length - 1; //there are "length" control points going along and m for a b-spline calculation is the number of control points minus 1
	int k = K_ORDER; //We'll assume all b-spline curves should be of order four (cubic). The constant is defined at teh top if you want to change it, though
	int vdelta = getdelta(vknots, (m + k + 1), v, 0); //As in the quickspline() algorithm, we need to get the delta knot to know what the relevant range to look at is.

	m = width - 1; //The m for control points going along the width
	int i;
	glm::vec3 temp;
	//std::cout << "Initial calls to quickspline()\n";
	for (i = 0; i < k; i++)
	{
		getcontrolpoints(control, &ucontrol, (vdelta - i), width, length, true); //This gets the control points for a row at column (vdelta - i) which we'll need for the quickspine() call
		temp = quickspline(&ucontrol, uknots, u, k, m);
		vrange.verts.push_back(temp); //we can still think of vrange as being a set of control points, but they'll be different than any of the points in "control"
	}
	//Let's think about how a brute force application would find the points of a b-spline curve. It would multiply every control point by some basis function with some basis function, then
	//sum all the terms together. what the quickspline() does, is exclude all terms in the sum that would evaluate to 0, and this finds a single point on a b-spline curve.
	//Now, what THIS function did above was find a series of points on a b-spline curve going in the u direction. So, basically we can treat these transformed points as though they were the
	//control points of some new b-spline curve on the surface, and going in the v direction. So, the rest of the function essentially works like quickspline(). We want to find one point
	//on the subcurve we found using u, and that is equivalent to finding one point on the surface.

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
	int k = K_ORDER;
	glm::vec3 temp;

	CPU_Geometry ucontrol;
	CPU_Geometry vcontrol;

	if (checkpointcount(k, (width - 1)) || checkpointcount(k, (length - 1))) return; //if we don't have enough control points to generate a surface, return immediately without changing surface's vertex vector.

	int m = width - 1;
	uknots = (float*)malloc((m + k + 1) * sizeof(float)); //allocates space for (m + k + 1) floats, and allows us to treat uknots as an array of size (m + k + 1)
	m = length - 1;
	vknots = (float*)malloc((m + k + 1) * sizeof(float));
	//std::cout << "Creating standard knots\n";
	standardknots(uknots, k, (width - 1));
	standardknots(vknots, k, (length - 1));
	//std::cout << "standard knots created\n";

	float step = 0.01f; //Setting the step to 0.01 means every surface generated will be 101 x 101 vertices.
	float u, v;

	surface->verts.clear();

	for (v = 0.f; v <= 1.f; v += step) //The outer loop goes along the length of the surface
	{
		for (u = 0.f; u <= 1.f; u += step) //The inner loop goes along the width.
		{
			//Several if statements will handle end-point interpolation
			if (u == 0.f && v == 0.f)
			{
				temp = point2dindex(control, 0, 0, width); //the control point at these indices will be some corner of the surface
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
				temp = quickspline(&vcontrol, vknots, v, k, (length - 1)); //in this case, Q(u,v) will be some point on a 2d b-spline curve that already has defined control points
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

	free(uknots); //Don't forget to free pointers that you allocated space to with malloc()
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

void rendertest(CPU_Geometry lineobj)
{
	GPU_Geometry output_gpu;
	output_gpu.setVerts(lineobj.verts);
	output_gpu.bind();
	glDrawArrays(GL_LINE_STRIP, 0, GLsizei(lineobj.verts.size()));
	//output_cpu.verts.clear();
}
