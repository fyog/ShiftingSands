#include "BSpline.h"

float calcomega(float * knots, float u, int i, int r)
{
	//The value of omega will be needed in the quickspline() function, so we define it here.
	float denom = knots[i + r - 1] - knots[i]; //Two knots are subtracted from each other to get the denominator. If the knot sequence was generated properly, then denom should always be positive,
	//(since if (i < j) -> then knots[i] <= knots[j], and therefore knots[j] - knots[i] >= 0). 
	if (denom == 0) return -1; //If r is defined properly, then denom should be strictly greater than 0
	float omega = (u - knots[i]) / denom; //and, of course, u is for the value of Q(u) we want to find, where Q(u) is the function of some B-spline
	return omega;
}

int getdelta(float* knots, int n, float u, int delta)
{
	//getdelta() will calculate the delta index in knots for some value of u. we also pass in the last known delta.

	int i = delta; //
	bool flag = (knots[i] > u || u >= knots[i + 1]); //This is equivalent to !(knots[i] <= u && u < knots[i + 1]). If this is true then i is NOT the knot index. If it's false, the loop won't loop.
	while (flag && i < (n - 1)) //n should be passed in at function call. There are n floats in knots, and on each iteration, the flag checks if the new delta's at i
	{
		//on the final iteration, i == (n - 2), and so (i + 1) is (n - 1), the final index in knots. Make sure n, delta, and knots are all properly defined before calling this function to avoid segfaults.
		//standardknots() is used to generate the knot sequence.
		flag = (knots[i] > u || u >= knots[i + 1]); //This is equivalent to !(knots[i] <= u && knots[i] < u). So, if the flag is false, then i is the knot value
		if (u == 1) flag = false; //If u == 1, that's a special case that is handled in the bspline() function (i think..)
		if (flag) i++; //If i is the knot index, then we don't want to increment i, since i is delta 
	}
	if (i < (n - 1)) return i; //if (i >= (n - 1), then the entire loop iterated through without finding delta, otherwise, return delta

	return (-1); //Error value. either check for the error value or just don't make errors.
}

void standardknots(float* knots, int k, int m)
{
	//This function will generate the standard knot sequence for a b-spline of order k with (m + 1) control points. make sure the knots pointer/array has been properly allocated/populated before calling function
	int n = m + k; //We want to generate (m + k + 1) knots, and the array starts counting at 0, so the final index in the array will be at knots[n] (as n is currently defined)
	int i;

	/*
	Remember that the standard knot sequence should begin with k 0's and end with k 1's, so we already know 2k of the knots we need for the sequence

	Depending on the values of m and k, it may not be possible to generate a standard knot sequence, but we assume the program will have made sure the values are properly defined (the checkpointcount() function
	can check if a CPU_Geometry object contains enough control points to generate a standard knot sequence of order k

	Recall that the parameter range for u is [0,1], and n = (m + k) (where n is the final index in the knots array, and there are (n + 1) knots). If the array starts counting at 0, and there are k 0's at the
	beginning, then the last 0 will be at index (k - 1). Similarly, if we know there are k 1's at the end, and n is the index of the last knot, then we know that the index of the first 1 is at

		j = n - (k - 1) -> (m + k) - (k - 1) == m + 1

	Imagine we had a sub-array of knots that excluded the repeated values at the beginning and end of knots. Call it sub[], perhaps. so, if we had sub[0] = knots[(k - 1)] then the first 1 in knots at knots[m + 1]
	should be equal to sub[(m + 1) - (k - 1)] = sub[(m + 1) + (1 - k)] = sub[m - k + 2] = sub[m - (k - 2)]. So, if we start from sub[0], and want to get to sub[m - (k - 2)] we'd have to take (m - (k - 2)) steps.
	If we want the knots to be equally distributed, then each step should go forward the same amount. Therefore:
	*/

	float step = 1 / (float(m - (k - 2))); //Defining step value

	

	for (i = 0; i <= (k - 1); i++)
	{
		knots[i] = 0.f; //Placing k 0.f value knots at beginning of sequence
	}

	for (i = n; i >= m + 1; i--)
	{
		knots[i] = 1.f; //Placing k 1.f value knots a the end of sequence.
	}
	float j = 1.f; //we sort of use j as a parallel value to i in the loop. It also increases by 1 in every iteration, but we use it with floats, so it's a float. Initialize j to 1

	for (i = k; i <= m; i++)
	{
		knots[i] = j * step; //Defining the remaining knots in the standard sequence
		j = j + 1.f;
	}
}

