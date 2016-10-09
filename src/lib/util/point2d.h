#ifndef POINT2D_H

#define POINT2D_H

struct Point2D
{
	Point2D() 										{ }
	Point2D(double x0, double y0) 								{ x = x0; y = y0; }

	double x, y;
};

#endif // POINT2D_H
