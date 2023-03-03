#include "BSpline.h"

float calcomega(float * knots, float u, int i, int r)
{
	float denom = knots[i + r - 1] - knots[i];
	if (denom == 0) return -1;
	float omega = (u - knots[i]) / denom;
	return omega;
}

int getdelta(float* knots, int n, float u, int delta)
{
	int i = delta;
	bool flag = (knots[i] > u || u >= knots[i + 1]); //This is equivalent to (knots[i] <= u && u < knots[i + 1]
	while (flag && i < (n - 1))
	{
		//on final iteration, i == (n - 2)
		flag = (knots[i] > u || u >= knots[i + 1]);
		if (u == 1) flag = false;
		if (flag) i++;
	}
	if (i < (n - 1)) return i;

	return (-1);
}

void standardknots(float* knots, int k, int m)
{
	int n = m + k;
	int i;

	float step = 1 / (float(m - (k - 2)));

	

	for (i = 0; i < (k - 1); i++)
	{
		knots[i] = 0.f;
	}

	for (i = n; i >= m + 1; i--)
	{
		knots[i] = 1.f;
	}
	float j = 0.f;
	for (i = (k - 1); i < (m + 1); i++)
	{
		knots[i] = j * step;
		j = j + 1.f;
	}

}

glm::vec3 onesplinecalc(float omega, CPU_Geometry* rangecopy, int poindex)
{
	glm::vec3 returner = glm::vec3(0.f, 0.f, 0.f);
	returner = (omega * (rangecopy->verts[poindex])) + ((1.f - omega) * (rangecopy->verts[poindex + 1]));
	return returner;
}

glm::vec3 quickspline(CPU_Geometry * control, float * knots, float u, int k, int m, bool georep, CPU_Geometry * linedemo)
{
	int n = m + k + 1;
	int delta = getdelta(knots, n, u, 0);

	CPU_Geometry rangelife;

	int i;
	for (i = 0; i < k; i++)
	{
		rangelife.verts.push_back(control->verts[delta - i]); //if i could settle down then i would settle down but i won't settle down
	}

	int r, tempdelt;
	float omega;
	for (r = k; r >= 2; r--)
	{
		tempdelt = delta;
		for (i = 0; i < (r - 1); i++)
		{
			//if (georep) drawdemo(&rangelife, linedemo, r);
			omega = calcomega(knots, u, tempdelt, r);
			rangelife.verts[i] = onesplinecalc(omega, &rangelife, i);
			tempdelt = tempdelt - 1;
		}
	}
	return rangelife.verts[0];
}

void bspline(int k, CPU_Geometry * control, float ustep, CPU_Geometry * curve)
{
	int m = control->verts.size() - 1; //there are m + 1 control points, therefore, if n is the number of control points, m + 1 = n -> m = n - 1
	float * knots;
	knots = (float*)malloc((m + k + 1) * sizeof(float));

	standardknots(knots, k, m);

	int i;
	for (i = 0; i < (m + k + 1); i++)
	{
		std::cout << knots[i] << ", ";
	}
	std::cout << "\n";

	curve->verts.clear();
	//curve->cols.clear();
	
	//glm::vec3 defaultcol = control->cols[0];
	curve->verts.push_back(control->verts[0]); //The first control point is interpolated with the curve
	//curve->cols.push_back(defaultcol);
	
	float u;


	
	for (u = ustep; u < 1.f; u += ustep)
	{
		curve->verts.push_back(quickspline(control, knots, u, k, m, false, NULL));
		//curve->cols.push_back(defaultcol);
	}

	curve->verts.push_back(control->verts[m]);
	//curve->cols.push_back(defaultcol);

	std::cout << "size of curve " << curve->verts.size() << "\n";
	
	free(knots);
}

bool checkpointcount(int k, CPU_Geometry* control)
{
	//If there are enough control points to generate a b-spline of order k, then the function returns true, otherwise, it returns false
	int m = control->verts.size() - 1;
	if (control->verts.size() >= k) return true;
	//there are m + 1 control points, so to satisfy the condition we'd need:
	//(m + k + 1) >= 2 * k -> m + 1 >= (2 * k) - k -> (m + 1) >= k
	std::cout << "Please place more control points. You need at least " << k << "\n";
	return false;
}