glm::vec3 onesplinecalc(float omega, CPU_Geometry* rangecopy, int poindex)
{
	glm::vec3 returner = glm::vec3(0.f, 0.f, 0.f); 
	returner = (omega * (rangecopy->verts[poindex])) + ((1.f - omega) * (rangecopy->verts[poindex + 1])); // find some point at omega between the two points
	return returner;
}

glm::vec3 quickspline(CPU_Geometry * control, float * knots, float u, int k, int m)
{
	int n = m + k + 1;
	int delta = getdelta(knots, n, u, 0);

	CPU_Geometry rangelife;

	int i;
	for (i = 0; i < k; i++)
	{
		rangelife.verts.push_back(control->verts[delta - i]); //if i could settle down then i would settle down but i won't settle down.. these are the words of malkmus and pavement..
		//rangelife will start out as the control points within a certain range from delta. Notice that they would also be placed in reverse order from how they were in the control vector.
	}

	/* The quickspline algorithm works by repeatedly sub-dividing the lines defined by the points in rangelife, then redefining the points in the vector, then repeating. So, if rangelife[i]^j
	and rangelife[i + 1]^j are two points, and j is the number of times the outer loop has iterated, then when the outer loop has iterated (j + 1) times, then rangelife[i]^(j + 1) will be some
	point on the line between rangelife[i]^j and rangelife[i + 1]^j */

	int r, tempdelt;
	float omega;
	for (r = k; r >= 2; r--) //on each iteration of the outer loop, the range of values to consider shrinks
	{
		tempdelt = delta;
		for (i = 0; i < (r - 1); i++)
		{
			//if (georep) drawdemo(&rangelife, linedemo, r); //vestigial code from assignment 2. don't need that any more!
			omega = calcomega(knots, u, tempdelt, r);
			rangelife.verts[i] = onesplinecalc(omega, &rangelife, i); //onesplinecalc() handles the actual point/line calculations
			tempdelt = tempdelt - 1;
		}
	}
	return rangelife.verts[0];
}

void bspline(int k, CPU_Geometry * control, float ustep, CPU_Geometry * curve)
{
	int m = control->verts.size() - 1; //there are m + 1 control points, therefore, if n is the number of control points, m + 1 = n -> m = n - 1
	float * knots;
	knots = (float*)malloc((m + k + 1) * sizeof(float)); //malloc allows us declare arrays of arbitrary size. from this point forward, knots will essentially act like 1d array of knots

	standardknots(knots, k, m); //The standard knot takes the pointer value of knots, then populates the array with the standard knot sequence for the given k and m.

	//int i; // the loop below was included for debug purposes, to show that the correct knot sequence is generated. We don't need it, or this variable any more! But uncomment if you'd like to see the knot values
	/*
	for (i = 0; i < (m + k + 1); i++)
	{
		std::cout << knots[i] << ", ";
	}
	std::cout << "\n";
	*/

	curve->verts.clear();
	//curve->cols.clear(); //The new boilerplate doesn't have cols in the CPU_Geometry objects, but the curve seems to render fine without it. (the line shows up as black by default)
	
	//glm::vec3 defaultcol = control->cols[0];
	curve->verts.push_back(control->verts[0]); //The first control point is interpolated with the curve, therefore, instead of calculating Q(u) at u == 0, we simply place the control point 
	//curve->cols.push_back(defaultcol);
	
	float u;

	for (u = ustep; u < 1.f; u += ustep)
	{
		curve->verts.push_back(quickspline(control, knots, u, k, m));
		//curve->cols.push_back(defaultcol);
	}

	curve->verts.push_back(control->verts[m]); //The final control point, like the first is interpolated with the curve. the cherry on top, one could say.
	//curve->cols.push_back(defaultcol);

	//std::cout << "size of curve " << curve->verts.size() << "\n";
	
	free(knots); //when you allocate memory to the pointer knots with malloc, you have to free it in the end, or else you'll get segfaults.
}

bool checkpointcount(int k, CPU_Geometry* control)
{
	//If there are enough control points to generate a b-spline of order k, then the function returns true, otherwise, it returns false
	//int m = control->verts.size() - 1;
	if (control->verts.size() >= k) return true;
	//there are m + 1 control points, and we want (m + k + 1) knots, and there has to be at least 2k knots. so to satisfy the condition we'd need:
	//(m + k + 1) >= 2 * k -> m + 1 >= (2 * k) - k -> (m + 1) >= k
	//std::cout << "Please place more control points. You need at least " << k << "\n";
	return false;
}

bool checkpointcount(int k, int m)
{
	if (m > k) return true;
	return false;
}

